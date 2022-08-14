import AWS from 'aws-sdk';

import fs from 'node:fs/promises';

class S3Store {
  bucket: string;
  s3: AWS.S3;

  constructor(endpoint: string, bucket: string, accessKeyId: string, secretAccessKey: string) {
    this.bucket = bucket;

    this.s3 = new AWS.S3({
      accessKeyId: accessKeyId,
      secretAccessKey: secretAccessKey,
      endpoint: endpoint,
      s3ForcePathStyle: true,
      signatureVersion: 'v4',
      params: { Bucket: bucket },
    });
  }

  async list(directory: string) {
    let isTruncated: boolean | undefined = true;
    let marker;
    const items: any[] = [];
    while (isTruncated) {
      const params: any = {};
      if (directory) params.Prefix = directory;
      if (marker) params.Marker = marker;

      const response = await this.s3.listObjects(params).promise();
      if (response.Contents) {
        items.push(...response.Contents);
        isTruncated = response.IsTruncated;
        if (isTruncated) {
          marker = response.Contents.slice(-1)[0].Key;
        }
      }
    }
    return items;
  }

  async fileExists(filename: string) {
    try {
      await this.s3
        .headObject({
          Bucket: this.bucket,
          Key: filename,
        })
        .promise();
      return true;
    } catch (error: any) {
      if (error.statusCode === 404) {
        return false;
      } else {
        throw error;
      }
    }
  }

  async writeJson(filename: string, data: any) {
    const jsonData = JSON.stringify(data); //, null, '\t');
    return this.s3
      .putObject({
        Bucket: this.bucket,
        Key: filename,
        Body: jsonData,
        ContentType: 'application/json',
      })
      .promise();
  }

  async write(filename: string, data: string, type: string) {
    return this.s3
      .putObject({
        Bucket: this.bucket,
        Key: filename,
        Body: data,
        ContentType: type,
      })
      .promise();
  }

  async readJson(filename: string) {
    const result = await this.s3.getObject({ Bucket: this.bucket, Key: filename }).promise();
    if (result.Body) {
      return JSON.parse(result.Body.toString('utf8'));
    }
    return undefined;
  }

  async download(filename: string, destination: string) {
    try {
      console.log('Downloading: ' + filename);
      const inStream = this.s3.getObject({ Bucket: this.bucket, Key: filename }).createReadStream();
      const file = await fs.open(destination, "w")
      const outStream = file.createWriteStream();
      inStream.pipe(outStream);
      console.log('End Downloading: ' + filename);
    } catch (error) {
      console.error(error);
      throw error;
    }
  }

  async downloadStream(filename: string) {
    try {
      return this.s3.getObject({ Bucket: this.bucket, Key: filename }).createReadStream();
    } catch (error) {
      console.error(error);
      throw error;
    }
  }

  async deleteObject(filename: string) {
    return this.s3.deleteObject({ Bucket: this.bucket, Key: filename }).promise();
  }
}

export default S3Store;
export { S3Store };
