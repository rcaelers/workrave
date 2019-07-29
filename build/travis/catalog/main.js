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
    const bucket = 'snapshots';
    const prefix = 'next';
    const accessKeyId = 'travis';
    const secretAccessKey = getEnv('SNAPSHOTS_SECRET_ACCESS_KEY');

    storage = new S3Store(bucket, accessKeyId, secretAccessKey);
    let catalog = new Catalog(storage, prefix);
    await catalog.load();
    await catalog.mergeCatalogs();
    await catalog.save();
  } catch (e) {
    console.error(e);
  }
};

main();
