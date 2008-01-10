#!/usr/bin/python
#
# Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.nl>
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
# $Id$
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

class ArgNode(object):
    def __init__(self):
        self.name = ''
        self.type = ''
        self.direction = ''
        self.hint = ''


class BindingNode(object):
    def __init__(self, name):
        self.file_name = name
        self.interfaces = []
        
    def parse(self):
        dom = parse(self.file_name)

        nodelist = dom.getElementsByTagName('node')
        for node in nodelist:
            self.handle_node(node)

    def handle_node(self, node):
        nodelist = node.getElementsByTagName('interface')
        for child in nodelist:
            p = InterfaceNode(self)
            p.handle(child)
            self.interfaces.append(p)


class InterfaceNode(object):
    def __init__(self, parent):
        self.parent = parent

        self.name = None
        self.csymbol = None
        self.qname = None
        
        self.methods = []
        self.signals = []
        self.structs = []
        self.sequences = []
        self.enums = []
        self.types = {}
        self.imports = []
        
    def handle(self, node):
        self.name = node.getAttribute('name')
        self.csymbol = node.getAttribute('csymbol')
        self.qname = self.name.replace('.','_')
        
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
                elif child.nodeName == 'enum':
                    p = EnumNode(self)
                    p.handle(child)
                    self.enums.append(p)
                elif child.nodeName == 'import':
                    p = ImportNode(self)
                    p.handle(child)
                    self.imports.append(p)

    def type2csymbol(self, type):
        if type == 'int':
            return 'int'
        elif type == 'int32':
            return 'gint32'
        elif type == 'uint32':
            return 'guint32'
        elif type == 'int64':
            return 'gint64'
        elif type == 'uint64':
            return 'guint64'
        elif type == 'string':
            return 'std::string'
        elif type == 'bool':
            return 'bool'
        if type in self.types:
            return self.types[type].csymbol
        else:
            return type

    def type2sig(self, type):
        if type == 'int':
            return 'i'
        elif type == 'int32':
            return 'i'
        elif type == 'uint32':
            return 'u'
        elif type == 'int64':
            return 'x'
        elif type == 'uint64':
            return 't'
        elif type == 'string':
            return 's'
        elif type == 'bool':
            return 'b'
        elif type in self.types:
            return self.types[type].sig()
        else:
            return ''


class MethodNode(object):
    def __init__(self, parent):
        object.__init__(self)
        self.parent = parent

        self.name = None
        self.csymbol = None
        self.qname = None
        self.condition = ""
        self.params = []

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
        p = ArgNode()
        p.name = node.getAttribute('name')
        p.type = node.getAttribute('type')
        p.direction = node.getAttribute('direction')
        p.hint = node.getAttribute('hint')
        
        self.params.append(p)

    def sig(self):
        method_sig = ''
        for p in self.params:
            param_sig = self.parent.type2sig(p.type)
            method_sig = method_sig + '%s\\0%s\\0%s\\0' % (p.direction, param_sig, p.name)

        return method_sig

    def return_type(self):
        ret = 'void'
        for p in self.params:
            if p.hint == 'return':
                ret = p.type
        return ret

    def return_name(self):
        ret = 'ret'
        for p in self.params:
            if p.hint == 'return':
                ret = p.name
        return ret
        


class SignalNode(object):
    def __init__(self, parent):
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
        p = ArgNode()

        p.name = node.getAttribute('name')
        p.type = node.getAttribute('type')
        
        self.params.append(p)


    def sig(self):
        method_sig = ''
        for p in self.params:
            param_sig = self.parent.type2sig(p.type)
            method_sig = method_sig + '%s\\0%s\\0' % (param_sig, p.name)

        return method_sig

    def return_type(self):
        ret = 'void'
        for p in self.params:
            if p.hint == 'return':
                ret = p.type
        return ret

    def return_name(self):
        ret = 'ret'
        for p in self.params:
            if p.hint == 'return':
                ret = p.name
        return ret


class StructNode(object):
    def __init__(self, parent):
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
        arg = ArgNode()
        arg.name = node.getAttribute('name')
        arg.type = node.getAttribute('type')

        self.fields.append(p)


    def sig(self):
        struct_sig = ''
        for f in self.fields:
            field_sig = self.parent.type2sig(f.type)
            struct_sig =  struct_sig + field_sig

        return '(' + struct_sig + ')'



class SequenceNode(object):
    def __init__(self, parent):
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



class EnumNode(object):
    def __init__(self, parent):
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
        arg = ArgNode()

        val = node.getAttribute('value')
        if val != '':
            self.count = int(val)
            
        arg.name = node.getAttribute('name')
        arg.csymbol = node.getAttribute('csymbol')
        arg.value = self.count
        
        self.values.append(arg)

    def sig(self):
        return 's'


class ImportNode(object):
    def __init__(self, parent):
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
    usage = "usage: %prog [options] <introspect.xml> <prefix>"
    parser = OptionParser(usage=usage)
    parser.add_option("-l", "--language",
                      dest="language",
                      help="Generate stubs for this language",
                      )

    (options, args) = parser.parse_args()

    templates = []
    directory = os.path.dirname(sys.argv[0])

    if options.language:
        if options.language == 'C':
            templates.append(directory+"/DBus-template.c")
            templates.append(directory+"/DBus-template.h")
            header_ext=".h"
        elif options.language == 'C++':
            templates.append(directory+"/DBus-template.cc")
            templates.append(directory+"/DBus-template.hh")
            header_ext=".hh"
        else:
            parser.error("Unsupported language: " + options.language)
            sys.exit(1)

    if len(templates) == 0:
        parser.error("Specify language")
        sys.exit(1)
            
    binding = BindingNode(args[0])
    binding.parse()

    binding.prefix = args[1]
    binding.include_filename = binding.prefix + header_ext

    for template_name in templates:
        t = Template(file=template_name)
        t.model = binding
        s = str(t)
        
        ext = os.path.splitext(template_name)[1]

        f = open(args[1] + ext, 'w+')
        try:
            f.write(s)
        finally:
            f.close()
