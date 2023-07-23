import nunjucks from 'nunjucks';
import moment from 'moment';
import path from 'path';
import semver from 'semver';
import yaml from 'js-yaml';
import { promises as fs } from 'fs';
import as from 'async';

import { dirname } from 'path';
import { fileURLToPath } from 'url';
import { NewsGenerator } from '../newsgen/generator.js';

const __dirname = dirname(fileURLToPath(import.meta.url));

class AppcastGenerator {
  params: any;
  catalog: any;
  news: any;
  input: string;

  tag_to_version(tag: any) {
    return tag
      .replace(/_([0-9])/g, '.$1')
      .replace(/-[0-9]+/g, '')
      .replace(/_/g, '-')
      .replace(/^v/g, '');
  }

  constructor(catalog: any, input: string, params: any) {
    this.catalog = catalog;
    this.params = params;
    this.input = input;
  }

  initNunjucks() {
    nunjucks
      .configure(
        path.join(__dirname, '..', '..', 'templates'), {
        autoescape: false,
        watch: false,
      })
      .addFilter('data_format', (date: string, format: string) => {
        return moment(date).format(format);
      })
      .addFilter('data_format_from_unix', (date: string, format: string) => {
        return moment.unix(+date).format(format);
      })
      .addFilter('channel', (item: any) => {
        const version = this.tag_to_version(item.tag);
        const increment = item.increment;

        if (increment !== '0' && increment !== '') {
          return 'dev';
        }

        const pre = semver.prerelease(version);
        if (pre) {
          return pre[0];
        }
        return '';
      })

      .addFilter('version', (item: any) => {
        return this.tag_to_version(item.tag);
      });
  }

  async generateNotes(release: string) {
    try {

      let params = {
        release: release,
        single: true,
        latest: false,
        template: "github",
        series: 0,
        increment: 0,
      };
      let generator = new NewsGenerator(this.news, params);
      let content = await generator.generate();
      return content;
    } catch (e) {
      console.error(e);
    }
    return '';
  }

  async fixup() {
    if (this.news) {
      const l = async (build: any) => {
        let tag = this.tag_to_version(build.tag);
        let notes = await this.generateNotes(tag);
        if (notes != '') {
          build.notes = notes;
        }
      }
      for (const build of this.catalog.builds) {
        await l(build)
      }
    }
  }

  async generate() {
    try {
      if (this.input) {
        this.news = yaml.load(await fs.readFile(this.input, 'utf8'));
      }
      await this.fixup();
      this.initNunjucks();
      const context = {
        builds: this.catalog.builds,
        environment: this.params.environment,
      };
      return nunjucks.render('appcast.tmpl', context);
    } catch (e) {
      console.error(e);
    }
    return '';
  }
}
export { AppcastGenerator };
