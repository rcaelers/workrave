import unified from 'unified';
import markdown from 'remark-parse';
import vfile from 'to-vfile';
import text from './markdown.js';

unified()
  .use(markdown)
  .use(text, { width: 78, dump: true })
  .process(vfile.readSync('markdown.md'), function(err, file) {
    if (err) throw err;
    file.extname = '.txt';
    vfile.writeSync(file);
  });
