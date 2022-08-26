import { S3Store } from '../../common/s3.js';
import { Catalog } from '../../catalog/catalog.js';
import { Signer } from '../../catalog/signer.js';

import { Command, Flags } from '@oclif/core'

export default class Appcast extends Command {
  static description = 'sign artifacts'

  static examples = [
    `$ citool sign`,
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
      default: 'travis',
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
    dry: Flags.boolean({
      char: 'd',
      description: 'Dry run. Result is not uploaded to storage',
      default: false,
    }),
    portable: Flags.boolean({
      char: 'p',
      description: 'Include portable build artifacts',
      default: false,
    }),
    dev: Flags.boolean({
      char: 'D',
      description: 'Include development build artifacts',
      default: false,
    })
  }

  async run(): Promise<void> {
    const { args, flags } = await this.parse(Appcast)

    try {
      let storage = new S3Store(flags.endpoint, flags.bucket, flags.key, flags.secret);
      let catalog = new Catalog(storage, flags.branch, flags.dry, false);
      let signer = new Signer(storage, catalog, flags);
      await signer.load();
      await signer.signAll();
      console.log('Done');
    } catch (e) {
      console.error(e);
    }
  }
}
