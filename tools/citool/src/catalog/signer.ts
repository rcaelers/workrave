import path from 'path';
import fsp from 'node:fs/promises';
import as from 'async';

import * as tar from "tar-fs"
import zstd from '@xingrz/cppzst';
import unzipper from 'unzipper';

import { S3Store } from '../common/s3.js';
import { Catalog } from './catalog.js';

class Signer {
  s3store: S3Store;
  catalog: Catalog;

  constructor(s3store: S3Store, catalog: Catalog) {
    this.s3store = s3store;
    this.catalog = catalog;
  }

  async load() {
    try {
      await this.catalog.load();
    } catch (e) {
      console.error(e);
    }
  }

  async save() {
    try {
      await this.catalog.save();
    } catch (e) {
      console.error(e);
    }
  }

  async signDeployArtifact(artifact: any, dir: string) {
    const url = artifact.url.replace(/^.+?[/]/, '');

    console.log('Signing artifact ' + url);
    const stream = await this.s3store.downloadStream(url);

    stream.pipe(zstd.decompressStream())
      .pipe(tar.extract(path.join(dir, artifact.path)))

    console.log('Signing artifact ' + url + '...');
  }


  async signPortableArtifact(artifact: any, dir: string) {
    const url = artifact.url.replace(/^.+?[/]/, '');

    console.log('Signing portable ' + url);
    const stream = await this.s3store.downloadStream(url);

    stream.pipe(unzipper.Extract({ path: path.join(dir, artifact.path) }));

    console.log('Signing portable ' + url + '...');
  }

  async signAll() {
    try {
      const dir = await fsp.mkdtemp(path.join('artifacts'));
      console.log('Downloading artifacts to ' + dir);
      as.forEachLimit(this.catalog.builds(), 4, async (build: any) => {
        as.forEachLimit(build.artifacts, 4, async (artifact: any) => {
          if (artifact.kind == 'deploy') {
            await this.signDeployArtifact(artifact, dir);
          }
          else if (artifact.kind == 'portable') {
            await this.signPortableArtifact(artifact, dir);
          }
        });
      });
    } catch (e) {
      console.error('Exception while signing artifacts: ' + e);
    }
  }
}

export { Signer };
