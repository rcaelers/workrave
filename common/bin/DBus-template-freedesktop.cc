// DBus-template.hh --- DBUS template
//
// Copyright (C) 2007, 2008, 2009, 2011 Rob Caelers <robc@krandor.nl>
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

\#ifdef HAVE_CONFIG_H
\#include "config.h"
\#endif

\#include <string>
\#include <list>
\#include <map>
\#include <deque>

\#include "DBus-freedesktop.hh"
\#include "DBusBinding-freedesktop.hh"
\#include "DBusException.hh"
\#include "${model.include_filename}"

using namespace std;

#for interface in $model.interfaces

#if interface.condition != ''
\#if $interface.condition
#end if

#for imp in interface.imports
#for ns in imp.namespaces
using namespace $ns;
#end for
#end for

#for imp in interface.imports
#for include in imp.includes
\#include "${include}"
#end for
#end for

class ${interface.qname}_Stub : public DBusBindingBase, public ${interface.qname}
{
private:
  typedef DBusMessage * (${interface.qname}_Stub::*DBusMethod)(void *object, DBusMessage *message);

  virtual DBusMessage *call(int method_num, void *object, DBusMessage *message);

  virtual DBusIntrospect *get_method_introspect()
  {
    return method_introspect;
  }

  virtual DBusIntrospect *get_signal_introspect()
  {
    return signal_introspect;
  }

public:
  ${interface.qname}_Stub(DBus *dbus);
  ~${interface.qname}_Stub();

  #for $m in interface.signals
  void ${m.qname}(const string &path, #slurp
  #set comma = ''
  #for p in m.params
  $comma $interface.type2csymbol(p.type) $p.name#slurp
  #set comma = ','
  #end for
  );
  #end for


private:
  #for $m in interface.methods
  DBusMessage *${m.qname}(void *object, DBusMessage *message);
  #end for

  #for enum in $interface.enums
  void get_${enum.qname}(DBusMessageIter *reader, ${enum.csymbol} *result);
  void put_${enum.qname}(DBusMessageIter *writer, const ${enum.csymbol} *result);
  #end for

  #for struct in $interface.structs
  void get_${struct.qname}(DBusMessageIter *reader, ${struct.csymbol} *result);
  void put_${struct.qname}(DBusMessageIter *writer, const ${struct.csymbol} *result);
  #end for

  #for seq in $interface.sequences
  void get_${seq.qname}(DBusMessageIter *reader, ${seq.csymbol} *result);
  void put_${seq.qname}(DBusMessageIter *writer, const ${seq.csymbol} *result);
  #end for

  #for dict in $interface.dictionaries
  void get_${dict.qname}(DBusMessageIter *reader, ${dict.csymbol} *result);
  void put_${dict.qname}(DBusMessageIter *writer, const ${dict.csymbol} *result);
  #end for

  static DBusMethod method_table[];
  static DBusIntrospect method_introspect[];
  static DBusIntrospect signal_introspect[];
};


${interface.qname} *${interface.qname}::instance(const DBus *dbus)
{
  ${interface.qname}_Stub *iface = NULL;
  DBusBindingBase *binding = dbus->find_binding("${interface.name}");

  if (binding != NULL)
    {
      iface = dynamic_cast<${interface.qname}_Stub *>(binding);
    }

  return iface;
}

${interface.qname}_Stub::${interface.qname}_Stub(DBus *dbus)
  : DBusBindingBase(dbus)
{
}

${interface.qname}_Stub::~${interface.qname}_Stub()
{
}

DBusMessage *
${interface.qname}_Stub::call(int method_num, void *object, DBusMessage *message)
{
  DBusMessage *ret = NULL;

  if (method_num >=0 && method_num < $len(interface.methods) )
    {
      DBusMethod m = method_table[method_num];
      if (m != NULL)
        {
          ret = (this->*m)(object, message);
        }
    }

  return ret;
}

#for enum in $interface.enums

void
${interface.qname}_Stub::get_${enum.qname}(DBusMessageIter *reader, ${enum.csymbol} *result)
{
  std::string value;
	int argtype = dbus_message_iter_get_arg_type(reader);

  if (argtype != DBUS_TYPE_STRING)
		throw DBusTypeException("Type mismatch. Excepted string");

  get_string(reader, &value);

  #set ifs = 'if'
  #for e in enum.values
  $ifs ("$e.name" == value)
    {
      *result = $e.csymbol;
    }
  #set $ifs = 'else if'
  #end for
  else
    {
      throw DBusTypeException("Illegal enum value");
    }
}

void
${interface.qname}_Stub::put_${enum.qname}(DBusMessageIter *writer, const ${enum.csymbol} *result)
{
  string value;
  switch (*result)
    {
    #for e in enum.values
    case $e.csymbol:
      value = "$e.name";
      break;
    #end for
    default:
      throw DBusTypeException("Illegal enum value");
    }

  put_string(writer, &value);
}

#end for

#for struct in $interface.structs

void
${interface.qname}_Stub::get_${struct.qname}(DBusMessageIter *reader, ${struct.csymbol} *result)
{
  DBusMessageIter it;
  dbus_message_iter_recurse(reader, &it);

  #for p in struct.fields
  #if p.type != p.ext_type
  $interface.type2csymbol(p.ext_type) _${p.name};
  #end if
  #end for

  #for p in struct.fields
  #if p.type != p.ext_type
  get_${p.ext_type}(&it, &_${p.name});
  #else
  get_${p.ext_type}(&it, ($interface.type2csymbol(p.ext_type) *) &(result->${p.name}));
  #end if
  #end for

  #for p in struct.fields
  #if p.type != p.ext_type
  result->${p.name} = ($interface.type2csymbol(p.type)) _${p.name};
  #end if
  #end for

  dbus_message_iter_next(reader);
}

void
${interface.qname}_Stub::put_${struct.qname}(DBusMessageIter *writer, const ${struct.csymbol} *result)
{
  DBusMessageIter it;
  dbus_bool_t ok;

  ok = dbus_message_iter_open_container(writer, DBUS_TYPE_STRUCT, NULL, &it);
  if (!ok)
    {
      throw DBusSystemException("Internal error");
    }

  #for p in struct.fields
  #if p.type != p.ext_type
  $interface.type2csymbol(p.ext_type) _${p.name} = ($interface.type2csymbol(p.ext_type))result->${p.name};
  #end if
  #end for

  #for p in struct.fields
  #if p.type != p.ext_type
  put_${p.ext_type}(&it, &_${p.name});
  #else
  put_${p.ext_type}(&it, ($interface.type2csymbol(p.type) *) &(result->${p.name}));
  #end if
  #end for

  ok = dbus_message_iter_close_container(writer, &it);
  if (!ok)
    {
      throw DBusSystemException("Internal error");
    }
}

#end for

#for seq in $interface.sequences

void
${interface.qname}_Stub::get_${seq.qname}(DBusMessageIter *reader, ${seq.csymbol} *result)
{
  DBusMessageIter it;

  dbus_message_iter_recurse(reader, &it);
  while (dbus_message_iter_has_next(&it))
  {
    $interface.type2csymbol(seq.data_type) tmp;
    get_${seq.data_type}(&it, &tmp);
    result->push_back(tmp);
  }

  dbus_message_iter_next(reader);
}

void
${interface.qname}_Stub::put_${seq.qname}(DBusMessageIter *writer, const ${seq.csymbol} *result)
{
  DBusMessageIter arr;
  ${seq.csymbol}::const_iterator it;
  dbus_bool_t ok;

  ok = dbus_message_iter_open_container(writer, DBUS_TYPE_ARRAY, "$interface.type2sig(seq.data_type)", &arr);
  if (!ok)
    {
      throw DBusSystemException("Internal error");
    }

  for(it = result->begin(); it != result->end(); it++)
  {
    put_${seq.data_type}(&arr, &(*it));
  }

  ok = dbus_message_iter_close_container(writer, &arr);
  if (!ok)
    {
      throw DBusSystemException("Internal error");
    }
}
#end for


#for dict in $interface.dictionaries

void
${interface.qname}_Stub::get_${dict.qname}(DBusMessageIter *reader, ${dict.csymbol} *result)
{
  DBusMessageIter arr_it;
  DBusMessageIter dict_it;

  dbus_message_iter_recurse(reader, &arr_it);
  while (dbus_message_iter_has_next(&arr_it))
  {
    $interface.type2csymbol(dict.key_type) key;
    $interface.type2csymbol(dict.value_type) value;

    dbus_message_iter_recurse(&arr_it, &dict_it);

    get_${dict.key_type}(&dict_it, &key);
    get_${dict.value_type}(&dict_it, &value);

    (*result)[key] = value;

    dbus_message_iter_next(&arr_it);
  }

  dbus_message_iter_next(reader);
}


void
${interface.qname}_Stub::put_${dict.qname}(DBusMessageIter *writer, const ${dict.csymbol} *result)
{
  DBusMessageIter arr_it;
  DBusMessageIter dict_it;
  ${dict.csymbol}::const_iterator it;
  dbus_bool_t ok;

  ok = dbus_message_iter_open_container(writer, DBUS_TYPE_ARRAY,
                                        "$interface.type2sig(dict.value_type)", &arr_it);
  if (!ok)
    {
      throw DBusSystemException("Internal error");
    }

  for (it = result->begin(); it != result->end(); it++)
    {
      ok = dbus_message_iter_open_container(&arr_it, DBUS_TYPE_DICT_ENTRY, NULL, &dict_it);
      if (!ok)
        {
          throw DBusSystemException("Internal error");
        }

      put_${dict.key_type}(&dict_it, &(it->first));
      put_${dict.value_type}(&dict_it, &(it->second));

      ok = dbus_message_iter_close_container(&arr_it, &dict_it);
      if (!ok)
        {
          throw DBusSystemException("Internal error");
        }
    }

  ok = dbus_message_iter_close_container(writer, &arr_it);
  if (!ok)
    {
      throw DBusSystemException("Internal error");
    }
}


#end for

#for method in $interface.methods

DBusMessage *
${interface.qname}_Stub::${method.name}(void *object, DBusMessage *message)
{
  DBusMessage *reply = NULL;

#if method.condition != ''
\#if $method.condition
#end if
  DBusMessageIter reader;
  DBusMessageIter writer;
  dbus_bool_t ok;

  try
    {
      #if method.csymbol != ""
      ${interface.csymbol} *dbus_object = (${interface.csymbol} *) object;
      #else
      (void) object;
      #end if

      #set have_in_args = False
      #for p in method.params
      #if p.direction == 'in'
      #set have_in_args = True
      #end if
      #if p.direction == 'sender'
      const char *sender = dbus_message_get_sender(message);
      #end if
      #if 'ptrptr' in p.hint
      $interface.type2csymbol(p.type) *${p.name} #slurp
      #else
      $interface.type2csymbol(p.type) ${p.name} #slurp
      #end if
      #if p.direction == 'bind'
      = ${p.bind} #slurp
      else if p.direction == 'sender'
      = sender #slurp
      #end if
      ;
      #end for

      ok = dbus_message_iter_init(message, &reader);
      #if have_in_args
      if (!ok)
        {
          throw DBusSystemException("No parameters");
        }
      #end if

      #for arg in method.params:
      #if $arg.direction == 'in'
      get_${arg.type}(&reader, &${arg.name});
      #end if
      #end for

      #if method.csymbol != ""
      #if method.return_type() != 'void'
      $method.return_name() = dbus_object->${method.csymbol}( #slurp
      #else
      dbus_object->${method.csymbol}( #slurp
      #end if
      #set comma = ''
      #for p in method.params
      #if p.hint == [] or 'ref' in p.hint
      $comma $p.name#slurp
      #else if 'ptr' in p.hint
      $comma &$p.name#slurp
      #else if 'ptrptr' in p.hint
      $comma &$p.name#slurp
      #end if
      #set comma = ','
      #end for
      );
      #end if

      reply = dbus_message_new_method_return(message);
      if (reply == NULL)
        {
          throw DBusSystemException("Internal error");
        }

      dbus_message_iter_init_append(reply, &writer);

      #for arg in method.params:
      #if arg.direction == 'out'
      #if 'ptrptr' in p.hint
      put_${arg.type}(&writer, ${arg.name});
      #else
      put_${arg.type}(&writer, &${arg.name});
      #end if
      #end if
      #end for
  }
  catch (DBusException)
    {
      if (reply != NULL)
        {
          dbus_message_unref(reply);
        }

      throw;
    }

#if method.condition != ''
\#else
 (void) object;

 reply = dbus_message_new_error(message,
                                "org.workrave.NotImplemented",
                                "This method is unavailable in current configuration");
\#endif
#end if
  return reply;

}


#end for

#for signal in interface.signals
void ${interface.qname}_Stub::${signal.qname}(const string &path, #slurp
#set comma = ''
#for p in signal.params
$comma $interface.type2csymbol(p.type) $p.name#slurp
#set comma = ','
#end for
)
{
  DBusMessage *msg = NULL;
  DBusMessageIter writer;

  msg = dbus_message_new_signal(path.c_str(),
                                "$interface.name",
                                "$signal.qname");
  if (msg == NULL)
    {
      throw DBusSystemException("Unable to send signal");
    }

  dbus_message_iter_init_append(msg, &writer);

  try
    {
      #for arg in signal.params:
      put_${arg.type}(&writer, &${arg.name});
      #end for
    }
  catch (DBusException &e)
    {
      dbus_message_unref(msg);
      throw;
    }

  send(msg);
}

#end for

${interface.qname}_Stub::DBusMethod ${interface.qname}_Stub::method_table[] = {
#for method in $interface.methods
  &${interface.qname}_Stub::$method.qname,
#end for
};

DBusIntrospect ${interface.qname}_Stub::method_introspect[] = {
#for method in $interface.methods
  { "$method.qname",
    "$method.introspect_sig()"
  },
#end for
  { NULL,
    NULL
  }
};

DBusIntrospect ${interface.qname}_Stub::signal_introspect[] = {
#for signal in $interface.signals
  { "$signal.qname",
    "$signal.introspect_sig()"
  },
#end for
  { NULL,
    NULL
  }
};

#if interface.condition != ''
 \#endif // $interface.condition
#end if

#end for

void init_${model.name}(DBus *dbus)
{
  #for interface in $model.interfaces
  #if interface.condition != ''
\#if $interface.condition
  #end if
  dbus->register_binding("$interface.name", new ${interface.qname}_Stub(dbus));
  #if interface.condition != ''
\#endif // $interface.condition
  #end if
  #end for
}
