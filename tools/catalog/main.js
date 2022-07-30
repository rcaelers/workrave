import { S3Store } from './s3.js';
import { Catalog } from './catalog.js';

import yargs from 'yargs';

const getEnv = (varName) => {
  const value = process.env[varName];

  if (!value) {
    throw `${varName} environment variable missing.`;
  }

  return value;
};

const main = async () => {
  let storage = null;

  try {
    const secretAccessKey = getEnv('SNAPSHOTS_SECRET_ACCESS_KEY');
    const gitRoot = getEnv('WORKSPACE');

    var args = yargs()
      .scriptName('catalog')
      .usage('$0 [args]')
      .help('h')
      .alias('h', 'help')
      .option('branch', {
        alias: 'b',
        default: 'v1.11',
      })
      .option('bucket', {
        default: 'snapshots',
      })
      .option('key', {
        default: 'travis',
      })
      .option('endpoint', {
        default: 'https://snapshots.workrave.org/',
      })
      .option('dry', {
        type: 'boolean',
        alias: 'd',
        default: false,
        describe: 'Dry run. Result is not uploaded to storage.',
      })
      .option('verbose', {
        alias: 'v',
        default: false,
      }).argv;

    storage = new S3Store(args.endpoint, args.bucket, args.dry, args.key, secretAccessKey);
    let catalog = new Catalog(storage, gitRoot, args.branch);
    await catalog.load();
    await catalog.process();
    await catalog.save();
  } catch (e) {
    console.error(e);
  }
};

main();
