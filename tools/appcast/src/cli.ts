import yargs from 'yargs/yargs';

import { promises as fs } from 'fs';
import { hideBin } from 'yargs/helpers';

import { Generator } from './generator.js';
import fetch from 'node-fetch';

export default class CLI {
  args: any;

  async run(): Promise<void> {
    this.args = await this.parse_args();

    const response = await fetch(
      'https://snapshots.workrave.org/snapshots/v1.11/catalog.json'
    );
    const catalog = await response.json();

    const params = {};

    const generator = new Generator(catalog, params);
    const content = await generator.generate();

    await fs.writeFile(this.args.output, content);
  }

  async parse_args(): Promise<any> {
    const args = yargs(hideBin(process.argv))
      .scriptName('appcast')
      .usage('$0 [args]')
      .help('h')
      .alias('h', 'help')
      .options({
        output: {
          alias: 'o',
          type: 'string'
        },
        release: {
          describe: 'First release to generate.',
          type: 'string'
        },
        verbose: {
          type: 'boolean',
          alias: 'v',
          default: false
        }
      })
      .demandOption(['output'], 'Please specify --output')
      .parseSync();

    return args;
  }
}
