#!/usr/bin/python
#
# Copyright (C) 2007, 2008, 2009, 2011 Rob Caelers <robc@krandor.nl>
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
    pass


class ArgNode(NodeBase):
    def __init__(self, interface_node):
        NodeBase.__init__(self)
        self.interface_node = interface_node
        self.name = ''
        self.type = ''
        self.ext_type = ''
        self.direction = ''
        self.hint = []

    def sig(self):
        return self.interface_node.type2sig(self.ext_type)


class DefaultTypeNode(NodeBase):
    def __init__(self, csymbol, type_sig):
        NodeBase.__init__(self)
        self.csymbol = csymbol
        self.type_sig = type_sig

    def sig(self):
        return self.type_sig

    
class TopNode(NodeBase):
    def __init__(self, name):
        NodeBase.__init__(self)
        self.file_name = name
        self.name = None
        self.interfaces = []
        
    def parse(self):
        dom = parse(self.file_name)

        nodelist = dom.getElementsByTagName('unit')
        for node in nodelist:
            self.handle_node(node)

    def handle_node(self, node):
        self.name = node.getAttribute('name')

        nodelist = node.getElementsByTagName('interface')
        for child in nodelist:
            p = InterfaceNode(self)
            p.handle(child)
            self.interfaces.append(p)


class InterfaceNode(NodeBase):
    def __init__(self, parent):
        NodeBase.__init__(self)
        self.parent = parent

        self.types = {}

        self.name = None
        self.csymbol = None
        self.qname = None
        
        self.methods = []
        self.signals = []
        self.structs = []
        self.sequences = []
        self.dictionaries = []
        self.enums = []
        self.imports = []

        self.add_default_types()
            
    def handle(self, node):
        self.name = node.getAttribute('name')
        self.csymbol = node.getAttribute('csymbol')
        self.qname = self.name.replace('.','_')
        self.condition = node.getAttribute('condition')
        
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
                    p = TypeNode(self)
                    p.handle(child)

    def add_default_types(self):
        self.types['void']= DefaultTypeNode('void','i')
        self.types['int']= DefaultTypeNode('int','i')
        self.types['uint8']= DefaultTypeNode('guint8', 'y')
        self.types['int16']= DefaultTypeNode('gint16','n')
        self.types['uint16']= DefaultTypeNode('guint16','q')
        self.types['int32']= DefaultTypeNode('gint32','i')
        self.types['uint32']= DefaultTypeNode('guint32','u')
        self.types['int64']= DefaultTypeNode('gint64','x')
        self.types['uint64']= DefaultTypeNode('guint64','t')
        self.types['string']= DefaultTypeNode('std::string','s')
        self.types['bool']= DefaultTypeNode('bool','b')
        self.types['double']= DefaultTypeNode('double','d')
        
    def type2csymbol(self, type):
        if type in self.types:
            return self.types[type].csymbol
        else:
            print 'C type of type ' + type + ' unknown'
            sys.exit(1)

    def type2sig(self, type):
        if type in self.types:
            return self.types[type].sig()
        else:
            print 'Signature of type ' + type + ' unknown'
            sys.exit(1)


class MethodNode(NodeBase):
    def __init__(self, parent):
        NodeBase.__init__(self)
        self.parent = parent
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
        p = ArgNode(self.parent)
        p.name = node.getAttribute('name')
        p.type = node.getAttribute('type')
        p.ext_type = node.getAttribute('ext_type')
        p.direction = node.getAttribute('direction')
        p.bind = node.getAttribute('bind')

        if p.ext_type == '':
            p.ext_type = p.type;

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
                param_sig = self.parent.type2sig(p.ext_type)
                method_sig = method_sig + '%s\\0%s\\0%s\\0' % (p.direction, param_sig, p.name)

        return method_sig

    def sig(self):
        method_sig = ''
        for p in self.params:
            if p.direction != 'bind':
                param_sig = self.parent.type2sig(p.ext_type)
                method_sig = method_sig + '%s\\0%s\\0%s\\0' % (p.direction, param_sig, p.name)

        return method_sig

    def sig_of_type(self, type):
        method_sig = ''
        for p in self.params:
            if p.direction == type:
                param_sig = self.parent.type2sig(p.ext_type)
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
    def __init__(self, parent):
        NodeBase.__init__(self)
        self.parent = parent

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
        p = ArgNode(self.parent)

        p.name = node.getAttribute('name')
        p.type = node.getAttribute('type')
        p.ext_type = node.getAttribute('ext_type')

        if p.ext_type == '':
            p.ext_type = p.type;

        hint = node.getAttribute('hint')
        if hint != None and hint != '':
            p.hint = hint.split(',')
        
        self.params.append(p)

    def introspect_sig(self):
        method_sig = ''
        for p in self.params:
            param_sig = self.parent.type2sig(p.ext_type)
            method_sig = method_sig + '%s\\0%s\\0' % (param_sig, p.name)

        return method_sig

    def sig(self):
        method_sig = ''
        for p in self.params:
            param_sig = self.parent.type2sig(p.ext_type)
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


class StructNode(NodeBase):
    def __init__(self, parent):
        NodeBase.__init__(self)
        self.parent = parent

    def handle(self, node):
        self.name = node.getAttribute('name')
        self.csymbol = node.getAttribute('csymbol')
        self.qname = self.name.replace('.','_')
        self.fields = []

        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'field':
                    self.handle_field(child)

        self.parent.types[self.name] = self
        
    def handle_field(self, node):
        arg = ArgNode(self.parent)
        arg.name = node.getAttribute('name')
        arg.type = node.getAttribute('type')
        arg.ext_type = node.getAttribute('ext_type')

        if arg.ext_type == '':
            arg.ext_type = arg.type;

        self.fields.append(arg)

    def sig(self):
        struct_sig = ''
        for f in self.fields:
            field_sig = self.parent.type2sig(f.ext_type)
            struct_sig =  struct_sig + field_sig

        return '(' + struct_sig + ')'


class SequenceNode(NodeBase):
    def __init__(self, parent):
        NodeBase.__init__(self)
        self.parent = parent

    def handle(self, node):
        self.name = node.getAttribute('name')
        self.csymbol = node.getAttribute('csymbol')
        self.qname = self.name.replace('.','_')
        self.container_type = node.getAttribute('container')
        self.data_type = node.getAttribute('type')
        
        self.parent.types[self.name] = self

    def sig(self):
        return 'a' + self.parent.type2sig(self.data_type)


class DictionaryNode(NodeBase):
    def __init__(self, parent):
        NodeBase.__init__(self)
        self.parent = parent

    def handle(self, node):
        self.name = node.getAttribute('name')
        self.csymbol = node.getAttribute('csymbol')
        self.qname = self.name.replace('.','_')
        self.key_type = node.getAttribute('key_type')
        self.value_type = node.getAttribute('value_type')

        if self.csymbol == '':
            self.csymbol = 'std::map<%s,%s>' % ( self.parent.type2csymbol(self.key_type),
                                                 self.parent.type2csymbol(self.value_type))
        
        self.parent.types[self.name] = self

    def sig(self):
        return 'e{' + \
               self.parent.type2sig(self.key_type) + \
               self.parent.type2sig(self.value_type) + '}'


class EnumNode(NodeBase):
    def __init__(self, parent):
        NodeBase.__init__(self)
        self.parent = parent
        self.count = 0
        
    def handle(self, node):
        self.name = node.getAttribute('name')
        self.csymbol = node.getAttribute('csymbol')
        self.qname = self.name.replace('.','_')
        self.values = []
        
        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'value':
                    self.handle_value(child)

        self.parent.types[self.name] = self
            
    def handle_value(self, node):
        arg = ArgNode(self.parent)

        val = node.getAttribute('value')
        if val != '':
            self.count = int(val)
            
        arg.name = node.getAttribute('name')
        arg.csymbol = node.getAttribute('csymbol')
        arg.value = self.count
        
        self.values.append(arg)

    def sig(self):
        return 's'


class TypeNode(NodeBase):
    def __init__(self, parent):
        NodeBase.__init__(self)
        self.parent = parent
        self.count = 0
        
    def handle(self, node):
        self.name = node.getAttribute('name')
        self.csymbol = node.getAttribute('csymbol')
        self.qname = self.name.replace('.','_')

        self.parent.types[self.name] = self
            
    def sig(self):
        print 'Signature of type ' + self.name + ' unknown'
        sys.exit(1)


class ImportNode(NodeBase):
    def __init__(self, parent):
        NodeBase.__init__(self)
        self.parent = parent
        self.includes = []
        self.namespaces = []
        
    def handle(self, node):
        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'include':
                    self.handle_include(child)
                elif child.nodeName == 'namespace':
                    self.handle_namespace(child)

    def handle_include(self, node):
        self.includes.append(node.getAttribute('name'))

    def handle_namespace(self, node):
        self.namespaces.append(node.getAttribute('name'))

   
# Main program

if __name__ == '__main__':
    usage = "usage: %prog [options] <introspect.xml>"
    parser = OptionParser(usage=usage)
    parser.add_option("-l", "--language",
                      dest="language",
                      help="Generate stubs for this language")
    parser.add_option("-g", "--gio",
                      action="store_true", dest="gio",
                      help="Generate GIO based stubs")
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

    brand = "freedesktop"
    if options.gio:
        brand = "gio"

    if options.language:
        if options.language == 'C':
            header_ext=".h"
        elif options.language == 'C++':
            if options.client:
                templates.append(directory+"/DBus-client-template-" + brand + ".cc")
                templates.append(directory+"/DBus-client-template-" + brand + ".hh")
            if options.server:
                templates.append(directory+"/DBus-template-" + brand + ".cc")
                templates.append(directory+"/DBus-template-" + brand + ".hh")
            header_ext=".hh"
        elif options.language == 'xml':
            templates.append(directory+"/DBus-xml.xml")
            header_ext=".xml"
        else:
            parser.error("Unsupported language: " + options.language)
            sys.exit(1)

    if len(templates) == 0:
        parser.error("Specify language")
        sys.exit(1)
            
    binding = TopNode(args[0])
    binding.parse()

    binding.include_filename = binding.name + header_ext
    
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
