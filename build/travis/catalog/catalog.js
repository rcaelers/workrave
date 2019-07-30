import merge from 'deepmerge';
import path from 'path';
import format from 'date-fns';

class Catalog {
  constructor(storage, prefix) {
    this.storage = storage;
    this.prefix = prefix;
    this.catalog = null;
    this.catalogFilename = path.join(this.prefix, 'catalog.json');
  }

  async load() {
    try {
      if (await this.storage.fileExists(this.catalogFilename)) {
        this.catalog = await this.storage.readJson(this.catalogFilename);
      } else {
        this.catalog = {};
      }
    } catch (e) {
      console.error(e);
    }
  }

  async save() {
    try {
      //console.log(JSON.stringify(this.catalog, null, '\t'));
      await this.storage.writeJson(this.catalogFilename, this.catalog);
    } catch (e) {
      console.error(e);
    }
  }

  async process() {
    try {
      await this.mergeCatalogs();
      await this.fixups();
    } catch (e) {
      console.error(e);
    }
  }

  async mergeCatalogs() {
    try {
      let backupFilename = path.join(this.prefix, format.format(new Date(), 'yyyyMMdd-HHmmss') + '-catalog.json');
      await this.storage.writeJson(backupFilename, this.catalog);

      var files = await this.storage.list(this.prefix);
      for (var i = 0; i < files.length; i++) {
        let fileInfo = files[i];
        let filename = path.basename(fileInfo.Key);
        let directory = path.dirname(fileInfo.Key);
        if (filename.endsWith('.json') && filename.startsWith('job-catalog-')) {
          let part = await this.storage.readJson(fileInfo.Key);
          this.catalog = merge(this.catalog, part);
          let backupFilename = path.join(directory, '.' + filename);
          await this.storage.writeJson(backupFilename, part);
          await this.storage.deleteObject(fileInfo.Key);
        }
      }

      this.catalog.builds = this.catalog.builds.sort((a, b) => (Date.parse(a.date) > Date.parse(b.date) ? -1 : 1));
    } catch (e) {
      console.error(e);
    }
  }

  async fixups() {
    try {
      this.catalog.builds.forEach(build => {
        build.artifacts.forEach(artifact => {
          artifact.url = artifact.url.replace('/workspace/source/_deploy', '');
          artifact.filename = artifact.filename.replace('/workspace/source/_deploy', '');
          artifact.platform = artifact.platform.replace('win32', 'windows');
        });
      });

      console.log(JSON.stringify(this.catalog, null, '\t'));
    } catch (e) {
      console.error(e);
    }
  }
}
export { Catalog };
