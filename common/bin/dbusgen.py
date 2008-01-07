#!/usr/bin/python
#
# Copyright (C) 2007 Rob Caelers <robc@krandor.nl>
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

class Model(object):
    def __init__(self):
        self.interfaces = []

class InterfaceModel(object):
    def __init__(self):
        self.name = ''
        self.methods = []
        self.signals = []
        self.structs = []
        self.sequences = []
        self.enums = []
        self.types = {}
        self.includes = []
        self.namespaces = []
        
    def type2csymbol(self, type):
        if type in self.types:
            return self.types[type].csymbol
        else:
            return type

    def type2sig(self, type):
        if type == 'int':
            return 'i'
        elif type == 'gint32':
            return 'i'
        elif type == 'guint32':
            return 'u'
        elif type == 'gint64':
            return 'x'
        elif type == 'guint64':
            return 't'
        elif type == 'string':
            return 's'
        elif type == 'bool':
            return 'b'
        elif type in self.types:
            return self.types[type].sig()
        else:
            return ''
        
class MethodModel(object):
    def __init__(self, parent):
        self.parent = parent
        pass

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
        

class SignalModel(object):
    def __init__(self, parent):
        self.parent = parent
        pass

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


class ArgModel(object):
    def __init__(self, parent):
        self.parent = parent
        self.name = ''
        self.type = ''
        self.direction = ''
        self.hint = ''
        pass
    
class StructModel(object):
    def __init__(self, parent):
        self.parent = parent
        pass

    def sig(self):
        struct_sig = ''
        for f in self.fields:
            field_sig = self.parent.type2sig(f.type)
            struct_sig =  struct_sig + field_sig

        return '(' + struct_sig + ')'

class SequenceModel(object):
    def __init__(self, parent):
        self.parent = parent
        pass

    def sig(self):
        return 'a' + self.parent.type2sig(self.data_type)


class EnumModel(object):
    def __init__(self, parent):
        self.parent = parent
        pass

    def sig(self):
        return 's'

class BindingParser(object):
    def __init__(self, name, model):
        self.file_name = name
        self.model = model

    def parse(self):
        dom = parse(self.file_name)

        nodelist = dom.getElementsByTagName('node')
        for node in nodelist:
            self.handle_node(node)

    def handle_node(self, node):
        nodelist = node.getElementsByTagName('interface')
        for child in nodelist:
            p = InterfaceParser(self.model)
            p.handle(child)


class InterfaceParser(object):
    def __init__(self, model):
        self.model = model
        
    def handle(self, node):
        self.m = InterfaceModel()

        self.m.name = node.getAttribute('name')
        self.m.csymbol = node.getAttribute('csymbol')
        self.m.qname = self.m.name.replace('.','_')
        
        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'method':
                    p = MethodParser(self.m)
                    p.handle(child)
                elif child.nodeName == 'signal':
                    p = SignalParser(self.m)
                    p.handle(child)
                elif child.nodeName == 'struct':
                    p = StructParser(self.m)
                    p.handle(child)
                elif child.nodeName == 'sequence':
                    p = SequenceParser(self.m)
                    p.handle(child)
                elif child.nodeName == 'enum':
                    p = EnumParser(self.m)
                    p.handle(child)
                elif child.nodeName == 'import':
                    p = ImportParser(self.m)
                    p.handle(child)

        self.model.interfaces.append(self.m)
        

class MethodParser(object):

    def __init__(self, model):
        object.__init__(self)
        self.model = model

    def handle(self, node):
        self.m = MethodModel(self.model)

        self.m.name = node.getAttribute('name')
        self.m.csymbol = node.getAttribute('csymbol')
        self.m.qname = self.m.name.replace('.','_')
        self.m.condition = node.getAttribute('condition')
        self.m.params = []
        
        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'arg':
                    self.handle_arg(child)

        self.model.methods.append(self.m)
        
    def handle_arg(self, node):
        p = ArgModel(self.model)

        p.name = node.getAttribute('name')
        p.type = node.getAttribute('type')
        p.direction = node.getAttribute('direction')
        p.hint = node.getAttribute('hint')
        
        self.m.params.append(p)

class SignalParser(object):

    def __init__(self, model):
        object.__init__(self)
        self.model = model

    def handle(self, node):
        self.m = SignalModel(self.model)

        self.m.name = node.getAttribute('name')
        self.m.csymbol = node.getAttribute('csymbol')
        self.m.qname = self.m.name.replace('.','_')
        self.m.params = []
        
        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'arg':
                    self.handle_arg(child)

        self.model.signals.append(self.m)
        
    def handle_arg(self, node):
        p = ArgModel(self.model)

        p.name = node.getAttribute('name')
        p.type = node.getAttribute('type')
        
        self.m.params.append(p)

class StructParser(object):

    def __init__(self, model):
        object.__init__(self)
        self.model = model

    def handle(self, node):
        self.m = StructModel(self.model)

        self.m.name = node.getAttribute('name')
        self.m.csymbol = node.getAttribute('csymbol')
        self.m.qname = self.m.name.replace('.','_')
        self.m.fields = []

        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'field':
                    self.handle_field(child)

        self.model.structs.append(self.m)
        self.model.types[self.m.name] = self.m
        
    def handle_field(self, node):
        p = ArgModel(self.model)
        p.name = node.getAttribute('name')
        p.type = node.getAttribute('type')

        self.m.fields.append(p)

class SequenceParser(object):

    def __init__(self, model):
        object.__init__(self)
        self.model = model

    def handle(self, node):
        self.m = SequenceModel(self.model)
        self.m.name = node.getAttribute('name')
        self.m.csymbol = node.getAttribute('csymbol')
        self.m.qname = self.m.name.replace('.','_')
        self.m.container_type = node.getAttribute('container')
        self.m.data_type = node.getAttribute('type')
        
        self.model.sequences.append(self.m)
        self.model.types[self.m.name] = self.m

class EnumParser(object):

    def __init__(self, model):
        object.__init__(self)
        self.model = model
        self.count = 0
        
    def handle(self, node):
        self.m = EnumModel(self.model)

        self.m.name = node.getAttribute('name')
        self.m.csymbol = node.getAttribute('csymbol')
        self.m.qname = self.m.name.replace('.','_')
        self.m.values = []
        
        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'value':
                    self.handle_value(child)

        self.model.enums.append(self.m)
        self.model.types[self.m.name] = self.m

    def handle_value(self, node):
        p = ArgModel(self.model)

        c = node.getAttribute('value')
        if c != '':
            self.count = int(c)
            
        p.name = node.getAttribute('name')
        p.csymbol = node.getAttribute('csymbol')
        p.value = self.count
        
        self.m.values.append(p)
        
class ImportParser(object):

    def __init__(self, model):
        object.__init__(self)
        self.model = model

    def handle(self, node):
        for child in node.childNodes:
            if child.nodeType == node.ELEMENT_NODE:
                if child.nodeName == 'include':
                    self.handle_include(child)
                elif child.nodeName == 'namespace':
                    self.handle_namespace(child)

    def handle_include(self, node):
        self.model.includes.append(node.getAttribute('name'))

    def handle_namespace(self, node):
        self.model.namespaces.append(node.getAttribute('name'))
   
# Main program

if __name__ == '__main__':
    usage = "usage: %prog [options] <introspect.xml> <prefix>"
    parser = OptionParser(usage=usage)
    parser.add_option("-c", "--client",
                      dest="clientbinding",
                      help="Generate client bindings",
                      )

    (options, args) = parser.parse_args()

    directory = os.path.dirname(sys.argv[0])

    model = Model()
    
    b = BindingParser(args[0], model)
    b.parse()

    t = Template(file=directory+"/DBus-template.hh")
    t.model = model
    s = str(t)

    f = open(args[1] + ".hh", 'w+')
    try:
        f.write(s)
    finally:
        f.close()

    model.prefix = args[1]
    model.include_filename = model.prefix + ".hh"
    
    t = Template(file=directory+"/DBus-template.cc")
    t.model = model
    s = str(t)

    f = open(args[1] + ".cc", 'w+')
    try:
        f.write(s)
    finally:
        f.close()
