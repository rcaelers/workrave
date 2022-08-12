import { promises as fs } from 'fs';

import { Generator } from '../../newsgen/generator.js';
import yaml from 'js-yaml';

import { Command, Flags } from '@oclif/core'

export default class Appcast extends Command {
  static description = 'sign artifacts'

  static examples = [
    `$ citool newsgen
`,
  ]

  static flags = {
    input: Flags.string({
      char: 'i',
      description: 'input',
      required: true,
    }),
    output: Flags.string({
      char: 'o',
      description: 'output',
      required: true,
    }),
    increment: Flags.integer({
      char: 'k',
      description: 'PPA increment',
      default: 1,
    }),
    ubuntu: Flags.string({
      char: 'U',
      description: 'The Ubuntu release name',
      default: 'focal'
    }),
    release: Flags.string({
      description: 'Only generate this release starting at this version',
    }),
    single: Flags.boolean({
      description: 'Generate only for a single release',
      default: false,
    }),
    latest: Flags.boolean({
      description: 'Generate only the latest release',
      default: false,
    }),
    template: Flags.string({
      char: 'T',
      description: 'Template to use.',
      default: 'NEWS'
    }),
    verbose: Flags.boolean({
      char: 'v',
      description: 'Verbose',
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
