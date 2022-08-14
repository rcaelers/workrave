import { promises as fs } from 'fs';

import { S3Store } from '../../common/s3.js';
import { Catalog } from '../../catalog/catalog.js';
import { Signer } from '../../catalog/signer.js';

import { Command, Flags } from '@oclif/core'

export default class Appcast extends Command {
  static description = 'sign artifacts'

  static examples = [
    `$ citool sign
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
      description: 'secret',
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
    dry: Flags.boolean({
      char: 'd',
      description: 'Dry run. Result is not uploaded to storage',
      default: false,
    })
  }

  async run(): Promise<void> {
    const { args, flags } = await this.parse(Appcast)

    console.log('Signing artifacts :' + JSON.stringify(flags, null, '\t'));

    try {
      let storage = new S3Store(flags.endpoint, flags.bucket, flags.key, flags.secret);
      let catalog = new Catalog(storage, flags.branch, flags.dry, false);
      let signer = new Signer(storage, catalog);
      await signer.load();
      await signer.signAll();
      console.log('Done');
    } catch (e) {
      console.error(e);
    }
  }
}
