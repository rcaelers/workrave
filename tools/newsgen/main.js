import { Generator } from './generator.js';
import { promises as fs } from 'fs';
import yargs from 'yargs';
import yaml from 'js-yaml';
import { hideBin } from 'yargs/helpers';

const main = async () => {
  let storage = null;

  try {
    const args = yargs(hideBin(process.argv))
      .scriptName('catalog')
      .usage('$0 [args]')
      .help('h')
      .alias('h', 'help')
      .options({
        input: {
          alias: 'i',
          type: 'string',
        },
        output: {
          alias: 'o',
          type: 'string',
        },
        increment: {
          describe: 'The PPA increment to use.',
          type: 'number',
          default: 1,
        },
        ubuntu: {
          describe: 'The Ubuntu release name.',
          type: 'string',
          default: 'focal',
        },
        release: {
          describe: 'Only generate this release starting at this version.',
          type: 'string',
        },
        single: {
          describe: 'Generate only for a single release',
          type: 'boolean',
          default: false,
        },
        latest: {
          describe: 'Generate only the latest release.',
          type: 'boolean',
          default: false,
        },
        template: {
          describe: 'Template to use.',
          type: 'string',
          default: 'NEWS',
        },
        verbose: {
          type: 'boolean',
          alias: 'v',
          default: false,
        },
      })
      .demandOption(['input', 'output', 'template'], 'Please specify --output and --template')
      .parseSync();

    let news = yaml.load(await fs.readFile(args.input, 'utf8'));
    let params = {
      release: args.release,
      single: args.single,
      latest: args.latest,
      template: args.template,
      series: args.ubuntu,
      increment: args.increment,
    };
    let generator = new Generator(news, params);
    let content = await generator.generate();
    await fs.writeFile(args.output, content);
  } catch (e) {
    console.error(e);
  }
};

main();
