import nunjucks from 'nunjucks';
import moment from 'moment';
import wrap from 'word-wrap';
import path from 'path';
import unified from 'unified';
import markdown from 'remark-parse';

import text from './markdown.js';

import { dirname } from 'path';
import { fileURLToPath } from 'url';

const __dirname = dirname(fileURLToPath(import.meta.url));

class Generator {
  constructor(releases) {
    this.releases = releases;
    nunjucks
      .configure({
        autoescape: false,
        watch: false
      })
      .addFilter('data_format', function(date, format) {
        return moment(date).format(format);
      })
      .addFilter('is_string', function(obj) {
        return typeof obj == 'string';
      })
      .addFilter('github', function(str) {
        return str.replace(/#(.+)/, '[#$1](https://github.com/rcaelers/workrave/issues/$1)');
      })
      .addFilter('wrap', function(str, width) {
        return wrap(str, { indent: '', width: width });
      })
      .addFilter('text', function(str) {
        return unified()
          .use(markdown)
          .use(text, { width: 78 })
          .processSync(str)
          .toString()
          .replace(/\n+$/g, '');
      });
  }

  async generate(version, template, extra) {
    try {
      if (version) {
        this.releases.releases = this.releases.releases.filter(function(release) {
          return release.version == version;
        });
      }
      let context = { ...extra, ...{ releases: this.releases } };
      let template_filename = path.join(__dirname, 'templates', template + '.tmpl');
      return nunjucks.render(template_filename, context);
    } catch (e) {
      console.error(e);
    }
  }
}
export { Generator };
