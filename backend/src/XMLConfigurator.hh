// XMLConfigurator.hh 
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef XMLCONFIGURATOR_HH
#define XMLCONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

#undef interface
#include <gdome.h>

#include "Configurator.hh"

class ConfigurationListener;

class XMLConfigurator : public Configurator
{
public:
  XMLConfigurator();
  XMLConfigurator(XMLConfigurator *parent);
  virtual ~XMLConfigurator();

  // Pure virtuals from Configurator
  virtual bool load(string filename);
  virtual bool save(string filename);
  virtual bool save();
  virtual bool get_value(string key, string *out) const;
  virtual bool get_value(string key, bool *out) const;
  virtual bool get_value(string key, int *out) const;
  virtual bool get_value(string key, long *out) const;
  virtual bool get_value(string key, double *out) const;
  virtual bool create_child(string key);
  virtual bool set_value(string key, string v);
  virtual bool set_value(string key, int v);
  virtual bool set_value(string key, long v);
  virtual bool set_value(string key, bool v);
  virtual bool set_value(string key, double v);
  virtual list<string> get_all_dirs(string key) const;
  virtual bool exists_dir(string key) const;
  
private:
  void init(GdomeNode *node);
  void save_to(GdomeDOMImplementation *impl, GdomeDocument **doc, GdomeElement *node);
  string strip_path(string &key) const;
  XMLConfigurator *get_child(string key) const;
  list<XMLConfigurator *> get_all_children() const;
  void changed(string key);
  
  string getName() const
  {
    return node_name;
  }

  void setName(string name)
  {
    node_name = name;

    if (parent != NULL)
      {
        node_path = parent->getPath() + node_name + "/";
      }
  }
  
  string getPath() const
  {
    return node_path;
  }

private:
  typedef map<string, string> Attributes;
  typedef map<string, XMLConfigurator *> Children;

  //! Parent
  XMLConfigurator *parent;

  //! File name of the last 'load'.
  string last_file_name;

  //! Name in XML file.
  string xml_node_name;

  //! My name (relative to parent)
  string node_name;

  //! My path (absolute name);
  string node_path;
  
  //! All child nodes.
  Children node_children;

  //! All attributes
  Attributes node_attributes;
};

#endif // XMLCONFIGURATOR_HH
