// XMLConfigurator.hh
//
// Copyright (C) 2001, 2002, 2006, 2007 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef XMLCONFIGURATOR_HH
#define XMLCONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

#undef interface
#include <gdome.h>

#include "IConfigBackend.hh"
#include "ConfigBackendAdapter.hh"

class XMLConfigurator : public virtual IConfigBackend, public virtual ConfigBackendAdapter
{
public:
  XMLConfigurator();
  XMLConfigurator(XMLConfigurator *parent);
  virtual ~XMLConfigurator();


  // Pure virtuals from Configurator
  virtual bool load(std::string filename);
  virtual bool save(std::string filename);
  virtual bool save();

  virtual bool remove_key(const std::string &key);

  virtual bool get_value(const std::string &key, std::string &out) const;
  virtual bool get_value(const std::string &key, bool &out) const;
  virtual bool get_value(const std::string &key, int &out) const;
  virtual bool get_value(const std::string &key, long &out) const;
  virtual bool get_value(const std::string &key, double &out) const;

  virtual bool set_value(const std::string &key, std::string v);
  virtual bool set_value(const std::string &key, int v);
  virtual bool set_value(const std::string &key, long v);
  virtual bool set_value(const std::string &key, bool v);
  virtual bool set_value(const std::string &key, double v);

private:
  void init(GdomeNode *node);
  void save_to(GdomeDOMImplementation *impl, GdomeDocument **doc, GdomeElement *node);
  std::string strip_path(std::string &key) const;
  XMLConfigurator *get_child(const std::string &key) const;
  std::list<XMLConfigurator *> get_all_children() const;
  virtual bool create_child(const std::string &key);

  std::string getName() const
  {
    return node_name;
  }

  void setName(std::string name)
  {
    node_name = name;

    if (parent != NULL)
      {
        node_path = parent->getPath() + node_name + "/";
      }
  }

  std::string getPath() const
  {
    return node_path;
  }

private:
  typedef std::map<std::string, std::string> Attributes;
  typedef std::map<std::string, XMLConfigurator *> Children;

  //! Parent
  XMLConfigurator *parent;

  //! File name of the last 'load'.
  std::string last_file_name;

  //! Name in XML file.
  std::string xml_node_name;

  //! My name (relative to parent)
  std::string node_name;

  //! My path (absolute name);
  std::string node_path;

  //! All child nodes.
  Children node_children;

  //! All attributes
  Attributes node_attributes;
};

#endif // XMLCONFIGURATOR_HH
