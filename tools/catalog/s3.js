import AWS from 'aws-sdk';
import fs from 'fs';
import path from 'path';
import promisify from 'util';

const S3_API_VERSION = '2006-03-01';

class S3Store {
  constructor(bucket, dry, accessKeyId, secretAccessKey) {
    this.bucket = bucket;
    this.dry = dry;

    this.s3 = new AWS.S3({
      accessKeyId: accessKeyId,
      secretAccessKey: secretAccessKey,
      endpoint: 'https://snapshots.workrave.org/',
      s3ForcePathStyle: true,
      signatureVersion: 'v4',
      params: { Bucket: bucket }
    });
  }

  async list(directory) {
    let isTruncated = true;
    let marker;
    const items = [];
    while (isTruncated) {
      let params = {};
      if (directory) params.Prefix = directory;
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

  async fileExists(filename) {
    try {
      await this.s3.headObject({ Key: filename }).promise();
      return true;
    } catch (error) {
      if (error.statusCode === 404) {
        return false;
      } else {
        throw error;
      }
    }
  }

  async writeJson(filename, data) {
    if (this.dry) {
      console.log('Dry run: writeJson(' + filename + '):');
      console.log(JSON.stringify(data, null, '\t'));
    } else {
      let jsonData = JSON.stringify(data); //, null, '\t');
      return this.s3
        .putObject({
          Key: filename,
          Body: jsonData,
          ContentType: 'application/json'
        })
        .promise();
    }
  }

  async readJson(filename) {
    let result = await this.s3.getObject({ Key: filename }).promise();
    return JSON.parse(result.Body.toString('utf8'));
  }

  async deleteObject(filename) {
    if (this.dry) {
      console.log('Dry run: deleteObject(' + filename + ')');
    } else {
      return this.s3.deleteObject({ Key: filename }).promise();
    }
  }
}

export default S3Store;
export { S3Store };
