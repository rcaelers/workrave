// DBus-template.hh --- DBUS template
//
// Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.nl>
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
// \$Id: DBus.hh 1404 2008-01-07 20:48:30Z rcaelers $
//

\#ifdef HAVE_CONFIG_H
\#include "config.h"
\#endif

\#include <string>
\#include <list>
\#include <map>
\#include <deque>

\#include "DBus.hh"
\#include "DBusException.hh"
\#include "DBusBinding.hh"
\#include "${model.include_filename}"

using namespace std;

#for interface in $model.interfaces

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

\#if 0
class ${interface.qname}_Binding : public DBusBindingBase, public ${interface.qname}
{
private:
  typedef DBusMessage * ($interface.qname::*DBusMethod)(void *object, DBusMessage *message);

  virtual DBusMessage *call(int method_num, void *object, DBusMessage *message);
   
  virtual DBusIntrospect *get_method_introspect()
  {
    return method_introspect;
  }

  virtual DBusIntrospect *get_signal_introspect()
  {
    return signal_introspect;
  }

  #for $m in interface.signals
private:
  void internal_emit_${m.qname}(string path, #slurp
  #set comma = ''
  #for p in m.params
  $comma $interface.type2csymbol(p.type) $p.name#slurp
  #set comma = ','
  #end for
  );
public:
  static void emit_${m.qname}(DBus *dbus, string path, #slurp
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

  static DBusMethod method_table[];
  static DBusIntrospect method_introspect[];
  static DBusIntrospect signal_introspect[];
};
\#endif

DBusMessage *
${interface.qname}::call(int method_num, void *object, DBusMessage *message)
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
${interface.qname}::get_${enum.qname}(DBusMessageIter *reader, ${enum.csymbol} *result)
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
${interface.qname}::put_${enum.qname}(DBusMessageIter *writer, const ${enum.csymbol} *result)
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
${interface.qname}::get_${struct.qname}(DBusMessageIter *reader, ${struct.csymbol} *result)
{
  DBusMessageIter it;
  dbus_message_iter_recurse(reader, &it);

  #for p in struct.fields
  get_${p.type}(&it, (${p.type} *) &(result->${p.name}));
  #end for

  dbus_message_iter_next(reader);
}

void
${interface.qname}::put_${struct.qname}(DBusMessageIter *writer, const ${struct.csymbol} *result)
{
  DBusMessageIter it;
  dbus_bool_t ok;
  
  ok = dbus_message_iter_open_container(writer, DBUS_TYPE_STRUCT, NULL, &it);
  if (!ok)
    {
      throw DBusSystemException("Internal error");
    }

  #for p in struct.fields
  put_${p.type}(&it, (${p.type} *) &(result->${p.name}));
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
${interface.qname}::get_${seq.qname}(DBusMessageIter *reader, ${seq.csymbol} *result)
{
  DBusMessageIter it;

  dbus_message_iter_recurse(reader, &it);
  while (dbus_message_iter_has_next(&it))
  {
    $interface.type2csymbol(seq.data_type) tmp;
    get_${seq.data_type}(&it, &tmp);
    result->push_back(tmp);
  }

  // TODO: is this next call correct?
  dbus_message_iter_close_container(reader, &it);
  dbus_message_iter_next(reader);
}

void
${interface.qname}::put_${seq.qname}(DBusMessageIter *writer, const ${seq.csymbol} *result)
{
  DBusMessageIter arr;
  ${seq.csymbol}::const_iterator it;
  dbus_bool_t ok;
  
  ok = dbus_message_iter_open_container(writer, DBUS_TYPE_ARRAY, "todo", &arr);
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

#for method in $interface.methods

// TODO: catch exception, free, rethrow
DBusMessage *
${interface.qname}::${method.name}(void *object, DBusMessage *message)
{
  DBusMessage *reply;

#if method.condition != ''
\#if $method.condition
#end if
  DBusMessageIter reader;
  DBusMessageIter writer;
  dbus_bool_t ok;
  
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
  #if p.hint == 'ptrptr'
  $interface.type2csymbol(p.type) *${p.name};
  #else
  $interface.type2csymbol(p.type) ${p.name};
  #end if
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
  #if p.hint == '' or p.hint == 'ref'
  $comma $p.name#slurp
  #else if p.hint == 'ptr'
  $comma &$p.name#slurp
  #else if p.hint == 'ptrptr'
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
  #if p.hint == 'ptrptr'
  put_${arg.type}(&writer, ${arg.name});
  #else                                                        
  put_${arg.type}(&writer, &${arg.name});
  #end if                                                         
  #end if
  #end for

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
void ${interface.qname}::internal_emit_${signal.qname}(string path, #slurp
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

  dbus_connection_send(connection, msg, NULL);
  dbus_message_unref(msg);
  dbus_connection_flush(connection);
}

void ${interface.qname}::emit_${signal.qname}(DBus *dbus, string path, #slurp
#set comma = ''
#for p in signal.params
$comma $interface.type2csymbol(p.type) $p.name#slurp
#set comma = ','
#end for
)
{
  DBusBindingBase *binding = dbus->find_binding("${interface.name}");
  if (binding == NULL)
    {
      throw DBusSystemException("Unknown binding");
    }
  
  ${interface.qname} *b = dynamic_cast<${interface.qname} *>(binding);
  if (b != NULL)
    {
      b->internal_emit_${signal.qname}(path, #slurp
#set comma = ''
#for p in signal.params
$comma $p.name#slurp
#set comma = ','
#end for
                                       );
   }
}
 
#end for
  
${interface.qname}::DBusMethod ${interface.qname}::method_table[] = {
#for method in $interface.methods
  &${interface.qname}::$method.qname,
#end for
};

DBusIntrospect ${interface.qname}::method_introspect[] = {
#for method in $interface.methods
  { "$method.qname",
    "$method.sig()"
  },
#end for
  { NULL,
    NULL
  }
};
  
DBusIntrospect ${interface.qname}::signal_introspect[] = {
#for signal in $interface.signals
  { "$signal.qname",
    "$signal.sig()"
  },
#end for
  { NULL,
    NULL
  }
};

#end for

void init_${model.prefix}(DBus *dbus)
{
  #for interface in $model.interfaces
  dbus->register_binding("$interface.name", new $interface.qname);
  #end for
}
