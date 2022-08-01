import path from 'path';
import fs from 'fs';
import format from 'date-fns';
import git from 'isomorphic-git';
import as from 'async';
import mergician from 'mergician';

class Catalog {
  constructor(storage, gitRoot, branch, dry, regenerate) {
    this.storage = storage;
    this.gitRoot = gitRoot;
    this.branch = branch;
    this.dry = dry;
    this.regenerate = regenerate;
    this.catalog = null;
    this.catalogFilename = path.join(this.branch, 'catalog.json');
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
    try {
      //console.log(JSON.stringify(this.catalog, null, '\t'));
      if (this.dry) {
        console.log('Dry run: writeJson(' + this.catalogFilename + '):');
        console.log(JSON.stringify(this.catalog, null, '\t'));
      } else {
        console.log('Saving:' + this.catalogFilename);
        await this.storage.writeJson(this.catalogFilename, this.catalog);
      }
    } catch (e) {
      console.error(e);
    }
  }

  async process() {
    try {
      await this.mergeCatalogs();
      await this.fixups();
      await this.sortCatalog();
      await this.removeOrphans();
      await this.updateGitLogs();
    } catch (e) {
      console.error(e);
    }
  }

  mergeBuild(part) {
    let build = this.catalog.builds.find((b) => b.id == part.id);
    let buildIndex = this.catalog.builds.findIndex((b) => b.id == part.id);
    if (build) {
      // build.artifacts = [...build.artifacts, ...part.artifacts];
      const mergedObj = mergician({
        appendArrays: true,
        beforeEach({ depth, key, srcObj, srcVal, targetObj, targetVal }) {
          if (key == 'notes') {
            return srcVal == null || srcVal.length == 0 ? targetVal : srcVal;
          }
          return undefined;
        },
      })(build, part);
      this.catalog.builds[buildIndex] = mergedObj;
    } else {
      this.catalog.builds.push(part);
    }
  }

  isCatalog(filename) {
    if (!filename.endsWith('.json')) {
      return false;
    }
    if (filename.startsWith('job-catalog-')) {
      return true;
    }
    if (this.regenerate && filename.startsWith('.job-catalog-')) {
      return true;
    }
    return false;
  }

  async mergeCatalogs() {
    try {
      let backupFilename = path.join(this.branch, format.format(new Date(), 'yyyyMMdd-HHmmss') + '-catalog.json');
      if (this.dry) {
        console.log('Dry run: writeJson(' + backupFilename + '):');
      } else {
        await this.storage.writeJson(backupFilename, this.catalog);
      }

      var files = await this.storage.list(this.branch);
      for (var i = 0; i < files.length; i++) {
        console.log('Processing ' + files[i]);
        let fileInfo = files[i];
        let filename = path.basename(fileInfo.Key);
        let directory = path.dirname(fileInfo.Key);
        console.log('Processing ' + filename);
        if (this.isCatalog(filename)) {
          let part = await this.storage.readJson(fileInfo.Key);
          this.mergeBuild(part.builds[0]);
          let backupFilename = path.join(directory, '.' + filename);

          if (this.dry || this.regenerate) {
            console.log('Dry run: deleteObject(' + backupFilename + ')');
          } else {
            await this.storage.writeJson(backupFilename, part);
            await this.storage.deleteObject(fileInfo.Key);
          }
        }
      }
    } catch (e) {
      console.error(e);
    }
  }

  async updateGitLogs() {
    try {
      if (this.catalog.builds.length > 1) {
        const hash = this.catalog.builds[0].hash;

        const commitList = await git.log({
          fs,
          dir: this.gitRoot,
          ref: hash,
        });

        var build_index = 0;
        var history_index = 0;

        this.catalog.builds[build_index].commits = [];

        while (build_index < this.catalog.builds.length - 1 && history_index < commitList.length) {
          const h = commitList[history_index];
          if (h.oid.startsWith(this.catalog.builds[build_index + 1].hash)) {
            build_index = build_index + 1;
            this.catalog.builds[build_index].commits = [];
          } else {
            history_index = history_index + 1;
            this.catalog.builds[build_index].commits.push(h);
          }
        }
      }
    } catch (e) {
      console.error(e);
    }
  }

  async removeOrphans() {
    try {
      let newBuilds = await as.filterLimit(this.catalog.builds, 4, async (build) => {
        let newArtifacts = await as.filterLimit(build.artifacts, 4, async (artifact) => {
          let filename = artifact.url.replace('snapshots/', '');
          let exists = false;
          try {
            console.log('Checking for artifact ' + filename);
            exists = await this.storage.fileExists(filename);
          } catch (e) {
            console.log('Exception while checking for artifact ' + filename + ': ' + e);
          }
          if (!exists) {
            console.log('Artifact ' + filename + ' disappeared');
            return false;
          }
          return true;
        });
        if (newArtifacts.length == 0) {
          console.log('Build ' + build.id + ' does not have any artifacts');
          return false;
        }
        return true;
      });
      this.catalog.builds = newBuilds;
    } catch (e) {
      console.error(e);
    }
  }

  async fixups() {
    try {
      this.catalog.builds.forEach((build) => {
        build.artifacts.forEach((artifact) => {
          artifact.url = artifact.url.replace('/workspace/source/_deploy', '');
          artifact.filename = artifact.filename.replace('/workspace/source/_deploy', '');
          artifact.platform = artifact.platform.replace('win32', 'windows');
          artifact.url = artifact.url.replace('//', '/');
          artifact.filename = artifact.filename.replace('//', '/');
        });
      });
    } catch (e) {
      console.error(e);
    }
  }

  async sortCatalog() {
    this.catalog.builds = this.catalog.builds.sort((a, b) => (Date.parse(a.date) > Date.parse(b.date) ? -1 : 1));
  }
}
export { Catalog };
