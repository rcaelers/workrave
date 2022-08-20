import { promises as fs } from 'fs';
import { Command, Flags } from '@oclif/core'
import yaml from 'js-yaml';

import { Generator } from '../../newsgen/generator.js';

export default class Appcast extends Command {
  static description = 'generate release notes in different formats';

  static examples = [
    `$ citool newsgen --input ./changes.yaml --template news -o NEWS`,
    `$ citool newsgen --input ./changes.yaml --single --release 1.10.50 --ubuntu jammy --increment 1 --template debian-changelog --output out`,
    `$ citool newsgen --input ./changes.yaml --single --release=1.11.0-alpha.2 --template github -o out`
  ]

  static flags = {
    input: Flags.string({
      char: 'i',
      description: 'YAML input file containing release notes',
      required: true,
    }),
    output: Flags.string({
      char: 'o',
      description: 'Output file',
      required: true,
    }),
    increment: Flags.integer({
      char: 'k',
      description: 'PPA increment for debian changelog',
      default: 1,
    }),
    ubuntu: Flags.string({
      char: 'U',
      description: 'Ubuntu release name for debian changelog',
      default: 'focal'
    }),
    release: Flags.string({
      description: 'Generate release notes starting from this release',
    }),
    single: Flags.boolean({
      description: 'Generate only release notes for the specified release',
      default: false,
    }),
    latest: Flags.boolean({
      description: 'Generate only realease notes for the latest release',
      default: false,
    }),
    template: Flags.string({
      char: 'T',
      description: 'Release notes template to use',
      default: 'NEWS'
    }),
    verbose: Flags.boolean({
      char: 'v',
      description: 'Verbose mode',
      default: false,
    }),
  }

  async run(): Promise<void> {
    const { args, flags } = await this.parse(Appcast)

    try {
      let news = yaml.load(await fs.readFile(flags.input, 'utf8'));

      let params = {
        release: flags.release,
        single: flags.single,
        latest: flags.latest,
        template: flags.template,
        series: flags.ubuntu,
        increment: flags.increment,
      };
      let generator = new Generator(news, params);
      let content = await generator.generate();
      await fs.writeFile(flags.output, content);
    } catch (e) {
      console.error(e);
    }
  };
}
