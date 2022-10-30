import { promises as fs } from 'fs';

import { AppcastGenerator } from '../../appcast/generator.js';
import { S3Store } from '../../common/s3.js';

import fetch from 'node-fetch';
import path from 'path';

import { Command, Flags } from '@oclif/core'

export default class Appcast extends Command {
  static description = 'generate appcast'

  static examples = [
    `$ citool appcast
`,
  ]

  static flags = {
    branch: Flags.string({
      char: 'b',
      description: 'branch',
      default: 'v1.11',
    }),
    bucket: Flags.string({
      char: 'B',
      description: 'bucket',
      default: 'snapshots',
    }),
    key: Flags.string({
      char: 'k',
      description: 'key',
      default: 'travis',
    }),
    secret: Flags.string({
      char: 's',
      description: 'key',
      env: 'SNAPSHOTS_SECRET_ACCESS_KEY',
      required: true,
    }),
    endpoint: Flags.string({
      char: 'E',
      description: 'endpoint',
      default: 'https://snapshots.workrave.org/',
    }),
    name: Flags.string({
      char: 'n',
      description: 'file name',
      default: 'appcast.xml',
    }),
    file: Flags.boolean({
      description: 'output to file instead of S3 bucket',
      default: false,
    }),
    release: Flags.string({
      char: 'n',
      description: 'First release to generate',
    }),
    dry: Flags.boolean({
      char: 'd',
      description: 'Dry run. Result is not uploaded to storage',
      default: false,
    }),
  }

  async run(): Promise<void> {
    const { args, flags } = await this.parse(Appcast)

    const response = await fetch(flags.endpoint + '/' + flags.bucket + '/' + flags.branch + '/catalog.json');
    const catalog = await response.json();

    const params = {};

    const generator = new AppcastGenerator(catalog, params);
    const content = await generator.generate();

    if (!flags.dry) {
      if (flags.file) {
        await fs.writeFile(flags.name, content);
      } else {
        const storage = new S3Store(flags.endpoint, flags.bucket, flags.key, flags.secret);
        await storage.write(path.posix.join(flags.branch, flags.name), content, 'text/xml');
      }
    }
  }
}
