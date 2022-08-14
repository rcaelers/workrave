import wrap from 'word-wrap';
import { unherit } from 'unherit';
import xtend from 'xtend';

import { Processor } from 'unified'

const text = function (this: Processor, options: any) {
  this.Compiler = class {
    renderer: Renderer;
    tree: any;
    file: any;

    constructor(tree: any, file: any) {
      this.renderer = new Renderer(tree, file, options);
      this.tree = tree;
      this.file = file;
    }

    public compile() {
      var result = this.renderer.one(this.tree);

      if (result.charAt(result.length - 1) !== '\n') {
        result += '\n';
      }
      return result;
    }
  }
}

class Renderer {
  tree: any;
  file: any;
  level: number;
  indent: number;
  width: number;
  dump: boolean;

  constructor(tree: any, file: any, options: any) {
    this.tree = tree;
    this.file = file;

    this.level = 0;
    this.indent = options.indent || 2;
    this.width = options.width || 78;
    this.dump = options.dump || false;
  }

  run(member: Exclude<keyof Renderer, "runOne">, node: any) {
    this[member](node);
  }

  one(node: any) {
    const type: string = node && node.type;

    if (!type) {
      throw new Error(`Node '${node}' does not have a type.`);
    }


    // @ts-expect-error:
    if (!Object.getPrototypeOf(this).hasOwnProperty(type) || typeof this[type] !== 'function') {
      throw new Error(`Cannot render node \`${type}\``);
    }


    // @ts-expect-error:
    return this[type](node);
  }

  all(parent: any) {
    const children = parent && parent.children;
    if (!children) return '';
    return children.map((child: any, index: number) => this.one(child)).join('');
  }

  inline(node: any) {
    return (node && (node.value || ('children' in node && this.all(node)))) || 'x';
  }

  emphasis(node: any) {
    return this.inline(node);
  }

  strong(node: any) {
    return this.inline(node);
  }

  delete(node: any) {
    return this.inline(node);
  }

  inlineCode(node: any) {
    return this.inline(node);
  }

  code(node: any) {
    return '\n' + this.indentify(node.value, this.indent) + '\n\n';
  }

  blockquote(node: any) {
    this.level += this.indent;
    var value = this.all(node);
    this.level -= this.indent;
    return this.indentify(value, this.indent) + '\n';
  }

  break(node: any) {
    return '\n';
  }

  thematicBreak(node: any) {
    return '-'.repeat(this.width - this.level) + '\n';
  }

  paragraph(node: any) {
    var txt = this.all(node);
    txt = wrap(txt, { indent: '', trim: true, width: this.width - this.level }) + '\n';
    return txt;
  }

  heading(node: any) {
    return this.all(node) + '\n';
  }

  link(node: any, href: any) {
    return this.all(node);
  }

  linkReference(node: any) {
    return this.all(node);
  }

  imageReference(node: any) {
    return this.all(node);
  }

  image(node: any) {
    return this.all(node);
  }

  text(node: any) {
    return node.value.replace(/[\n ]+/g, ' ');
  }

  list(node: any) {
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

  listItem(node: any) {
    return this.all(node);
  }

  table(node: any) { }

  tableRow(node: any) { }

  tableCell(node: any) { }

  root(node: any) {
    if (this.dump) {
      console.log(JSON.stringify(node, null, '  '));
    }
    return this.all(node);
  }

  indentify(text: string, indent: number, start: string | undefined = undefined) {
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
