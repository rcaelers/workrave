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
    const gitRoot = getEnv('TRAVIS_BUILD_DIR');

    storage = new S3Store(bucket, accessKeyId, secretAccessKey);
    let catalog = new Catalog(storage, gitRoot, prefix);
    await catalog.load();
    await catalog.process();
    await catalog.save();
  } catch (e) {
    console.error(e);
  }
};

main();
