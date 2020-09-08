import { Generator } from './generator.js';
import { promises as fs } from 'fs';
import yargs from 'yargs';

const main = async () => {
  let storage = null;

  try {
    var args = yargs
      .scriptName('catalog')
      .usage('$0 [args]')
      .help('h')
      .alias('h', 'help')
      .option('output', {
        alias: 'o'
      })
      .options('increment', {
        describe: 'The PPA increment to use.',
        default: 1
      })
      .options('ubuntu-release', {
        describe: 'The Ubuntu release name.',
        default: 'focal'
      })
      .options('release', {
        describe: 'Only generate this release.'
      })
      .options('template', {
        describe: 'Template to use.',
        default: 'NEWS'
      })
      .option('verbose', {
        alias: 'v',
        default: false
      })
      .demandOption(['output', 'template'], 'Please specify --output and --template').argv;

    let extra = { series: args.series, increment: args.increment };
    let generator = new Generator();
    let content = await generator.generate(args.release, args.template, extra);

    await fs.writeFile(args.output, content);
  } catch (e) {
    console.error(e);
  }
};

main();
