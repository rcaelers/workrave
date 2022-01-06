import { Generator } from './generator.js';
import { promises as fs } from 'fs';
import yargs from 'yargs';
import yaml from 'js-yaml';
import { hideBin } from 'yargs/helpers';

const main = async () => {
  let storage = null;

  try {
    var args = yargs(hideBin(process.argv))
      .scriptName('catalog')
      .usage('$0 [args]')
      .help('h')
      .alias('h', 'help')
      .option('input', {
        alias: 'i'
      })
      .option('output', {
        alias: 'o'
      })
      .options('increment', {
        describe: 'The PPA increment to use.',
        default: 1
      })
      .options('ubuntu', {
        describe: 'The Ubuntu release name.',
        default: 'focal'
      })
      .options('release', {
        describe: 'Only generate this release starting at this version.'
      })
      .options('single', {
        describe: 'Generate only for a single release'
      })
      .options('template', {
        describe: 'Template to use.',
        default: 'NEWS'
      })
      .option('verbose', {
        alias: 'v',
        default: false
      })
      .demandOption(['input', 'output', 'template'], 'Please specify --output and --template').argv;

    let extra = { series: args.ubuntu, increment: args.increment };
    let news = yaml.safeLoad(await fs.readFile(args.input, 'utf8'));
    let generator = new Generator(news);
    let content = await generator.generate(args.release, args.single, args.template, extra);

    await fs.writeFile(args.output, content);
  } catch (e) {
    console.error(e);
  }
};

main();
