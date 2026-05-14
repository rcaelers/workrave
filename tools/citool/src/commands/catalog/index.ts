import { S3Store } from '../../common/s3.js';
import { Catalog } from '../../catalog/catalog.js';
import { Builder } from '../../catalog/builder.js';

import { Command, Flags } from '@oclif/core'

export default class CatalogCmd extends Command {
  static description = 'update artifacts catalog in S3 storage'

  static examples = [
    `$ citool catalog`,
  ]

  static flags = {
    branch: Flags.string({
      char: 'b',
      description: 'Workave branch to use',
      default: 'v1.11',
    }),
    bucket: Flags.string({
      char: 'B',
      description: 'S3 bucket to use',
      default: 'snapshots',
    }),
    key: Flags.string({
      char: 'k',
      description: 'Access key for S3 access',
      default: 'github',
    }),
    secret: Flags.string({
      char: 's',
      description: 'Secret for S3 access',
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
      description: 'S3 endpoint',
      default: 'https://snapshots.workrave.org/',
    }),
    name: Flags.string({
      char: 'n',
      description: 'Output filename',
      default: 'appcast.xml',
    }),
    file: Flags.boolean({
      description: 'Output to local file instead of S3 bucket',
      default: false,
    }),
    release: Flags.string({
      char: 'n',
      description: 'Generate release notes starting from this release',
    }),
    dry: Flags.boolean({
      char: 'd',
      description: 'Dry run. Result is not uploaded to S3 storage',
      default: false,
    }),
    regenerate: Flags.boolean({
      char: 'r',
      description: 'Regenerate catalog',
      default: false,
    }),
  }

  async run(): Promise<void> {
    const { args, flags } = await this.parse(CatalogCmd)

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
