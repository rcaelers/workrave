// DBus-template.hh --- DBUS template
//
// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

\#include "dbus/DBus-qt5.hh"
\#include "dbus/DBusBinding-qt5.hh"
\#include "dbus/DBusException.hh"
\#include "${model.include_filename}"

using namespace std;
using namespace workrave::dbus;

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
  typedef void (${interface.qname}_Stub::*DBusMethodPointer)(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  struct DBusMethod
  {
    const string name;
    DBusMethodPointer fn;
  };

  virtual bool call(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  virtual const char *get_interface_introspect()
  {
    return interface_introspect;
  }

public:
  ${interface.qname}_Stub(DBus *dbus);
  ~${interface.qname}_Stub();

#for $m in interface.signals
  void ${m.qname}(const string &path, #slurp
  #set comma = ''
  #for p in m.params
    #if p.hint == []
      $comma $interface.type2csymbol(p.type) $p.name#slurp
    #else if 'ptr' in p.hint
      $comma $interface.type2csymbol(p.type) *$p.name#slurp
    #else if 'ref' in p.hint
      $comma $interface.type2csymbol(p.type) &$p.name#slurp
    #end if
  #set comma = ','
  #end for
  );
#end for


private:
#for $m in interface.methods
  void ${m.qname}(void *object, const QDBusMessage &message, const QDBusConnection &connection);
#end for

#for enum in $interface.enums
  void get_${enum.qname}(const QVariant &variant, ${enum.csymbol} *result);
  QVariant put_${enum.qname}(const ${enum.csymbol} *result);
#end for

#for struct in $interface.structs
  void get_${struct.qname}(const QVariant &variant, ${struct.csymbol} *result);
  QVariant put_${struct.qname}(const ${struct.csymbol} *result);
#end for

#for seq in $interface.sequences
  void get_${seq.qname}(const QVariant &variant, ${seq.csymbol} *result);
  QVariant put_${seq.qname}(const ${seq.csymbol} *result);
#end for

#for dict in $interface.dictionaries
  void get_${dict.qname}(const QVariant &variant, ${dict.csymbol} *result);
  QVariant put_${dict.qname}(const ${dict.csymbol} *result);
#end for

  static const DBusMethod method_table[];
  static const char *interface_introspect;
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

bool
${interface.qname}_Stub::call(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  string method_name = message.member().toStdString();
  const DBusMethod *table = method_table;
  while (table->fn != NULL)
    {
      if (method_name == table->name)
        {
          DBusMethodPointer ptr = table->fn;
          if (ptr != NULL)
            {
              (this->*ptr)(object, message, connection);
            }
          return true;
        }
      table++;
    }
  throw DBusUsageException(std::string("No such member:") + method_name );
}

#for enum in $interface.enums

void
${interface.qname}_Stub::get_${enum.qname}(const QVariant &variant, ${enum.csymbol} *result)
{
  std::string value;

  get_string(variant, &value);

#set if_stm = 'if'
#for e in enum.values
  $if_stm ("$e.name" == value)
    {
      *result = $e.csymbol;
    }
  #set $if_stm = 'else if'
#end for
  else
    {
      throw DBusTypeException("Illegal enum value");
    }
}

QVariant
${interface.qname}_Stub::put_${enum.qname}(const ${enum.csymbol} *result)
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

  return put_string(&value);
}

#end for

#for struct in $interface.structs

void
${interface.qname}_Stub::get_${struct.qname}(const QVariant &variant, ${struct.csymbol} *result)
{
#set num_expected_fields = len($struct.fields)

  QVariantList vlist = variant.value<QVariantList>();
  if (vlist.size() != $num_expected_fields)
    {
      throw DBusSystemException("Incorrect number of field in struct");
    }

#for p in struct.fields
  #if p.type != p.ext_type
  $interface.type2csymbol(p.ext_type) _${p.name};
  #end if
#end for

#set index = 0
#for p in struct.fields
  QVariant v_${p.name} = vlist.at($index);

  #if p.type != p.ext_type
  get_${p.ext_type}(v_${p.name}, &_${p.name});
  #else
  get_${p.ext_type}(v_${p.name}, ($interface.type2csymbol(p.ext_type) *) &(result->${p.name}));
  #end if
  #set index = index + 1
#end for

#for p in struct.fields
  #if p.type != p.ext_type
  result->${p.name} = ($interface.type2csymbol(p.type)) _${p.name};
  #end if
#end for
}


QVariant
${interface.qname}_Stub::put_${struct.qname}(const ${struct.csymbol} *result)
{

#for p in struct.fields
  #if p.type != p.ext_type
  $interface.type2csymbol(p.ext_type) f_${p.name} = ($interface.type2csymbol(p.ext_type))result->${p.name};
  #end if
#end for

  QVariantList vlist;

#for p in struct.fields
  #if p.type != p.ext_type
  QVariant v = put_${p.ext_type}(&f_${p.name});
  #else
  QVariant v = put_${p.ext_type}(($interface.type2csymbol(p.type) *) &(result->${p.name}));
  #end if
  vlist.append(vlist, v);
#end for

  return vlist;
}

#end for

#for seq in $interface.sequences

void
${interface.qname}_Stub::get_${seq.qname}(const QVariant &variant, ${seq.csymbol} *result)
{
  QVariantList vlist = variant.value<QVariantList>();

  QVariant child;
  for (auto child : vlist)
    {
      $interface.type2csymbol(seq.data_type) tmp;
      get_${seq.data_type}(child, &tmp);
      result->push_back(tmp);
    }
}

QVariant
${interface.qname}_Stub::put(const ${seq.csymbol} *result)
{
  QVariantList vlist;

  for (auto it : result)
  {
    QVariant v = put_${seq.data_type}(&(*it));
    vlist.append(v);
  }

  return vlist;
}
#end for


#for dict in $interface.dictionaries

void
${interface.qname}_Stub::get_${dict.qname}(const QVariant &variant, ${dict.csymbol} *result)
{
  QVariantMap vmap;

  for (auto child : vmap)
    {
      QString v_key = child.key();
      QVariant v_value = child.value()

      $interface.type2csymbol(dict.key_type) key;
      $interface.type2csymbol(dict.value_type) value;

      get_${dict.key_type}(v_key, &key);
      get_${dict.value_type}(v_value, &value);

      (*result)[key] = value;
    }
}


QVariant
${interface.qname}_Stub::put_${dict.qname}(const ${dict.csymbol} *result)
{
  QVariantMap vmap;

  for (for it in result)
    {
      QString v_key = put_${dict.key_type}(&(it->first));
      QVariant v_value = put_${dict.value_type}(&(it->second));

      vmap[v_key] = v_value;
    }

  return cmap;
}


#end for

#for method in $interface.methods

void
${interface.qname}_Stub::${method.name}(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
#if method.condition != ''
\#if $method.condition
#end if

  try
    {
#if method.csymbol != ""
      ${interface.csymbol} *dbus_object = (${interface.csymbol} *) object;
#else
      (void) object;
#end if

#for p in method.params
  #if 'ptrptr' in p.hint
      $interface.type2csymbol(p.type) *p_${p.name} #slurp
  #else
      $interface.type2csymbol(p.type) p_${p.name} #slurp
  #end if
  #if p.direction == 'bind'
      = ${p.bind} #slurp
  #else if p.direction == 'sender'
      = sender #slurp
  #end if
      ;
#end for

      int num_in_args = message.arguments().size();
      if (num_in_args != $method.num_in_args)
        {
          throw DBusSystemException("Incorrect numer of in-parameters");
        }

#set index = 0
#for arg in method.params:
  #if $arg.direction == 'in'
      QVariant v_${arg.name} = message.arguments().at($index);
      get_${arg.type}(v_${arg.name}, &p_${arg.name});
    #set index = index + 1
  #end if
#end for

#if method.csymbol != ""
  #if method.return_type() != 'void'
      p_$method.return_name() = dbus_object->${method.csymbol}( #slurp
  #else
      dbus_object->${method.csymbol}( #slurp
  #end if

  #set comma = ''
  #for p in method.params
    #if p.hint == [] or 'ref' in p.hint
      $comma p_$p.name#slurp
    #else if 'ptr' in p.hint
      $comma &p_$p.name#slurp
    #else if 'ptrptr' in p.hint
      $comma &p_$p.name#slurp
    #end if
    #set comma = ','
  #end for
      );
#end if

     QDBusMessage reply = message.createReply();
                                                                
#if method.num_out_args > 0

  #for arg in method.params:
    #if arg.direction == 'out'
      #if 'ptrptr' in p.hint
      QVariant v_${arg.name} = put_${arg.type}(p_${arg.name});
      #else
      QVariant v_${arg.name} = put_${arg.type}(&p_${arg.name});
      #end if
      reply << v_${arg.name};
    #end if
  #end for
#end if

      connection.send(reply);
    }
  catch (DBusException)
    {
      throw;
    }

#if method.condition != ''
\#else
  message.createErrorReply(QDBusError::UnknownMethod,
                          "This method is unavailable in current configuration");
\#endif
#end if
}


#end for

#for signal in interface.signals
void ${interface.qname}_Stub::${signal.qname}(const string &path, #slurp
#set comma = ''
  #for p in signal.params
    #if p.hint == []
      $comma $interface.type2csymbol(p.type) $p.name#slurp
    #else if 'ptr' in p.hint
      $comma $interface.type2csymbol(p.type) *$p.name#slurp
    #else if 'ref' in p.hint
      $comma $interface.type2csymbol(p.type) &$p.name#slurp
    #end if
  #set comma = ','
  #end for
)
{
  QDBusMessage sig = QDBusMessage::createSignal(QString::fromStdString(path), "${interface.name}", "${signal.name}");
                                               
#if len(signal.params) > 0
  #for arg in signal.params:
    #if 'ptr' in p.hint
  QVariant v_${arg.name} = put_${arg.type}(${arg.name});
    #else
  QVariant v_${arg.name} = put_${arg.type}(&${arg.name});
    #end if
  #end for
#end if

  dbus->get_connection().send(sig);
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
