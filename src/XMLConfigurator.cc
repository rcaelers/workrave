// XMLConfigurator.cc --- Configuration Access
//
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <sstream>
#include <iostream>

#include "XMLConfigurator.hh"

using namespace std;


//! Constructor
XMLConfigurator::XMLConfigurator()
  : parent(NULL)
{
}


//! Constructor
/*!
 *  \param parent pointer to the parent XML node.
 */
XMLConfigurator::XMLConfigurator(XMLConfigurator *p)
  : parent(p)
{
}


//! Destructor
XMLConfigurator::~XMLConfigurator()
{
  Children::const_iterator i;
  for (i = node_children.begin(); i != node_children.end(); i++)
    {
      delete i->second;
    }
}


bool
XMLConfigurator::load(string filename)
{
  TRACE_ENTER("XMLConfigurator::load");

  last_file_name = filename;
  
  GdomeException exc;

  // First get a DOMImplementation reference
  GdomeDOMImplementation *domImpl = NULL;
  domImpl = gdome_di_mkref();

  // Create URI from filename
  string uri = "file:///" + filename;
  
  // Load XML from file.
  GdomeDocument *doc = NULL;
  doc = gdome_di_createDocFromURI(domImpl, uri.c_str(), GDOME_LOAD_PARSING, &exc);

  GdomeElement *root = NULL;
  if (doc != NULL)
    {
      // Get root element
      root = gdome_doc_documentElement(doc, &exc);

      if (root != NULL)
        {
          GdomeNode *node = GDOME_N(root);
          init(node);

          gdome_el_unref(root, &exc);      
        }
      
      gdome_di_freeDoc(domImpl, doc, &exc);
    }

  gdome_di_unref(domImpl, &exc);

  TRACE_EXIT()
  return root != NULL;
}


bool
XMLConfigurator::save(string filename)
{
  string saveStr;

  GdomeException exc;

  // First get a DOMImplementation reference
  GdomeDOMImplementation *domImpl = NULL;
  domImpl = gdome_di_mkref();

  // Create Doc
  GdomeDocument *doc = NULL;
  GdomeElement *root = NULL;
  
  save_to(domImpl, &doc, root);

  if (doc != NULL)
    {
      /* Save the created document */
      gdome_di_saveDocToFile(domImpl, doc, filename.c_str(), GDOME_SAVE_STANDARD, &exc);
      gdome_di_freeDoc(domImpl, doc, &exc);
    }
  
  gdome_di_unref(domImpl, &exc);

  return false;
}


bool
XMLConfigurator::save()
{
  return save(last_file_name);
}


bool
XMLConfigurator::get_value(string key, string *out) const
{
  TRACE_ENTER_MSG("XMLConfigurator::get_value", key);
  bool ret = false;

  string path = strip_path(key);
  if (path != "")
    {
      XMLConfigurator *child = get_child(path);
      if (child != NULL)
        {
          ret = child->get_value(key, out);
        }
    }
  else
    {
      Attributes::const_iterator i = node_attributes.find(key);
  
      if (i != node_attributes.end())
        {
          *out = (*i).second;
          ret = true;
        }
    }
  
  TRACE_RETURN(*out);
  return ret;
}


bool
XMLConfigurator::get_value(string key, bool *out) const
{
  string s;
  bool ret = get_value(key, &s);
    
  if (ret)
    {
      if (s == "true" || s == "yes" || s == "TRUE" || s == "1")
        *out = true;
      else
        *out = false;
    }
  return ret;
}


bool
XMLConfigurator::get_value(string key, int *out) const
{
  string s;
  bool ret = get_value(key, &s);
    
  if (ret)
    {
      *out = atoi(s.c_str());
    }
  return ret;
}


bool
XMLConfigurator::get_value(string key, long *out) const
{
  string s;
  bool ret = get_value(key, &s);
    
  if (ret)
    {
      *out = atol(s.c_str());
    }
  return ret;
}


bool
XMLConfigurator::get_value(string key, double *out) const
{
  string s;
  bool ret = get_value(key, &s);
    
  if (ret)
    {
      *out = atof(s.c_str());
    }
  return ret;
}


bool
XMLConfigurator::set_value(string key, string v)
{
  TRACE_ENTER_MSG("XMLConfigurator::set_value", key  << " " << v);
  bool ret = false;

  string path = strip_path(key);

  if (path != "")
    {
      XMLConfigurator *child = get_child(path);
      if (child != NULL)
        {
          ret = child->set_value(key, v);
        }
      else if (create_child(path))
        {
          child = get_child(path);
          if (child != NULL)
            {
              ret = child->set_value(key, v);
            }
        }
    }
  else
    {
      node_attributes[key] = v;
      ret = true;
      changed(node_path + key);
    }

  TRACE_EXIT();
  return ret;
}


bool
XMLConfigurator::set_value(string key, int v)
{
  stringstream ss;
  ss << v;
  set_value(key, ss.str());

  return true;
}


bool
XMLConfigurator::set_value(string key, long v)
{
  stringstream ss;
  ss << v;
  set_value(key, ss.str());

  return true;
}


bool
XMLConfigurator::set_value(string key, bool v)
{
  stringstream ss;
  if (v)
    {
      ss << "1";
    }
  else
    {
      ss << "0";
    }      
  set_value(key, ss.str());

  return true;
}


bool
XMLConfigurator::set_value(string key, double v)
{
  stringstream ss;
  ss << v;
  set_value(key, ss.str());

  return true;
}




bool
XMLConfigurator::exists_dir(string key) const
{
  return get_child(key);
}


list<std::string>
XMLConfigurator::get_all_dirs(string key) const
{
  list<std::string> ret;
  XMLConfigurator *child = get_child(key);

  if (child != NULL)
    {
      list<XMLConfigurator *> children = child->get_all_children();
      
      list<XMLConfigurator *>::const_iterator i;
      for (i = children.begin(); i != children.end(); i++)
        {
          ret.push_back((*i)->getName());
        }
    }
  return ret;
}


//! Informs all interested parties that the specified key has changed.
void
XMLConfigurator::changed(string key)
{
  TRACE_ENTER_MSG("XMLConfigurator::changed", key << " " << node_path);
  
  if (parent != NULL)
    {
      // Inform parent. Only the top-level XMLConfigurator registers listeners...
      parent->changed(key);
    }
  else
    {
      // Inform subscribers.
      fire_configurator_event(key);
    }
  TRACE_EXIT();
}


//! Initialize this node.
/*!
 *  \param node the Gdome XML node.
 */
void
XMLConfigurator::init(GdomeNode *node)
{ 
  TRACE_ENTER("XMLConfigurator::init");
  GdomeException exc;

  GdomeDOMString *nodeName = gdome_n_nodeName(node, &exc);
  GdomeDOMString *nodeValue = gdome_n_nodeValue(node, &exc);

  if (nodeName != NULL)
    {
      setName(nodeName->str);
      xml_node_name = nodeName->str;
  }
  
  GdomeNodeType nodeType = (GdomeNodeType) gdome_n_nodeType(node, &exc);
  switch (nodeType)
    {
    case GDOME_TEXT_NODE:
      {
        break;
      }

    case GDOME_DOCUMENT_NODE:
      {
        GdomeNode *child = gdome_n_firstChild(node, &exc);
        
        while (child != NULL)
          {
            if (gdome_n_nodeType(child, &exc) == GDOME_ELEMENT_NODE)
              {
                XMLConfigurator *n = new XMLConfigurator(this);
                n->init(child);
                  
                node_children[n->getName()] = n;
              }

            child = gdome_n_nextSibling(child, &exc);
          }
      }
      break;
    
    case GDOME_ELEMENT_NODE:
      {
        GdomeNamedNodeMap *attributes = gdome_n_attributes(node, &exc);
          
        int attrCount = gdome_nnm_length(attributes, &exc);
        for (int i = 0; i < attrCount; i++)
          {
            GdomeNode *attribute = gdome_nnm_item(attributes, i, &exc);

            GdomeDOMString *k = gdome_n_nodeName(attribute, &exc);
            GdomeDOMString *v = gdome_n_nodeValue(attribute, &exc);

            if (k != NULL && v != NULL)
              {
                string key = k->str;
                string value = v->str;
                
                node_attributes[key] = value;
                
                if (key == "id")
                  {
                    setName(value);
                    xml_node_name = value;
                  }
              }

            if (k != NULL)
              gdome_str_unref(k);

            if (v != NULL)
              gdome_str_unref(v);
            
          }

        GdomeNode *child = gdome_n_firstChild(node, &exc);
        
        while (child != NULL)
          {
            if (gdome_n_nodeType(child, &exc) == GDOME_ELEMENT_NODE)
              {
                XMLConfigurator *n = new XMLConfigurator(this);
                n->init(child);
                  
                node_children[n->getName()] = n;
              }

            child = gdome_n_nextSibling(child, &exc);
          }
      }
      break;
    default:
      cerr << "Unrecognized node type = "
           << nodeType << endl;
    }

  if (nodeName != NULL)
    gdome_str_unref(nodeName);

  if (nodeValue != NULL)
    gdome_str_unref(nodeValue);

  TRACE_EXIT();
}


void
XMLConfigurator::save_to(GdomeDOMImplementation *impl, GdomeDocument **doc, GdomeElement *node)
{
  TRACE_ENTER_MSG("XMLConfigurator::save_to", node_name);

  GdomeException exc; 
  GdomeElement *elem = NULL;

  /* Create a new element */
  GdomeDOMString *name = NULL;

  if (node_name == "")
    {
      name = gdome_str_mkref("workrave");
    }
  else if (xml_node_name != "")
    {
      name = gdome_str_mkref(xml_node_name.c_str());
    }
  else
    {
      GdomeDOMString *node_name = gdome_n_nodeName((GdomeNode *)node, &exc);

      if (node_name != NULL)
        {
          xml_node_name = node_name->str;
          gdome_str_unref(node_name);
        }
    }

  if (node == NULL || *doc == NULL)
    {
      // Top level node -> Create Doc
      *doc = gdome_di_createDocument(impl, NULL, name, NULL, &exc);
  
      if (doc != NULL)
        {
          // Get root element
          elem = gdome_doc_documentElement(*doc, &exc);
        }
    }
  else
    {
      elem = gdome_doc_createElement(*doc, name, &exc);
  
      /* append the element to the childs list of the root element */
      gdome_el_appendChild(node, (GdomeNode *)elem, &exc);
    }
  
  gdome_str_unref (name);
  
  // Add Text Node.
  // if (m_text != "")
  //   {
  //     GdomeDOMString *value = NULL;
  //     value = gdome_str_mkref(m_text.c_str());
  // 
  //     GdomeText *textNode = gdome_doc_createTextNode(*doc, value, &exc);
  //     gdome_str_unref(value);
  // 
  //     gdome_el_appendChild (elem, (GdomeNode *)textNode, &exc);
  //     gdome_t_unref(textNode, &exc);
  //   }

  Attributes::iterator i = node_attributes.begin();
  while (i != node_attributes.end())
    {
      GdomeDOMString *name = NULL;
      GdomeDOMString *value = NULL;
      
      name = gdome_str_mkref((*i).first.c_str());
      value = gdome_str_mkref((*i).second.c_str());
      
      gdome_el_setAttribute(elem, name, value, &exc);
      
      gdome_str_unref(name);
      gdome_str_unref(value);
      
      i++;
    }
  
  Children::iterator ic = node_children.begin();
  while (ic != node_children.end())
    {
      ic->second->save_to(impl, doc, elem);
      ic++;
    }

  gdome_el_unref(elem, &exc);
  TRACE_EXIT();
}


string
XMLConfigurator::strip_path(string &key) const
{
  TRACE_ENTER("XMLConfigurator::strip_path")
  std::string::size_type pos = key.rfind('/');

  string path;
  
  if (pos != std::string::npos)
    {
      path = key.substr(0, pos);
      key = key.substr(pos + 1);
    }

  TRACE_EXIT();
  return path;
}


XMLConfigurator *
XMLConfigurator::get_child(string key) const
{
  TRACE_ENTER_MSG("XMLConfigurator::get_child", key << "(" << node_name << ")");

  XMLConfigurator *ret = NULL;

  std::string::size_type pos = key.find('/');

  string prefix;
  string newKey;
  
  if (pos != std::string::npos)
    {
      prefix = key.substr(0, pos);
      newKey = key.substr(pos + 1);
    }
  else
    {
      prefix = key;
      newKey = "";
    }
  
  TRACE_MSG("new = |" << prefix << "|" << newKey << "|");

  Children::const_iterator i = node_children.find(prefix);
  if (i != node_children.end())
    {
      TRACE_MSG("found");
      
      if (newKey != "")
        {
          TRACE_MSG("recursing");
          ret = i->second->get_child(newKey);
        }
      else
        {
          TRACE_MSG("this");
          ret = i->second;
        }
    }
  else
    {
      TRACE_MSG("not found");
    }

  TRACE_RETURN(" " << (ret != NULL));
  return ret;
}



bool
XMLConfigurator::create_child(string key)
{
  TRACE_ENTER_MSG("XMLConfigurator::create_child", "|" << key << "|");

  bool ret = true; //TODO:

  if (key != "")
    {
      std:: string::size_type pos = key.find('/');

      string prefix;
      string newKey;
  
      if (pos != std::string::npos)
        {
          prefix = key.substr(0, pos);
          newKey = key.substr(pos + 1);
        }
      else
        {
          prefix = key;
          newKey = "";
        }
  
  
      TRACE_MSG("new = " << prefix << " " << newKey);

      Children::const_iterator i = node_children.find(prefix);
  
      if (i != node_children.end())
        {
          TRACE_MSG("found " << newKey);
          i->second->create_child(newKey);
        }
      else
        {
          XMLConfigurator *n = new XMLConfigurator(this);
      
          n->setName(prefix);
          n->xml_node_name = prefix;
          node_children[prefix] = n;

          TRACE_MSG("|" << node_path << "|" << prefix << "|" << newKey << "|");
          changed(node_path + prefix);
          
          n->create_child(newKey);
        }
    }
  
  TRACE_EXIT();
  return ret;
}


//! Returns all children.
list<XMLConfigurator *>
XMLConfigurator::get_all_children() const
{
  list<XMLConfigurator *> ret;
  
  Children::const_iterator i;
  for (i = node_children.begin(); i != node_children.end(); i++)
    {
      ret.push_back(i->second);
    }

  return ret;
}
