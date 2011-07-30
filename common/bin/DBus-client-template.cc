// DBus-client-template.hh --- DBUS template
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

\#ifdef HAVE_CONFIG_H
\#include "config.h"
\#endif

\#include <string>
\#include <list>
\#include <map>
\#include <deque>

\#include "DBusBinding.hh"
\#include "DBusException.hh"
\#include "${model.include_filename}"

using namespace std;
using namespace workrave;

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

class ${interface.qname}_Impl : public ${interface.qname}, public DBusBaseTypes
{
public:
  ${interface.qname}_Impl(DBus *dbus, const string &service, const string &path);

  #for $method in interface.methods
  #slurp
  $interface.type2csymbol(method.return_type()) ${method.qname}(#slurp
  #set comma = ''
  #for p in method.params
  #if p.direction != 'out' or (not 'return' in p.hint)
  #if 'const' in p.hint
  const #slurp
  #end if
  $comma $interface.type2csymbol(p.type) #slurp
  #set comma = ','
  #end if
  #if p.direction == 'out'
    #if 'return' in p.hint
    /**/ #slurp
    #slurp
    #else if 'ptr' in p.hint
     *$p.name#slurp
    #else
     &$p.name#slurp
    #end if
  #else if p.direction == 'in'
    #if 'ref' in p.hint
    &$p.name#slurp
    #else if 'ptr' in p.hint
    *$p.name#slurp
    #else
    $p.name#slurp
    #end if
  #end if
  #end for
  );
  #slurp
  void ${method.qname}_async(#slurp
  #set comma = ''
  #for p in method.params
  #if p.direction == 'in'
    #if 'const' in p.hint
      const #slurp
    #end if
    $comma $interface.type2csymbol(p.type) #slurp
    #if 'ref' in p.hint
    &$p.name#slurp
    #else if 'ptr' in p.hint
    *$p.name#slurp
    #else
    $p.name#slurp
    #end if
    #set comma = ','
  #end if
  #end for
    $comma ${method.name}_slot slot #slurp
  );
  #slurp


  #end for

private:
  #for $method in interface.methods
  typedef sigc::signal<void, DBusError * #slurp
  #set comma = ','
  #for p in method.params
  #if p.direction == 'out'
    #if 'const' in p.hint
      const #slurp
    #end if
    $comma $interface.type2csymbol(p.type) #slurp
    #if 'ref' in p.hint
    &#slurp
    #else if 'ptr' in p.hint
    *#slurp
    #end if
    #set comma = ','
  #end if
  #end for
  > ${method.qname}_signal;
  #slurp
  struct ${method.qname}_async_closure
  {
     class ${interface.qname}_Impl *impl;
     ${method.qname}_signal signal;
  };
  static void ${method.qname}_async_closure_free(void *mem)
  {
    delete (${method.qname}_async_closure *)mem;
  }

  static void ${method.qname}_fcn_static(DBusPendingCall *pending, void *user_data);
  void ${method.qname}_fcn(DBusPendingCall *pending, void *user_data);

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

  DBus *dbus;
  string service;
  string path;
};


${interface.qname}_Impl::${interface.qname}_Impl(DBus *dbus, const string &service, const string &path)
  : dbus(dbus), service(service), path(path)
{
}

${interface.qname} *${interface.qname}::instance(DBus *dbus, const string &service, const string &path)
{
  return new ${interface.qname}_Impl(dbus, service, path);
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
  get_${p.type}(&it, ($interface.type2csymbol(p.type) *) &(result->${p.name}));
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
  put_${p.type}(&it, ($interface.type2csymbol(p.type) *) &(result->${p.name}));
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

  dbus_message_iter_next(reader);
}

void
${interface.qname}::put_${seq.qname}(DBusMessageIter *writer, const ${seq.csymbol} *result)
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
${interface.qname}::get_${dict.qname}(DBusMessageIter *reader, ${dict.csymbol} *result)
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
${interface.qname}::put_${dict.qname}(DBusMessageIter *writer, const ${dict.csymbol} *result)
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

$interface.type2csymbol(method.return_type()) ${interface.qname}_Impl::${method.qname}(#slurp
  #set comma = ''
  #for p in method.params
  #if p.direction != 'out' or (not 'return' in p.hint)
  #if 'const' in p.hint
  const #slurp
  #end if
  $comma $interface.type2csymbol(p.type) #slurp
  #set comma = ','
  #end if
  #if p.direction == 'out'
    #if 'return' in p.hint
    /**/ #slurp
    #slurp
    #else if 'ptr' in p.hint
     *$p.name#slurp
    #else
     &$p.name#slurp
    #end if
  #else if p.direction == 'in'
    #if 'ref' in p.hint
    &$p.name#slurp
    #else if 'ptr' in p.hint
    *$p.name#slurp
    #else
    $p.name#slurp
    #end if
  #end if
  #end for
  )
{
#if method.condition != ''
\#if $method.condition
#end if

  DBusMessage *message = NULL;
  DBusMessage *reply = NULL;
  DBusPendingCall *pending = NULL;

  DBusMessageIter reader;
  DBusMessageIter writer;
  dbus_bool_t ok;

#set have_in_args = False
#for p in method.params
#if p.direction == 'in'
#set have_in_args = True
#end if
#if 'return' in p.hint
  #if 'ptrtr' in p.hint
    $interface.type2csymbol(p.type) *${p.name} = NULL;
  #else
    $interface.type2csymbol(p.type) ${p.name};
  #end if
#end if
#end for

  try
    {
      message = dbus_message_new_method_call(service.c_str(),
                                             path.c_str(),
                                             "$interface.name",  "$method.name");

      dbus_message_iter_init_append(message, &writer);

  #for arg in method.params:
  #if arg.direction == 'in'
  #if 'ptrptr' in p.hint
      put_${arg.type}(&writer, ${arg.name});
  #else
      put_${arg.type}(&writer, &${arg.name});
  #end if
  #end if
  #end for

      if (!dbus_connection_send_with_reply(dbus->conn(), message, &pending, -1))
        {
          throw DBusSystemException("Cannot send");
        }

      if (NULL == pending)
        {
          throw DBusSystemException("No pending reply");
        }

      dbus_connection_flush(dbus->conn());

      // free message
      dbus_message_unref(message);
      message = NULL;

      // block until we receive a reply
      dbus_pending_call_block(pending);

      // get the reply message
      reply = dbus_pending_call_steal_reply(pending);
      if (NULL == reply)
        {
          throw DBusSystemException("No reply");
        }

      // free the pending message handle
      dbus_pending_call_unref(pending);
      pending = NULL;

      ok = dbus_message_iter_init(reply, &reader);
#if have_in_args
      if (!ok)
        {
          throw DBusSystemException("No parameters");
        }
#end if

#for arg in method.params:
#if $arg.direction == 'out'
      get_${arg.type}(&reader, &${arg.name});
#end if
#end for

    }
  catch (DBusException)
    {
      if (reply != NULL)
        {
          dbus_message_unref(reply);
        }

      if (message != NULL)
        {
          dbus_message_unref(message);
        }

      if (pending != NULL)
        {
          dbus_pending_call_unref(pending);
        }

      throw;
    }

#if method.return_type() != 'void'
  return $method.return_name();
#end if

#if method.condition != ''
\#else
    (void) object;

  reply = dbus_message_new_error(message,
                                 "org.workrave.NotImplemented",
                                 "This method is unavailable in current configuration");
\#endif
#end if
}


void ${interface.qname}_Impl::${method.qname}_async(#slurp
  #set comma = ''
  #for p in method.params
  #if p.direction == 'in'
    #if 'const' in p.hint
      const #slurp
    #end if
    $comma $interface.type2csymbol(p.type) #slurp
    #if 'ref' in p.hint
    &$p.name#slurp
    #else if 'ptr' in p.hint
    *$p.name#slurp
    #else
    $p.name#slurp
    #end if
    #set comma = ','
  #end if
  #end for
  $comma ${method.name}_slot slot
  )
{
#if method.condition != ''
\#if $method.condition
#end if

  DBusMessage *message = NULL;
  DBusPendingCall *pending = NULL;

  DBusMessageIter writer;

  try
    {
      message = dbus_message_new_method_call(service.c_str(),
                                             path.c_str(),
                                             "$interface.name",  "$method.name");

      dbus_message_iter_init(message, &writer);

  #for arg in method.params:
  #if arg.direction == 'in'
  #if 'ptrptr' in p.hint
      put_${arg.type}(&writer, ${arg.name});
  #else
      put_${arg.type}(&writer, &${arg.name});
  #end if
  #end if
  #end for

      if (!dbus_connection_send_with_reply(dbus->conn(), message, &pending, -1))
        {
          throw DBusSystemException("Cannot send");
        }

      if (NULL == pending)
        {
          throw DBusSystemException("No pending reply");
        }

      ${method.qname}_async_closure *closure = new ${method.qname}_async_closure;
      closure->impl = this;
      closure->signal.connect(slot);

      if (!dbus_pending_call_set_notify(pending,
                                        ${interface.qname}_Impl::${method.qname}_fcn_static,
                                        closure, ${method.qname}_async_closure_free))
        {
          throw DBusSystemException("Cannot set notifier");
        }

      // free message
      dbus_message_unref(message);
      message = NULL;
    }
  catch (DBusException)
    {
      if (message != NULL)
        {
          dbus_message_unref(message);
        }

      if (pending != NULL)
        {
          dbus_pending_call_unref(pending);
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
}

void ${interface.qname}_Impl::${method.qname}_fcn_static(DBusPendingCall *pending, void *user_data)
{
  ${method.qname}_async_closure *closure = (${method.qname}_async_closure *)user_data;

  closure->impl->${method.qname}_fcn(pending, user_data);
}

void ${interface.qname}_Impl::${method.qname}_fcn(DBusPendingCall *pending, void *user_data)
{
#if method.condition != ''
\#if $method.condition
#end if
  ${method.qname}_async_closure *closure = (${method.qname}_async_closure *)user_data;
	DBusMessage *reply = NULL;
  DBusMessageIter reader;
	DBusError error;
  dbus_bool_t ok;

#set have_out_args = False
#for p in method.params
  #if p.direction == 'out'
    #set have_out_args = True
    #if 'ptrtr' in p.hint
      $interface.type2csymbol(p.type) *${p.name} = NULL;
    #else
      $interface.type2csymbol(p.type) ${p.name};
    #end if
  #end if
#end for

	dbus_error_init(&error);

  try
    {
      reply = dbus_pending_call_steal_reply(pending);
      if (reply == NULL)
        {
          throw DBusSystemException("Cannot get reply");
        }

      ok = dbus_message_iter_init(reply, &reader);
#if have_out_args
      if (!ok)
        {
          throw DBusSystemException("No parameters");
        }
#end if

#for arg in method.params:
#if $arg.direction == 'out'
      get_${arg.type}(&reader, &${arg.name});
#end if
#end for

      closure->signal.emit((DBusError *)NULL #slurp
#for p in method.params
#if p.direction == 'out'
      , $p.name #slurp
#end if
#end for
      );
    }
  catch (DBusException)
    {
      if (reply != NULL)
        {
          dbus_message_unref(reply);
        }

      if (pending != NULL)
        {
          dbus_pending_call_unref(pending);
        }

      throw;
    }
#if method.condition != ''
\#endif
#end if
}





#end for


#if interface.condition != ''
\#endif
#end if

#end for
