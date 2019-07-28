import AWS from 'aws-sdk';
import fs from 'fs';
import path from 'path';

class Catalog {
  constructor(storage) {
    this.storage = storage;
    this.catalog = null;
  }

  async load() {
    try {
      if (await this.storage.fileExists('catalog.json')) {
        this.catalog = await this.storage.readJson('catalog.json');
        console.log(JSON.stringify(this.catalog, null, '\t'));
      } else {
        this.catalog = {};
      }
      // this.storage.writeJson('catalog.json', this.catalog);
      var content = await this.storage.list('');
      this.parseStorageContent(content);
    } catch (e) {
      console.error(e);
    }
  }

  parseStorageContent(content) {
    for (var i = 0; i < content.length; i++) {
      this.parseFile(content[i]);
    }
  }

  async parseFile(fileInfo) {
    if (fileInfo.Key.endsWith('.json') && fileInfo.Key != 'catalog.json') {
      try {
        let part = null;
        if (await this.storage.fileExists(fileInfo.Key)) {
          part = await this.storage.readJson(fileInfo.Key);
        }
        console.log(fileInfo.Key);
        console.log(JSON.stringify(this.catalog, null, '\t'));
      } catch (e) {
        console.error(e);
      }
    }
  }
}

export { Catalog };
