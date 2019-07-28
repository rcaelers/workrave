import AWS from 'aws-sdk';
import fs from 'fs';
import path from 'path';

const S3_API_VERSION = '2006-03-01';

class S3Store {
  constructor(region, bucket, prefix, accessKeyId, secretAccessKey) {
    this.bucket = bucket;
    this.prefix = prefix;

    //AWS.config.update({ region: 'eu-central-1' });
    //this.s3 = new AWS.S3({
    //  apiVersion: S3_API_VERSION,
    //  region: region,
    //  accessKeyId: accessKeyId,
    //  secretAccessKey: secretAccessKey,
    //  params: { Bucket: bucket }
    //});

    this.s3 = new AWS.S3({
      accessKeyId: accessKeyId,
      secretAccessKey: secretAccessKey,
      endpoint: 'https://snapshots.workrave.org/',
      s3ForcePathStyle: true,
      signatureVersion: 'v4',
      params: { Bucket: bucket }
    });
  }

  async list(prefix) {
    let isTruncated = true;
    let marker;
    const items = [];
    while (isTruncated) {
      let params = {};
      if (prefix) params.Prefix = prefix;
      if (marker) params.Marker = marker;

      try {
        const response = await this.s3.listObjects(params).promise();
        response.Contents.forEach(item => {
          items.push(item);
        });
        isTruncated = response.IsTruncated;
        if (isTruncated) {
          marker = response.Contents.slice(-1)[0].Key;
        }
      } catch (error) {
        throw error;
      }
    }
    return items;
  }

  async fileExists(file) {
    try {
      await this.s3.headObject({ Key: path.join(this.prefix, file) }).promise();
      return true;
    } catch (error) {
      if (error.statusCode === 404) {
        return false;
      } else {
        throw error;
      }
    }
  }

  async writeJson(file, data) {
    return await this.s3.putObject({ Key: path.join(this.prefix, file), Body: JSON.stringify(data) }).promise();
  }

  async readJson(file) {
    let result = await this.s3.getObject({ Key: path.join(this.prefix, file) }).promise();
    return JSON.parse(result.Body.toString('utf8'));
  }

  async deleteObject(file) {
    return await this.s3.deleteObject({ Key: path.join(this.prefix, file) }).promise();
  }
}

export default S3Store;
export { S3Store };
