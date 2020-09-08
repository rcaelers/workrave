import fs from 'fs';
import nunjucks from 'nunjucks';
import yaml from 'js-yaml';
import moment from 'moment';
import wrap from 'word-wrap';

import unified from 'unified';
import markdown from 'remark-parse';

import text from './markdown.js';

class Generator {
  constructor(s) {
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
    this.releases = yaml.safeLoad(fs.readFileSync('../../changes.yaml', 'utf8'));
  }

  async generate(version, template, extra) {
    try {
      if (version) {
        this.releases.releases = this.releases.releases.filter(function(release) {
          return release.version == version;
        });
      }
      let context = { ...extra, ...{ releases: this.releases } };
      return nunjucks.render('templates/' + template + '.tmpl', context);
    } catch (e) {
      console.error(e);
    }
  }
}
export { Generator };
