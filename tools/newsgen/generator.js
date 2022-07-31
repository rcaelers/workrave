import nunjucks from 'nunjucks';
import moment from 'moment';
import wrap from 'word-wrap';
import path from 'path';
import { unified } from 'unified';
import markdown from 'remark-parse';
import semver from 'semver';

import text from './markdown.js';

import { dirname } from 'path';
import { fileURLToPath } from 'url';

const __dirname = dirname(fileURLToPath(import.meta.url));

class Generator {
  constructor(news) {
    this.news = news;

    nunjucks
      .configure({
        autoescape: false,
        watch: false,
      })
      .addFilter('data_format', function (date, format) {
        return moment(date).format(format);
      })
      .addFilter('is_string', function (obj) {
        return typeof obj == 'string';
      })
      .addFilter('github', function (str) {
        return str.replace(/#(.+)/, '[#$1](https://github.com/rcaelers/workrave/issues/$1)');
      })
      .addFilter('wrap', function (str, width) {
        return wrap(str, { indent: '', width: width });
      })
      .addFilter('channel', function (str) {
        let pre = semver.prerelease(str);
        if (pre) {
          return pre[0];
        }
        return '';
      })
      .addFilter('text', function (str) {
        return unified().use(markdown).use(text, { width: 78 }).processSync(str).toString().replace(/\n+$/g, '');
      });
  }

  async generate(version, single, template, extra) {
    try {
      if (version) {
        let versionIndex = this.news.releases.findIndex(function (release) {
          return version == release.version;
        });

        if (single && versionIndex + 1 < this.news.releases.length) {
          extra['previous_version'] = this.news.releases[versionIndex + 1].version;
        }

        this.news.releases = this.news.releases.filter(function (release, index) {
          return single ? versionIndex == index : versionIndex >= index;
        });
      }
      let context = { ...extra, ...{ releases: this.news } };
      let template_filename = path.join(__dirname, 'templates', template + '.tmpl');
      return nunjucks.render(template_filename, context);
    } catch (e) {
      console.error(e);
    }
    return '';
  }
}
export { Generator };
