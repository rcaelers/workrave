#!/usr/bin/python
#
# Copyright (C) 2007, 2008, 2009, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
# All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
"""
DBUS C++ binding generator
"""

import re
import string
import sys
import os
import xml

from Cheetah.Template import Template
from optparse import OptionParser
from xml.dom.minidom import parse

class NodeBase(object):
    def sig(self):
        return "undefined"

    def symbol(self):
        return "undefined"
    pass


class ArgNode(NodeBase):
    def __init__(self, interface_node):
        NodeBase.__init__(self)
        self.interface_node = interface_node
        self.name = ''
        self.type = ''
        self.direction = ''
        self.hint = []

    def symbol(self):
        return self.csymbol

    def sig(self):
        return self.interface_node.get_type(self.type).sig()

class TypeNode(NodeBase):
    name = "undefined"
    qname = "undefined"
    csymbol = None
    csymbol_internal = None
    type_sig = None
    top_node = None
    condition = None
    
    def __init__(self, csymbol = None, type_sig = None):
        NodeBase.__init__(self)
        self.csymbol = csymbol
        self.csymbol_internal = None
        self.type_sig = type_sig
        self.top_node = None

    def symbol(self):
        return self.csymbol

    def symbol_int(self):
        if self.csymbol_internal:
            return self.csymbol_internal
        else:
            return self.symbol()
    
    def sig(self):
        return self.type_sig

    def value_to_internal(s):
        return s

    def value_from_internal(s):
        return s

class QStringTypeNode(TypeNode):
    def __init__(self):
        TypeNode.__init__(self)
        self.csymbol = "std::string"
        self.csymbol_internal = "QString"
        self.type_sig = 's'
        self.top_node = None

    def to_internal(s):
        return 'QString::fromStdString(' + s + ')'

    def from_internal(s):
        return s + '.toStdString()'
        
class UserTypeNode(TypeNode):
    def __init__(self, top_node):
        TypeNode.__init__(self)
        self.top_node = top_node

    def handle(self, node):
        self.name = node.getAttribute('name')
        self.qname = self.name.replace('.','_')
        self.csymbol = node.getAttribute('csymbol')

        self.top_node.types[self.name] = self
            
    def sig(self):
        print 'Signature of type ' + self.name + ' unknown'
        sys.exit(1)

   
class TopNode(NodeBase):
    def __init__(self, inname, name, backend, header_ext):
        NodeBase.__init__(self)
        self.file_name = inname
        self.namespace = None
        self.namespace_list = []
        self.name = name
        self.guard = ""
        self.backend = backend
        self.header_ext = header_ext
        self.interfaces = []
        self.interfaces_map = {}
        self.types = {}
        self.structs = []
        self.sequences = []
        self.dictionaries = []
        self.enums = []
        self.imports = []

        self.include_filename = self.name + self.header_ext
        self.add_default_types()
        
    def parse(self):
        dom = parse(self.file_name)

        nodelist = dom.getElementsByTagName('unit')
        for node in nodelist:
            self.handle_node(node)

    def handle_node(self, node):
        self.namespace = node.getAttribute('namespace')
        if self.namespace :
            self.namespace_list = self.namespace.split(".")
            self.guard = self.namespace.replace(".", "_") + "_"

        self.guard = self.guard + self.name
        self.guard = self.guard.upper()


        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'interface':
                    p = InterfaceNode(self)
                    p.handle(child)
                    self.interfaces.append(p)
                    self.interfaces_map[p.name] = p
                elif child.nodeName == 'struct':
                    p = StructNode(self)
                    p.handle(child)
                    self.structs.append(p)
                elif child.nodeName == 'sequence':
                    p = SequenceNode(self)
                    p.handle(child)
                    self.sequences.append(p)
                elif child.nodeName == 'dictionary':
                    p = DictionaryNode(self)
                    p.handle(child)
                    self.dictionaries.append(p)
                elif child.nodeName == 'enum':
                    p = EnumNode(self)
                    p.handle(child)
                    self.enums.append(p)
                elif child.nodeName == 'import':
                    p = ImportNode(self)
                    p.handle(child)
                    self.imports.append(p)
                elif child.nodeName == 'type':
                    p = UserTypeNode(self)
                    p.handle(child)

    def add_default_types(self):
        self.types['void']= TypeNode('void','i')
        self.types['int']= TypeNode('int','i')
        self.types['uint8']= TypeNode('uint8_t', 'y')
        self.types['int16']= TypeNode('int16_t','n')
        self.types['uint16']= TypeNode('uint16_t','q')
        self.types['int32']= TypeNode('int32_t','i')
        self.types['uint32']= TypeNode('uint32_t','u')
        self.types['int64']= TypeNode('int64_t','x')
        self.types['uint64']= TypeNode('uint64_t','t')
        self.types['bool']= TypeNode('bool','b')
        self.types['double']= TypeNode('double','d')

        if self.backend == 'qt5':
            self.types['string']= QStringTypeNode()
        else:
            self.types['string']= TypeNode('std::string','s')

    def get_type(self, typename):
        if typename in self.types:
            return self.types[typename]
        else:
            print 'Cannot find type ' + typename
            sys.exit(1)
            
class InterfaceNode(NodeBase):
    def __init__(self, top_node):
        NodeBase.__init__(self)
        self.top_node = top_node

        self.name = None
        self.csymbol = None
        self.qname = None
        self.namespace = None
        self.namespace_list = []
        
        self.methods = []
        self.signals = []

    def handle(self, node):
        self.name = node.getAttribute('name')
        self.csymbol = node.getAttribute('csymbol')
        self.qname = self.name.replace('.','_')
        self.condition = node.getAttribute('condition')

        self.namespace = node.getAttribute('namespace')
        if self.namespace :
            self.namespace_list = self.namespace.split(".")
        
        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'method':
                    p = MethodNode(self)
                    p.handle(child)
                    self.methods.append(p)
                elif child.nodeName == 'signal':
                    p = SignalNode(self)
                    p.handle(child)
                    self.signals.append(p)

    def get_type(self, type):
        return self.top_node.get_type(type)
            
    def symbol(self):
        return self.csymbol

class MethodNode(NodeBase):
    def __init__(self, interface_node):
        NodeBase.__init__(self)
        self.interface_node = interface_node
        self.name = None
        self.csymbol = None
        self.qname = None
        self.condition = ""
        self.params = []
        self.num_in_args = 0
        self.num_out_args = 0
        
    def handle(self, node):
        self.name = node.getAttribute('name')
        self.csymbol = node.getAttribute('csymbol')
        self.qname = self.name.replace('.','_')
        self.condition = node.getAttribute('condition')
        
        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'arg':
                    self.handle_arg(child)

    def handle_arg(self, node):
        p = ArgNode(self.interface_node)
        p.name = node.getAttribute('name')
        p.type = node.getAttribute('type')
        p.direction = node.getAttribute('direction')
        p.bind = node.getAttribute('bind')

        if p.direction == 'in':
            self.num_in_args = self.num_in_args + 1
        if p.direction == 'out':
            self.num_out_args = self.num_out_args + 1
            
        hint = node.getAttribute('hint')
        if hint != None and hint != '':
            p.hint = hint.split(',')
            
        self.params.append(p)

    def introspect_sig(self):
        method_sig = ''
        for p in self.params:
            if p.direction != 'bind':
                param_sig = self.interface_node.get_type(p.type).sig()
                method_sig = method_sig + '%s\\0%s\\0%s\\0' % (p.direction, param_sig, p.name)

        return method_sig

    def symbol(self):
        return self.csymbol

    def sig(self):
        method_sig = ''
        for p in self.params:
            if p.direction != 'bind':
                param_sig = self.interface_node.get_type(p.type).sig()
                method_sig = method_sig + '%s\\0%s\\0%s\\0' % (p.direction, param_sig, p.name)

        return method_sig

    def sig_of_type(self, type):
        method_sig = ''
        for p in self.params:
            if p.direction == type:
                param_sig = self.interface_node.get_type(p.type).sig()
                method_sig = method_sig + '%s' % (param_sig, )

        return '(' + method_sig + ')'

    def return_type(self):
        ret = 'void'
        for p in self.params:
            if 'return' in p.hint:
                ret = p.type
        return ret

    def return_name(self):
        ret = 'ret'
        for p in self.params:
            if 'return' in p.hint:
                ret = p.name
        return ret
        

class SignalNode(NodeBase):
    def __init__(self, interface_node):
        NodeBase.__init__(self)
        self.interface_node = interface_node

    def handle(self, node):
        self.name = node.getAttribute('name')
        self.csymbol = node.getAttribute('csymbol')
        self.qname = self.name.replace('.','_')
        self.params = []
        
        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'arg':
                    self.handle_arg(child)

    def handle_arg(self, node):
        p = ArgNode(self.interface_node)

        p.name = node.getAttribute('name')
        p.type = node.getAttribute('type')

        hint = node.getAttribute('hint')
        if hint != None and hint != '':
            p.hint = hint.split(',')
        
        self.params.append(p)

    def introspect_sig(self):
        method_sig = ''
        for p in self.params:
            param_sig = self.interface_node.get_type(p.type).sig()
            method_sig = method_sig + '%s\\0%s\\0' % (param_sig, p.name)

        return method_sig

    def symbol(self):
        return self.csymbol

    def sig(self):
        method_sig = ''
        for p in self.params:
            param_sig = self.interface_node.get_type(p.type).sig()
            method_sig = method_sig + param_sig

        return '(' + method_sig + ')'

    def return_type(self):
        ret = 'void'
        for p in self.params:
            if 'return' in p.hint:
                ret = p.type
        return ret

    def return_name(self):
        ret = 'ret'
        for p in self.params:
            if 'return' in p.hint:
                ret = p.name
        return ret


class StructNode(TypeNode):
    def __init__(self, top_node):
        TypeNode.__init__(self)
        self.top_node = top_node

    def handle(self, node):
        self.name = node.getAttribute('name')
        self.csymbol = node.getAttribute('csymbol')
        self.condition = node.getAttribute('condition')
        self.qname = self.name.replace('.','_')
        self.fields = []

        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'field':
                    self.handle_field(child)

        self.top_node.types[self.name] = self
        
    def handle_field(self, node):
        arg = ArgNode(self.top_node)
        arg.name = node.getAttribute('name')
        arg.type = node.getAttribute('type')

        self.fields.append(arg)

    def sig(self):
        struct_sig = ''
        for f in self.fields:
            field_sig = self.top_node.get_type(f.type).sig()
            struct_sig =  struct_sig + field_sig

        return '(' + struct_sig + ')'


class SequenceNode(TypeNode):
    def __init__(self, top_node):
        TypeNode.__init__(self)
        self.top_node = top_node

    def handle(self, node):
        self.name = node.getAttribute('name')
        self.condition = node.getAttribute('condition')
        self.csymbol = node.getAttribute('csymbol')
        self.qname = self.name.replace('.','_')
        self.container_type = node.getAttribute('container')
        self.data_type = node.getAttribute('type')
        
        self.top_node.types[self.name] = self

    def sig(self):
        return 'a' + self.top_node.get_type(self.data_type).sig()


class DictionaryNode(TypeNode):
    def __init__(self, top_node):
        TypeNode.__init__(self)
        self.top_node = top_node

    def handle(self, node):
        self.name = node.getAttribute('name')
        self.condition = node.getAttribute('condition')
        self.csymbol = node.getAttribute('csymbol')
        self.qname = self.name.replace('.','_')
        self.key_type = node.getAttribute('key_type')
        self.value_type = node.getAttribute('value_type')

        if self.csymbol == '':
            self.csymbol = 'std::map<%s,%s>' % ( self.top_node.get_type(self.key_type).symbol(),
                                                 self.top_node.get_type(self.value_type).symbol())
        
        self.top_node.types[self.name] = self

    def sig(self):
        return 'e{' + \
               self.top_node.get_type(self.key_type).sig() + \
               self.top_node.get_type(self.value_type).sig() + '}'


class EnumNode(TypeNode):
    def __init__(self, top_node):
        TypeNode.__init__(self)
        self.top_node = top_node
        self.count = 0
        
    def handle(self, node):
        self.name = node.getAttribute('name')
        self.condition = node.getAttribute('condition')
        self.csymbol = node.getAttribute('csymbol')
        self.qname = self.name.replace('.','_')
        self.values = []
        
        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'value':
                    self.handle_value(child)

        self.top_node.types[self.name] = self
            
    def handle_value(self, node):
        arg = ArgNode(self.top_node)

        val = node.getAttribute('value')
        if val != '':
            self.count = int(val)
            
        arg.name = node.getAttribute('name')
        arg.csymbol = node.getAttribute('csymbol')
        arg.value = self.count
        
        self.values.append(arg)

    def sig(self):
        return 's'


class ImportNode(NodeBase):
    def __init__(self, top_node):
        NodeBase.__init__(self)
        self.top_node = top_node
        self.includes = []
        self.namespaces = []
        self.condition = None
        
    def handle(self, node):
        self.condition = node.getAttribute('condition')
        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'include':
                    self.handle_include(child)
                elif child.nodeName == 'namespace':
                    self.handle_namespace(child)

    def handle_include(self, node):
        name = node.getAttribute('name')
        condition = node.getAttribute('condition')
        self.includes.append((name, condition))

    def handle_namespace(self, node):
        name = node.getAttribute('name')
        condition = node.getAttribute('condition')
        self.namespaces.append((name, condition))

   
# Main program

if __name__ == '__main__':
    usage = "usage: %prog [options] <introspect.xml> [out-filename-prefix]"
    parser = OptionParser(usage=usage)
    parser.add_option("-l", "--language",
                      dest="language",
                      help="Generate stubs for this language")
    parser.add_option("-b", "--backend",
                      dest="backend",
                      help="DBUS backend to use: freedesktop or gip")
    parser.add_option("-c", "--client",
                      action="store_true", dest="client",
                      help="Generate client stubs")
    parser.add_option("-s", "--server",
                      action="store_true", dest="server",
                      help="Generate server stubs"
                      )
    (options, args) = parser.parse_args()

    templates = []
    directory = os.path.dirname(sys.argv[0])


    if len(args) < 1 or len(args) > 2:
        parser.error("Expected one or two parameters")
        
    if options.backend not in ["gio", "freedesktop", "qt5"]:
        parser.error("Unsupported backend: " + options.backend)

    if options.language:
        if options.language == 'C':
            header_ext=".h"
        elif options.language == 'C++':
            if options.client:
                templates.append(directory+"/../data/DBus-client-template-" + options.backend + ".cc")
                templates.append(directory+"/../data/DBus-client-template-" + options.backend + ".hh")
            if options.server:
                templates.append(directory+"/../data/DBus-template-" + options.backend + ".cc")
                templates.append(directory+"/../data/DBus-template-" + options.backend + ".hh")
            header_ext=".hh"
        elif options.language == 'xml':
            templates.append(directory+"/../data/DBus-xml.xml")
            header_ext=".xml"
        else:
            parser.error("Unsupported language: " + options.language)
            sys.exit(1)

    if len(templates) == 0:
        parser.error("Specify language")
        sys.exit(1)

    name = None
    if len(args) >= 2:
        name = args[1]
        
    binding = TopNode(args[0], name, options.backend, header_ext)
    binding.parse()

    for template_name in templates:
        t = Template(file=template_name)
        t.model = binding
        s = str(t)
        
        ext = os.path.splitext(template_name)[1]

        f = open(binding.name + ext, 'w+')
        try:
            f.write(s)
        finally:
            f.close()
