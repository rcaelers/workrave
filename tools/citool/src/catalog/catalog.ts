import path from 'path';

import { S3Store } from '../common/s3.js';

class Catalog {
  private storage: S3Store;
  private dry: boolean;
  private regenerate: boolean;
  private catalog: any;
  private catalogFilename: string;

  constructor(storage: S3Store, branch: string, dry: boolean, regenerate: boolean) {
    this.storage = storage;
    this.dry = dry;
    this.regenerate = regenerate;

    this.catalog = null;
    this.catalogFilename = path.posix.join(branch, 'catalog.json');
  }

  async load() {
    try {
      if (!this.regenerate && (await this.storage.fileExists(this.catalogFilename))) {
        this.catalog = await this.storage.readJson(this.catalogFilename);
      } else {
        this.catalog = {};
      }
      if (!this.catalog.builds) {
        this.catalog.builds = [];
      }
    } catch (e) {
      console.error(e);
    }
  }

  async save() {
    this.saveAs(this.catalogFilename);
  }

  async saveAs(filename: string) {
    try {
      if (this.dry) {
        console.log('Dry run: would save' + filename + ':');
        console.log(JSON.stringify(this.catalog, null, '\t'));
      } else {
        console.log('Saving:' + filename);
        await this.storage.writeJson(filename, this.catalog);
      }
    } catch (e) {
      console.error(e);
    }
  }

  builds(): any {
    return this.catalog.builds;
  }

  setBuilds(builds: any) {
    this.catalog.builds = builds;
  }
}

export { Catalog };
