import wrap from 'word-wrap';
import unherit from 'unherit';
import xtend from 'xtend';

function text(options) {
  var Local = unherit(createCompiler(options || {}));
  Local.prototype.options = xtend(Local.prototype.options, this.data('settings'), options);
  this.Compiler = Local;
}

function createCompiler(defaults) {
  Compiler.prototype.compile = compile;

  return Compiler;

  function Compiler(tree, file) {
    this.renderer = new Renderer(tree, file, this.options);
    this.tree = tree;
    this.file = file;
  }

  function compile() {
    var result = this.renderer.one(this.tree);

    if (result.charAt(result.length - 1) !== '\n') {
      result += '\n';
    }

    return result;
  }
}

class Renderer {
  constructor(tree, file, options) {
    this.tree = tree;
    this.file = file;

    this.level = 0;
    this.indent = options.indent || 2;
    this.width = options.width || 78;
    this.dump = options.dump || false;
  }

  one(node) {
    const type = node && node.type;

    if (!type) {
      throw new Error(`Node '${node}' does not have a type.`);
    }

    if (!Object.getPrototypeOf(this).hasOwnProperty(type) || typeof this[type] !== 'function') {
      throw new Error(`Cannot render node \`${type}\``);
    }

    return this[type](node);
  }

  all(parent) {
    const children = parent && parent.children;
    if (!children) return '';
    return children.map((child, index) => this.one(child)).join('');
  }

  inline(node) {
    return (node && (node.value || ('children' in node && this.all(node)))) || 'x';
  }

  emphasis(node) {
    return this.inline(node);
  }

  strong(node) {
    return this.inline(node);
  }

  delete(node) {
    return this.inline(node);
  }

  inlineCode(node) {
    return this.inline(node);
  }

  code(node) {
    return '\n' + this.indentify(node.value, this.indent) + '\n\n';
  }

  blockquote(node) {
    this.level += this.indent;
    var value = this.all(node);
    this.level -= this.indent;
    return this.indentify(value, this.indent) + '\n';
  }

  break(node) {
    return '\n';
  }

  thematicBreak(node) {
    return '-'.repeat(this.width - this.level) + '\n';
  }

  paragraph(node) {
    var txt = this.all(node);
    txt = wrap(txt, { indent: '', trim: true, width: this.width - this.level }) + '\n';
    return txt;
  }

  heading(node) {
    return this.all(node) + '\n';
  }

  link(node, href) {
    return this.all(node);
  }

  linkReference(node) {
    return this.all(node);
  }

  imageReference(node) {
    return this.all(node);
  }

  image(node) {
    return this.all(node);
  }

  text(node) {
    return node.value.replace(/[\n ]+/g, ' ');
  }

  list(node) {
    var start = node.start;
    var children = node.children;
    var length = children.length;

    var indent;
    if (start) {
      indent = Math.ceil(Math.log10(start + length)) + 2;
    } else {
      indent = this.indent;
    }
    this.level += indent;

    var values = [];
    var index = -1;
    while (++index < length) {
      var txt = this.all(children[index]).replace(/^\n|\n\n$|\n$/g, '');
      var prefix = '- ';
      if (start) {
        prefix = '' + (start + index) + '. ';
      }

      txt = this.indentify(txt, indent, prefix);
      values.push(txt);
    }

    this.level -= indent;

    return values.join('\n') + '\n\n';
  }

  listItem(node) {
    return this.all(node);
  }

  table(node) {}

  tableRow(node) {}

  tableCell(node) {}

  root(node) {
    if (this.dump) {
      console.log(JSON.stringify(node, null, '  '));
    }
    return this.all(node);
  }

  indentify(text, indent, start = undefined) {
    if (!text) return text;
    if (start == undefined) {
      start = ' '.repeat(indent);
    } else {
      start += ' '.repeat(indent - start.length);
    }
    return start + text.split('\n').join('\n' + ' '.repeat(indent));
  }
}

export default text;
export { text };
