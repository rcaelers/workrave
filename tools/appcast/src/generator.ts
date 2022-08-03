import nunjucks from 'nunjucks';
import moment from 'moment';
import path from 'path';
import semver from 'semver';

import { dirname } from 'path';
import { fileURLToPath } from 'url';

const __dirname = dirname(fileURLToPath(import.meta.url));

class Generator {
  params: any;
  catalog: any;

  constructor(catalog: any, params: any) {
    this.catalog = catalog;
    this.params = params;

    nunjucks
      .configure({
        autoescape: false,
        watch: false
      })
      .addFilter('data_format', function (date: string, format: string) {
        return moment(date).format(format);
      })
      .addFilter('channel', function (str: string) {
        const pre = semver.prerelease(str);
        if (pre) {
          return pre[0];
        }
        return '';
      });
  }

  async generate() {
    try {
      const context = {
        builds: this.catalog.builds
      };
      const template_filename: string = path.join(
        __dirname,
        '..',
        'templates',
        'appcast.tmpl'
      );
      return nunjucks.render(template_filename, context);
    } catch (e) {
      console.error(e);
    }
    return '';
  }
}
export { Generator };
