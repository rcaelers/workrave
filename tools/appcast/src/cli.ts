import yargs from 'yargs/yargs';

import { promises as fs } from 'fs';
import { hideBin } from 'yargs/helpers';

import { Generator } from './generator.js';
import { S3Store } from './s3.js';

import fetch from 'node-fetch';
import path from 'path';

const getEnv = (varName: string) => {
  const value = process.env[varName];

  if (!value) {
    throw `${varName} environment variable missing.`;
  }

  return value;
};

export default class CLI {
  args: any;

  async run(): Promise<void> {
    this.args = await this.parse_args();

    const response = await fetch('https://snapshots.workrave.org/snapshots/v1.11/catalog.json');
    const catalog = await response.json();

    const params = {};

    const generator = new Generator(catalog, params);
    const content = await generator.generate();

    if (!this.args.dry) {
      if (this.args.file) {
        await fs.writeFile(this.args.name, content);
      } else {
        const secretAccessKey = getEnv('SNAPSHOTS_SECRET_ACCESS_KEY');
        const storage = new S3Store(this.args.endpoint, this.args.bucket, this.args.key, secretAccessKey);
        await storage.write(path.join(this.args.branch, this.args.name), content, 'text/xml');
      }
    }
  }

  async parse_args(): Promise<any> {
    const args = yargs(hideBin(process.argv))
      .scriptName('appcast')
      .usage('$0 [args]')
      .help('h')
      .alias('h', 'help')
      .options({
        branch: {
          alias: 'b',
          default: 'v1.11',
        },
        bucket: {
          default: 'snapshots',
        },
        key: {
          default: 'travis',
        },
        endpoint: {
          default: 'https://snapshots.workrave.org/',
        },
        name: {
          alias: 'n',
          type: 'string',
          default: 'appcast.xml',
        },
        file: {
          type: 'boolean',
          default: false,
        },
        release: {
          describe: 'First release to generate.',
          type: 'string',
        },
        dry: {
          type: 'boolean',
          alias: 'd',
          default: false,
          describe: 'Dry run. Result is not uploaded to storage.',
        },
        verbose: {
          type: 'boolean',
          alias: 'v',
          default: false,
        },
      })
      .parseSync();

    return args;
  }
}
