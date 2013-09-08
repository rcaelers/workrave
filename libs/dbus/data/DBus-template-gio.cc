// DBus-template.hh --- DBUS template
//
// Copyright (C) 2007, 2008, 2009, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

\#include <stdlib.h>
\#include <gio/gio.h>

\#include "dbus/DBusBindingGio.hh"
\#include "dbus/DBusException.hh"
\#include "${model.include_filename}"

using namespace std;
using namespace workrave::dbus;

class ${model.name}_Marshall : public DBusMarshallGio
{
public:
#for enum in $model.enums
#if enum.condition != ''
 \#if $enum.condition
#end if
  void get_${enum.qname}(GVariant *variant, ${enum.symbol()} *result);
  GVariant *put_${enum.qname}(const ${enum.symbol()} *result);
#if enum.condition
 \#endif // $enum.condition
#end if
#end for

#for struct in $model.structs
#if struct.condition
 \#if $struct.condition
#end if
  void get_${struct.qname}(GVariant *variant, ${struct.symbol()} *result);
  GVariant *put_${struct.qname}(const ${struct.symbol()} *result);
#if struct.condition
 \#endif // $struct.condition
#end if
#end for

#for seq in $model.sequences
#if seq.condition
 \#if $seq.condition
#end if
  void get_${seq.qname}(GVariant *variant, ${seq.symbol()} *result);
  GVariant *put_${seq.qname}(const ${seq.symbol()} *result);
#if seq.condition
 \#endif // $seq.condition
#end if
#end for

#for dict in $model.dictionaries
#if dict.condition
 \#if $dict.condition
#end if
  void get_${dict.qname}(GVariant *variant, ${dict.symbol()} *result);
  GVariant *put_${dict.qname}(const ${dict.symbol()} *result);
#if dict.condition
 \#endif // $dict.condition
#end if
#end for
};

#for enum in $model.enums

#if enum.condition != ''
 \#if $enum.condition
#end if

void
${model.name}_Marshall::get_${enum.qname}(GVariant *variant, ${enum.symbol()} *result)
{
  std::string value;

  get_string(variant, &value);

#set if_stm = 'if'
#for e in enum.values
  $if_stm ("$e.name" == value)
    {
      *result = $e.symbol();
    }
  #set $if_stm = 'else if'
#end for
  else
    {
      throw DBusRemoteException("Illegal enum value");
    }
}

GVariant *
${model.name}_Marshall::put_${enum.qname}(const ${enum.symbol()} *result)
{
  string value;
  switch (*result)
    {
    #for e in enum.values
    case $e.symbol():
      value = "$e.name";
      break;
    #end for
    default:
      throw DBusRemoteException("Illegal enum value");
    }

  return put_string(&value);
}

#if enum.condition
 \#endif // $enum.condition
#end if

#end for

#for struct in $model.structs

#if struct.condition
 \#if $struct.condition
#end if
 
void
${model.name}_Marshall::get_${struct.qname}(GVariant *variant, ${struct.symbol()} *result)
{
#set num_expected_fields = len($struct.fields)

  gsize num_fields = g_variant_n_children(variant);
  if (num_fields != $num_expected_fields)
    {
      throw DBusRemoteException("Incorrect number of field in struct");
    }

#set index = 0
#for p in struct.fields
  GVariant *v_${p.name} = g_variant_get_child_value(variant, $index);

  get_${p.type}(v_${p.name}, &result->${p.name});
  #set index = index + 1
#end for

#for p in struct.fields
  g_variant_unref(v_${p.name});
#end for
}


GVariant *
${model.name}_Marshall::put_${struct.qname}(const ${struct.symbol()} *result)
{
  GVariantBuilder builder;
  g_variant_builder_init(&builder, (GVariantType *)"$struct.sig()");

  GVariant *v;

#for p in struct.fields
  v = put_${p.type}(&(result->${p.name}));
  g_variant_builder_add_value(&builder, v);
#end for

  return g_variant_builder_end(&builder);
}

#if struct.condition
 \#endif // $struct.condition
#end if

#end for

#for seq in $model.sequences

#if seq.condition
 \#if $seq.condition
#end if
 
void
${model.name}_Marshall::get_${seq.qname}(GVariant *variant, ${seq.symbol()} *result)
{
  GVariantIter iter;
  g_variant_iter_init(&iter, variant);

  GVariant *child;
  while ((child = g_variant_iter_next_value(&iter)))
    {
      $model.get_type(seq.data_type).symbol() tmp;
      get_${seq.data_type}(child, &tmp);
      result->push_back(tmp);

      g_variant_unref (child);
    }
}

GVariant *
${model.name}_Marshall::put_${seq.qname}(const ${seq.symbol()} *result)
{
  GVariantBuilder builder;
  g_variant_builder_init(&builder, (GVariantType *)"$seq.sig()");

  ${seq.symbol()}::const_iterator it;

  for (it = result->begin(); it != result->end(); it++)
  {
    GVariant *v = put_${seq.data_type}(&(*it));
    g_variant_builder_add_value(&builder, v);
  }

  return g_variant_builder_end(&builder);
}

#if seq.condition
 \#endif // $seq.condition
#end if
#end for


#for dict in $model.dictionaries

#if dict.condition
 \#if $dict.condition
#end if
 
void
${model.name}_Marshall::get_${dict.qname}(GVariant *variant, ${dict.symbol()} *result)
{
  GVariantIter iter;
  g_variant_iter_init(&iter, variant);

  GVariant *child;
  while ((child = g_variant_iter_next_value(&iter)))
    {
      GVariant *v_key = g_variant_get_child_value(child, 0);
      GVariant *v_value = g_variant_get_child_value(child, 1);

      $model.get_type(dict.key_type).symbol() key;
      $model.get_type(dict.value_type).symbol() value;

      get_${dict.key_type}(v_key, &key);
      get_${dict.value_type}(v_value, &value);

      (*result)[key] = value;

      g_variant_unref(v_key);
      g_variant_unref(v_value);
      g_variant_unref(child);
    }
}


GVariant *
${model.name}_Marshall::put_${dict.qname}(const ${dict.symbol()} *result)
{
  GVariantBuilder builder;
  g_variant_builder_init(&builder, (GVariantType *)"$dict.sig()");

  ${dict.symbol()}::const_iterator it;

  for (it = result->begin(); it != result->end(); it++)
    {
      GVariant *v_key = put_${dict.key_type}(&(it->first));
      GVariant *v_value = put_${dict.value_type}(&(it->second));

      GVariant *v_entry = g_variant_new_dict_entry(v_key, v_value);
      g_variant_builder_add_value(&builder, v_entry);
    }

  return g_variant_builder_end(&builder);
}

#if dict.condition
 \#endif // $dict.condition
#end if

#end for

#for interface in $model.interfaces

#if interface.condition != ''
\#if $interface.condition
#end if

#for $ns in $interface.namespace_list
namespace $ns // interface $interface.name namespace
{
#end for
  
class ${interface.qname}_Stub : public DBusBindingGio, public ${interface.qname}, ${model.name}_Marshall
{
private:
  typedef void (${interface.qname}_Stub::*DBusMethodPointer)(void *object, GDBusMethodInvocation *invocation, const std::string &sender, GVariant *inargs);

  struct DBusMethod
  {
    const string name;
    DBusMethodPointer fn;
  };

  virtual void call(const std::string &method_name, void *object, GDBusMethodInvocation *invocation, const std::string &sender, GVariant *inargs);

  virtual const char *get_interface_introspect()
  {
    return interface_introspect;
  }

public:
  ${interface.qname}_Stub(IDBus::Ptr dbus);
  ~${interface.qname}_Stub();

#for $m in interface.signals
  void ${m.qname}(const string &path #slurp
  #for p in m.params
    #if p.hint == []
      , $interface.get_type(p.type).symbol() $p.name#slurp
    #else if 'ptr' in p.hint
      , $interface.get_type(p.type).symbol() *$p.name#slurp
    #else if 'ref' in p.hint
      , $interface.get_type(p.type).symbol() &$p.name#slurp
    #end if
  #end for
  );
#end for

private:
#for $m in interface.methods
  void ${m.qname}(void *object, GDBusMethodInvocation *invocation, const std::string &sender, GVariant *inargs);
#end for

  static const DBusMethod method_table[];
  static const char *interface_introspect;
};


${interface.qname} *${interface.qname}::instance(const workrave::dbus::IDBus::Ptr dbus)
{
  ${interface.qname}_Stub *iface = NULL;
  DBusBinding *binding = dbus->find_binding("${interface.name}");

  if (binding != NULL)
    {
      iface = dynamic_cast<${interface.qname}_Stub *>(binding);
    }

  return iface;
}

${interface.qname}_Stub::${interface.qname}_Stub(IDBus::Ptr dbus)
  : DBusBindingGio(dbus)
{
}

${interface.qname}_Stub::~${interface.qname}_Stub()
{
}

void
${interface.qname}_Stub::call(const std::string &method_name, void *object, GDBusMethodInvocation *invocation, const std::string &sender, GVariant *inargs)
{
  const DBusMethod *table = method_table;
  while (table->fn != NULL)
    {
      if (method_name == table->name)
        {
          DBusMethodPointer ptr = table->fn;
          if (ptr != NULL)
            {
              (this->*ptr)(object, invocation, sender, inargs);
            }
          return;
        }
      table++;
    }
  throw DBusRemoteException(std::string("No such member:") + method_name );
}

#for method in $interface.methods

void
${interface.qname}_Stub::${method.name}(void *object, GDBusMethodInvocation *invocation, const std::string &sender, GVariant *inargs)
{
#if method.condition != ''
\#if $method.condition
#end if
  (void) sender;

  try
    {
#if method.symbol() != ""
      ${interface.symbol()} *dbus_object = (${interface.symbol()} *) object;
#else
      (void) object;
#end if

#for p in method.params
      $interface.get_type(p.type).symbol() p_${p.name} #slurp
  #if p.direction == 'bind'
      = ${p.bind} #slurp
  #else if p.direction == 'sender'
      = sender #slurp
  #end if
      ;
#end for

      gsize num_in_args = g_variant_n_children(inargs);
      if (num_in_args != $method.num_in_args)
        {
          throw DBusRemoteException("Incorrect numer of in-parameters");
        }

#set index = 0
#for arg in method.params:
  #if $arg.direction == 'in'
      GVariant *v_${arg.name} = g_variant_get_child_value(inargs, $index);
      get_${arg.type}(v_${arg.name}, &p_${arg.name});
    #set index = index + 1
  #end if
#end for

#if method.symbol() != ""
  #if method.return_type() != 'void'
      p_$method.return_name() = dbus_object->${method.symbol()}( #slurp
  #else
      dbus_object->${method.symbol()}( #slurp
  #end if

  #set comma = ''
  #for p in method.params
    #if p.hint == [] or 'ref' in p.hint
      $comma p_$p.name#slurp
    #else if 'ptr' in p.hint
      $comma &p_$p.name#slurp
    #end if
    #set comma = ','
  #end for
      );
#end if

#if method.num_out_args > 0
      GVariantBuilder builder;
      g_variant_builder_init(&builder, (GVariantType*)"$method.sig_of_type('out')");

  #for arg in method.params:
    #if arg.direction == 'out'
      GVariant *v_${arg.name} = put_${arg.type}(&p_${arg.name});
      g_variant_builder_add_value(&builder, v_${arg.name});
    #end if
  #end for

      GVariant *out = g_variant_builder_end(&builder);
#else
      GVariant *out = NULL;
#end if

      g_dbus_method_invocation_return_value(invocation, out);
    }
  catch (DBusException)
    {
      throw;
    }

#if method.condition != ''
\#else
 (void) object;

  g_dbus_method_invocation_return_dbus_error (invocation,
                                              "org.workrave.NotImplemented",
                                              "This method is unavailable in current configuration");
\#endif
#end if
}


#end for

#for signal in interface.signals
void ${interface.qname}_Stub::${signal.qname}(const string &path #slurp
  #for p in signal.params
    #if p.hint == []
      , $interface.get_type(p.type).symbol() $p.name#slurp
    #else if 'ptr' in p.hint
      , $interface.get_type(p.type).symbol() *$p.name#slurp
    #else if 'ref' in p.hint
      , $interface.get_type(p.type).symbol() &$p.name#slurp
    #end if
  #end for
)
{
  IDBusPrivateGio::Ptr p = boost::dynamic_pointer_cast<IDBusPrivateGio>(dbus);
  
  GDBusConnection *connection = p->get_connection();
  if (connection == NULL)
    {
      return;
    }

#if len(signal.params) > 0
  GVariantBuilder builder;
  g_variant_builder_init(&builder, (GVariantType*)"$signal.sig()");

  #for arg in signal.params:
    #if 'ptr' in p.hint
  GVariant *v_${arg.name} = put_${arg.type}(${arg.name});
    #else
  GVariant *v_${arg.name} = put_${arg.type}(&${arg.name});
    #end if
  g_variant_builder_add_value(&builder, v_${arg.name});
  #end for

  GVariant *out = g_variant_builder_end(&builder);
#else
  GVariant *out = NULL;
#end if

  GError *error = NULL;
  g_dbus_connection_emit_signal(connection,
                                NULL,
                                path.c_str(),
                                "${interface.name}",
                                "${signal.name}",
                                out,
                                &error);

  if (error != NULL)
    {
      g_error_free(error);
    }
}

#end for

const ${interface.qname}_Stub::DBusMethod ${interface.qname}_Stub::method_table[] = {
#for method in $interface.methods
  { "$method.name", &${interface.qname}_Stub::$method.qname },
#end for
  { "", NULL }
};

const char *
${interface.qname}_Stub::interface_introspect =
  "  <interface name=\"$interface.name\">\n"
#for method in $interface.methods
  "    <method name=\"$method.qname\">\n"
  #for p in method.params
    #if p.direction == 'in' or p.direction == 'out'
  "      <arg type=\"$p.sig()\" name=\"$p.name\" direction=\"$p.direction\" />\n"
    #end if
  #end for
  "    </method>\n"
#end for
  #for signal in $interface.signals
  "    <signal name=\"$signal.qname\">\n"
    #for a in signal.params
  "      <arg type=\"$p.sig()\" name=\"$p.name\" />\n"
    #end for
  "    </signal>\n"
  #end for
  "  </interface>\n";

#for $ns in reversed($interface.namespace_list)
} // namespace $ns
#end for
 
#if interface.condition != ''
 \#endif // $interface.condition
#end if

#end for

void init_${model.name}(IDBus::Ptr dbus)
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
 
