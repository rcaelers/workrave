import { S3Store } from './s3.js';
import { Catalog } from './catalog.js';

const getEnv = varName => {
  const value = process.env[varName];

  if (!value) {
    throw `${varName} environment variable missing.`;
  }

  return value;
};

const main = async () => {
  let storage = null;

  try {
    const region = 'eu-central-1';
    const bucket = 'snapshots';
    const prefix = '';
    const accessKeyId = 'travis';
    const secretAccessKey = getEnv('SNAPSHOTS_SECRET_ACCESS_KEY');

    storage = new S3Store(region, bucket, prefix, accessKeyId, secretAccessKey);
    let catalog = new Catalog(storage);
    catalog.load();
  } catch (e) {
    console.error(e);
  }
};

main();
