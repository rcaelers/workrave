import { promises as fs } from 'fs';

import { S3Store } from '../../common/s3.js';
import { Catalog } from '../../catalog/catalog.js';
import { Builder } from '../../catalog/builder.js';

import { Command, Flags } from '@oclif/core'

export default class Appcast extends Command {
  static description = 'generate artifacts catalog'

  static examples = [
    `$ citool catalog
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
    workspace: Flags.string({
      char: 'w',
      description: 'Git workspace',
      env: 'WORKSPACE',
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
    regenerate: Flags.boolean({
      char: 'r',
      description: 'regenerate',
      default: false,
    }),
  }

  async run(): Promise<void> {
    const { args, flags } = await this.parse(Appcast)

    try {
      let storage = new S3Store(flags.endpoint, flags.bucket, flags.key, flags.secret);
      let catalog = new Catalog(storage, flags.branch, flags.dry, flags.regenerate);
      let builder = new Builder(storage, catalog, flags.workspace, flags.branch, flags.dry, flags.regenerate);
      await builder.load();
      await builder.process();
      await builder.save();
    } catch (e) {
      console.error(e);
    }
  }
}
