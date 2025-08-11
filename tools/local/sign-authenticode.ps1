param(
    [Parameter(ValueFromRemainingArguments=$true)]
    [string[]]$Arguments
)

$FileToSign = $null

foreach ($arg in $Arguments) {
    if ($arg -notmatch '^[/-]') {
        if (Test-Path $arg -PathType Leaf) {
            $FileToSign = $arg
        }
    }
}

if (-not $FileToSign) {
    Write-Error "Error: No file to sign found in arguments: $($Arguments -join ' ')"
    exit 1
}

Write-Host "Inno Setup SignTool: Signing file $FileToSign"

try {
    $healthCheck = Invoke-WebRequest -Uri "http://studio.local:50051/health" -Method GET -TimeoutSec 5 -UseBasicParsing
    if ($healthCheck.StatusCode -ne 200) {
        throw "Service not responding"
    }
} catch {
    Write-Error "Error: Workrave signing service is not running"
    exit 1
}

$boundary = [System.Guid]::NewGuid().ToString()
$filePath = Resolve-Path $FileToSign
$fileName = [System.IO.Path]::GetFileName($filePath)
$fileBytes = [System.IO.File]::ReadAllBytes($filePath)

$LF = "`r`n"
$bodyStart = "--$boundary$LF" +
    "Content-Disposition: form-data; name=`"file`"; filename=`"$fileName`"$LF" +
    "Content-Type: application/octet-stream$LF$LF"
$bodyEnd = "$LF--$boundary--$LF"

$bodyStartBytes = [System.Text.Encoding]::UTF8.GetBytes($bodyStart)
$bodyEndBytes = [System.Text.Encoding]::UTF8.GetBytes($bodyEnd)
$bodyBytes = $bodyStartBytes + $fileBytes + $bodyEndBytes

try {
    $response = Invoke-WebRequest -Uri "http://studio.local:50051/sign/authenticode" -Method POST -Body $bodyBytes -ContentType "multipart/form-data; boundary=$boundary" -TimeoutSec 300 -UseBasicParsing

    if ($response.StatusCode -eq 200) {
        [System.IO.File]::WriteAllBytes($filePath, $response.Content)
        Write-Host "Inno Setup SignTool: Successfully signed $FileToSign"
        exit 0
    } else {
        Write-Error "Inno Setup SignTool: API returned status code $($response.StatusCode)"
        exit 1
    }
} catch {
    Write-Error "Inno Setup SignTool: Failed to sign $FileToSign - $($_.Exception.Message)"
    exit 1
}
