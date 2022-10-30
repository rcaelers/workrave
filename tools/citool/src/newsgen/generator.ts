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
import { int } from 'aws-sdk/clients/datapipeline.js';

const __dirname = dirname(fileURLToPath(import.meta.url));

class NewsGenerator {
  private news: any;
  private params: any;

  constructor(news: any, params: any) {
    this.news = news;
    this.params = params;

    nunjucks
      .configure(
        path.join(__dirname, '..', '..', 'templates'), {
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
        //return unified().use(markdown).use(text, { width: 78 }).processSync(str).toString().replace(/\n+$/g, '');
        return unified().use(markdown).use(text, { width: 78 }).processSync(str).toString().replace(/\n+$/g, '');
      });
  }

  async generate() {
    try {
      let extra: any = {};
      if (this.params.release || this.params.latest) {
        let versionIndex = 0;
        if (this.params.release) {
          versionIndex = this.news.releases.findIndex((release: any) => {
            return this.params.release == release.version;
          });
        }

        if (this.params.single && versionIndex + 1 < this.news.releases.length) {
          extra['previous_version'] = this.news.releases[versionIndex + 1].version;
        }

        this.news.releases = this.news.releases.filter((release: any, index: int) => {
          return this.params.single ? versionIndex == index : versionIndex >= index;
        });
      }
      let context = {
        ...extra,
        ...{ series: this.params.series, increment: this.params.increment, releases: this.news },
      };
      return nunjucks.render(this.params.template + '.tmpl', context);
    } catch (e) {
      console.error(e);
    }
    return '';
  }
}

export { NewsGenerator };
