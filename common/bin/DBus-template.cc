// Automatically generated - do not edit
//
//

\#include "DBus.hh"
\#include "DBusException.hh"
\#include "${model.include_filename}"

#* #for $interface in $model.interfaces
\#include "${interface.name}.hh"
#end for
*#

\#include <string>
\#include <list>
\#include <map>
\#include <deque>

using namespace std;

#for interface in $model.interfaces

    
DBusMessage *
${interface.qname}::call(int method_num, void *object, DBusMessage *dbus_message)
{
  DBusMessage *ret = NULL;
  
  if (method_num >=0 && method_num < $len(interface.methods) )
    {
      DBusMethod m = method_table[method_num];
      if (m != NULL)
        {
          ret = (this->*m)(object, dbus_message);
        }
    }
  
  return ret;
}
 
#for enum in $interface.enums

void
${interface.qname}::dbus_get_${enum.qname}(DBusMessageIter *reader, ${enum.csymbol} *result)
{
  std::string value;
	int argtype = dbus_message_iter_get_arg_type(reader);

  if (argtype != DBUS_TYPE_STRING)
		throw DBusTypeException("Type mismatch. Excepted string");

  dbus_get_string(reader, &value);
  
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
${interface.qname}::dbus_put_${enum.qname}(DBusMessageIter *writer, const ${enum.csymbol} *result)
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
        	
  dbus_put_string(writer, &value);
}

#end for

#for struct in $interface.structs

void
${interface.qname}::dbus_get_${struct.qname}(DBusMessageIter *reader, ${struct.csymbol} *result)
{
  DBusMessageIter it;
  dbus_message_iter_recurse(reader, &it);

  #for p in struct.fields
  dbus_get_${p.type}(&it, (${p.type} *) &(result->${p.name}));
  #end for

  dbus_message_iter_next(reader);
}

void
${interface.qname}::dbus_put_${struct.qname}(DBusMessageIter *writer, const ${struct.csymbol} *result)
{
  DBusMessageIter it;
  dbus_message_iter_open_container(writer, DBUS_TYPE_STRUCT, NULL, &it);

  #for p in struct.fields
  dbus_put_${p.type}(&it, (${p.type} *) &(result->${p.name}));
  #end for

  dbus_message_iter_close_container(writer, &it);
}

#end for

#for seq in $interface.sequences

void
${interface.qname}::dbus_get_${seq.qname}(DBusMessageIter *reader, ${seq.csymbol} *result)
{
  DBusMessageIter it;
  dbus_message_iter_recurse(reader, &it);
  while (dbus_message_iter_has_next(&it))
  {
    $interface.type2csymbol(seq.data_type) tmp;
    dbus_get_${seq.data_type}(&it, &tmp);
    result->push_back(tmp);
  }

  dbus_message_iter_close_container(reader, &it);
  dbus_message_iter_next(reader);
}

void
${interface.qname}::dbus_put_${seq.qname}(DBusMessageIter *writer, const ${seq.csymbol} *result)
{
  DBusMessageIter arr;
  dbus_message_iter_open_container(writer, DBUS_TYPE_ARRAY, "todo", &arr);

  ${seq.csymbol}::const_iterator it;

  for(it = result->begin(); it != result->end(); it++)
  {
    dbus_put_${seq.data_type}(&arr, &(*it));
  }
  
  dbus_message_iter_close_container(writer, &arr);
}

#end for

#for method in $interface.methods

DBusMessage *
${interface.qname}::${method.name}(void *object, DBusMessage *dbus_message)
{
  DBusMessage *dbus_reply;

#if method.condition != ''
\#if $method.condition
#end if
  DBusMessageIter reader;
  DBusMessageIter writer;

  #if method.csymbol != ""
  ${interface.csymbol} *dbus_object = (${interface.csymbol} *) object;
  #else
  (void) object;
  #end if
  
  #for p in method.params
  #if p.hint == 'ptrptr'
  $interface.type2csymbol(p.type) *${p.name};
  #else
  $interface.type2csymbol(p.type) ${p.name};
  #end if
  #end for
  
  dbus_message_iter_init(dbus_message, &reader);

  #for arg in method.params:
  #if $arg.direction == 'in'
  dbus_get_${arg.type}(&reader, &${arg.name});
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
                                                          
  dbus_reply = dbus_message_new_method_return(dbus_message);
  dbus_message_iter_init_append(dbus_reply, &writer);
  
  #for arg in method.params:
  #if arg.direction == 'out'
  #if p.hint == 'ptrptr'
  dbus_put_${arg.type}(&writer, ${arg.name});
  #else                                                        
  dbus_put_${arg.type}(&writer, &${arg.name});
  #end if                                                         
  #end if
  #end for

#if method.condition != ''
\#else
 (void) object;

 dbus_reply = dbus_message_new_error(dbus_message,
                                     "org.workrave.NotImplemented",
                                     "This member is unavailable in current configuration");
\#endif
#end if                                                          
  return dbus_reply;

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
  DBusMessage *msg;
  DBusMessageIter writer;

  msg = dbus_message_new_signal(path.c_str(),
                                "$interface.name",
                                "$signal.qname");
  if (msg == NULL)
    {
      throw DBusSystemException("Unable to send signal");
    }

  dbus_message_iter_init_append(msg, &writer);
  
  #for arg in signal.params:
  dbus_put_${arg.type}(&writer, &${arg.name});
  #end for

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
