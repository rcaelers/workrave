import path from 'path';
import fs from 'fs';
import format from 'date-fns';
import git from 'isomorphic-git';
import as from 'async';

class Catalog {
  constructor(storage, gitRoot, branch) {
    this.storage = storage;
    this.gitRoot = gitRoot;
    this.branch = branch;
    this.catalog = null;
    this.catalogFilename = path.join(this.branch, 'catalog.json');
  }

  async load() {
    try {
      if (await this.storage.fileExists(this.catalogFilename)) {
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
      await this.storage.writeJson(this.catalogFilename, this.catalog);
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
    let build = this.catalog.builds.find(b => b.id == part.id);
    if (build) {
      build.artifacts = [...build.artifacts, ...part.artifacts];
    } else {
      this.catalog.builds.push(part);
    }
  }

  async mergeCatalogs() {
    try {
      let backupFilename = path.join(this.branch, format.format(new Date(), 'yyyyMMdd-HHmmss') + '-catalog.json');
      await this.storage.writeJson(backupFilename, this.catalog);

      var files = await this.storage.list(this.branch);
      for (var i = 0; i < files.length; i++) {
        let fileInfo = files[i];
        let filename = path.basename(fileInfo.Key);
        let directory = path.dirname(fileInfo.Key);
        if (filename.endsWith('.json') && filename.startsWith('job-catalog-')) {
          let part = await this.storage.readJson(fileInfo.Key);
          this.mergeBuild(part.builds[0]);
          let backupFilename = path.join(directory, '.' + filename);
          await this.storage.writeJson(backupFilename, part);
          await this.storage.deleteObject(fileInfo.Key);
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
          ref: hash
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
      let newBuilds = await as.filterLimit(this.catalog.builds, 4, async build => {
        let newArtifacts = await as.filterLimit(build.artifacts, 4, async artifact => {
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
      this.catalog.builds.forEach(build => {
        build.artifacts.forEach(artifact => {
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
