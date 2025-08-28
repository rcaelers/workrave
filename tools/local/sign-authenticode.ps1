param(
    [Parameter(ValueFromRemainingArguments=$true)]
    [string[]]$Arguments
)

Write-Host "Inno Setup SignTool: Processing arguments: $($Arguments -join ' ')"

$FileToSign = $null

foreach ($arg in $Arguments) {
    if ($arg -notmatch '^[/-]') {
        if (Test-Path $arg -PathType Leaf) {
            $FileToSign = $arg
            Write-Host "Inno Setup SignTool: Found file to sign: $FileToSign"
            break
        }
    }
}

if (-not $FileToSign) {
    Write-Error "Error: No file to sign found in arguments: $($Arguments -join ' ')"
    exit 1
}

Write-Host "Inno Setup SignTool: Signing file $FileToSign"

try {
    Write-Host "Inno Setup SignTool: Checking signing service health..."
    $healthCheck = Invoke-WebRequest -Uri "http://studio.local:50051/health" -Method GET -TimeoutSec 5 -UseBasicParsing
    if ($healthCheck.StatusCode -ne 200) {
        throw "Service not responding properly (Status: $($healthCheck.StatusCode))"
    }
    Write-Host "Inno Setup SignTool: Signing service is healthy"
} catch {
    Write-Warning "Warning: Workrave signing service is not available - $($_.Exception.Message)"
    exit 1
}

$boundary = [System.Guid]::NewGuid().ToString()
$filePath = Resolve-Path $FileToSign
$fileName = [System.IO.Path]::GetFileName($filePath)
$fileInfo = Get-Item $filePath

$LF = "`r`n"
$bodyStart = "--$boundary$LF" +
    "Content-Disposition: form-data; name=`"file`"; filename=`"$fileName`"$LF" +
    "Content-Type: application/octet-stream$LF$LF"
$bodyEnd = "$LF--$boundary--$LF"

$bodyStartBytes = [System.Text.Encoding]::UTF8.GetBytes($bodyStart)
$bodyEndBytes = [System.Text.Encoding]::UTF8.GetBytes($bodyEnd)

# Calculate total content length without loading entire file into memory
$totalLength = $bodyStartBytes.Length + $fileInfo.Length + $bodyEndBytes.Length

Write-Host "Inno Setup SignTool: File size: $($fileInfo.Length) bytes, Total request size: $totalLength bytes"

try {
    Write-Host "Inno Setup SignTool: Sending request to signing service..."

    $request = [System.Net.HttpWebRequest]::Create("http://studio.local:50051/sign/authenticode")
    $request.Method = "POST"
    $request.ContentType = "multipart/form-data; boundary=$boundary"
    $request.ContentLength = $totalLength
    $request.Timeout = 600000  # 10 minutes
    $request.ReadWriteTimeout = 600000  # 10 minutes for read/write operations
    $request.UserAgent = "Workrave-SignTool/1.0"
    $request.SendChunked = $false
    $request.KeepAlive = $true
    $request.ServicePoint.Expect100Continue = $false

    $requestStream = $request.GetRequestStream()

    try {
        # Write the multipart header
        $requestStream.Write($bodyStartBytes, 0, $bodyStartBytes.Length)

        # Stream the file content in chunks with progress reporting
        $fileStream = [System.IO.File]::OpenRead($filePath)
        $buffer = New-Object byte[] 65536  # 64KB buffer for better performance
        $bytesRead = 0
        $totalBytesRead = 0
        $lastProgressTime = Get-Date

        do {
            $bytesRead = $fileStream.Read($buffer, 0, $buffer.Length)
            if ($bytesRead -gt 0) {
                $requestStream.Write($buffer, 0, $bytesRead)
                $totalBytesRead += $bytesRead

                # Progress reporting every 5MB or 5 seconds
                $currentTime = Get-Date
                if (($totalBytesRead % 5242880 -lt $bytesRead) -or ($currentTime - $lastProgressTime).TotalSeconds -gt 5) {
                    $percentComplete = [math]::Round(($totalBytesRead / $fileInfo.Length) * 100, 1)
                    Write-Host "Inno Setup SignTool: Upload progress: $percentComplete% ($totalBytesRead / $($fileInfo.Length) bytes)"
                    $lastProgressTime = $currentTime
                }
            }
        } while ($bytesRead -gt 0)

        $fileStream.Close()

        # Write the multipart footer
        $requestStream.Write($bodyEndBytes, 0, $bodyEndBytes.Length)
        Write-Host "Inno Setup SignTool: Upload complete, waiting for response..."
    }
    finally {
        $requestStream.Close()
    }

    # Get response with increased timeout handling
    Write-Host "Inno Setup SignTool: Getting response from signing service..."
    $response = $request.GetResponse()

    if ($response.StatusCode -eq [System.Net.HttpStatusCode]::OK) {
        Write-Host "Inno Setup SignTool: Downloading signed file..."
        $responseStream = $response.GetResponseStream()

        # Stream the response to avoid memory issues with large files
        $tempFilePath = "$filePath.tmp"
        $fileWriteStream = [System.IO.File]::Create($tempFilePath)

        try {
            $responseBuffer = New-Object byte[] 65536  # 64KB buffer
            $totalResponseBytes = 0
            $lastProgressTime = Get-Date

            do {
                $responseBytesRead = $responseStream.Read($responseBuffer, 0, $responseBuffer.Length)
                if ($responseBytesRead -gt 0) {
                    $fileWriteStream.Write($responseBuffer, 0, $responseBytesRead)
                    $totalResponseBytes += $responseBytesRead

                    # Progress reporting every 5MB or 5 seconds
                    $currentTime = Get-Date
                    if (($totalResponseBytes % 5242880 -lt $responseBytesRead) -or ($currentTime - $lastProgressTime).TotalSeconds -gt 5) {
                        Write-Host "Inno Setup SignTool: Download progress: $totalResponseBytes bytes received"
                        $lastProgressTime = $currentTime
                    }
                }
            } while ($responseBytesRead -gt 0)
        }
        finally {
            $fileWriteStream.Close()
            $responseStream.Close()
            $response.Close()
        }

        # Replace original file with signed version
        Move-Item $tempFilePath $filePath -Force
        Write-Host "Inno Setup SignTool: Successfully signed $FileToSign (final size: $totalResponseBytes bytes)"
        exit 0
    } else {
        Write-Error "Inno Setup SignTool: API returned status code $($response.StatusCode)"
        $response.Close()
        exit 1
    }
} catch [System.Net.WebException] {
    $webEx = $_.Exception
    Write-Host "Inno Setup SignTool: Web exception occurred: $($webEx.Message)"
    Write-Host "Inno Setup SignTool: Exception status: $($webEx.Status)"

    if ($webEx.Response) {
        try {
            $errorStream = $webEx.Response.GetResponseStream()
            $errorReader = New-Object System.IO.StreamReader($errorStream)
            $errorText = $errorReader.ReadToEnd()
            Write-Host "Inno Setup SignTool: Server response: $errorText"
            $errorReader.Close()
            $webEx.Response.Close()
        } catch {
            Write-Host "Inno Setup SignTool: Could not read error response"
        }
    }

    Write-Error "Inno Setup SignTool: Failed to sign $FileToSign - $($webEx.Message)"
    exit 1
} catch [System.IO.IOException] {
    $ioEx = $_.Exception
    Write-Host "Inno Setup SignTool: I/O exception occurred: $($ioEx.Message)"
    Write-Error "Inno Setup SignTool: Failed to sign $FileToSign - Network or file I/O error: $($ioEx.Message)"
    exit 1
} catch {
    Write-Host "Inno Setup SignTool: Unexpected exception: $($_.Exception.GetType().FullName)"
    Write-Host "Inno Setup SignTool: Exception message: $($_.Exception.Message)"
    Write-Error "Inno Setup SignTool: Failed to sign $FileToSign - $($_.Exception.Message)"
    exit 1
}
