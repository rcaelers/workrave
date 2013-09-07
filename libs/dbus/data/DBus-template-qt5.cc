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

\#include <boost/lexical_cast.hpp>

\#include "dbus/DBusBindingQt5.hh"
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

class ${interface.qname}_Stub : public DBusBindingQt5, public ${interface.qname}
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
  ${interface.qname}_Stub(IDBus::Ptr dbus);
  virtual ~${interface.qname}_Stub();

#for $m in interface.signals
  void ${m.qname}(const string &path, #slurp
  #set comma = ''
  #for p in m.params
    #if p.hint == []
      $comma $interface.get_type(p.type).symbol() $p.name#slurp
    #else if 'ptr' in p.hint
      $comma $interface.get_type(p.type).symbol() *$p.name#slurp
    #else if 'ref' in p.hint
      $comma $interface.get_type(p.type).symbol() &$p.name#slurp
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
  void get_${enum.qname}(const QVariant &variant, ${enum.symbol()} *result);
  QVariant put_${enum.qname}(const ${enum.symbol()} *result);
#end for

#for struct in $interface.structs
  void get_${struct.qname}(const QVariant &variant, ${struct.symbol()} *result);
  QVariant put_${struct.qname}(const ${struct.symbol()} *result);
#end for

#for seq in $interface.sequences
  void get_${seq.qname}(const QVariant &variant, ${seq.symbol()} *result);
  QVariant put_${seq.qname}(const ${seq.symbol()} *result);
#end for

#for dict in $interface.dictionaries
  void get_${dict.qname}(const QVariant &variant, ${dict.symbol()} *result);
  QVariant put_${dict.qname}(const ${dict.symbol()} *result);
#end for

  static const DBusMethod method_table[];
  static const char *interface_introspect;
};


${interface.qname} *${interface.qname}::instance(const IDBus::Ptr dbus)
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
  : DBusBindingQt5(dbus)
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
  throw DBusRemoteException(DBUS_ERROR_UNKNOWN_METHOD,
                            std::string("No such member:") + method_name );
}

#for enum in $interface.enums

void
${interface.qname}_Stub::get_${enum.qname}(const QVariant &variant, ${enum.symbol()} *result)
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
      throw DBusRemoteException(DBUS_ERROR_INVALID_ARGS,
                                "Illegal enum value");
    }
}

QVariant
${interface.qname}_Stub::put_${enum.qname}(const ${enum.symbol()} *result)
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
      throw DBusRemoteException(DBUS_ERROR_INVALID_ARGS, "Illegal enum value");
    }

  return put_string(&value);
}

#end for

#for struct in $interface.structs

void
${interface.qname}_Stub::get_${struct.qname}(const QVariant &variant, ${struct.symbol()} *result)
{
#set num_expected_fields = len($struct.fields)

  const QDBusArgument arg = variant.value<QDBusArgument>();

  if (arg.currentType() != QDBusArgument::StructureType)
    {
      throw DBusRemoteException(DBUS_ERROR_INVALID_ARGS,
                                std::string("Incorrect parameter type"));
    }
  
#for p in struct.fields
  #if p.type != p.ext_type
  $interface.get_type(p.ext_type).symbol() _${p.name};
  #end if
#end for

  arg.beginStructure();
#set index = 0
#for p in struct.fields
  if (arg.atEnd())
  {
    throw DBusRemoteException(DBUS_ERROR_INVALID_ARGS,
                              std::string("Too few number of field in struct ${struct.qname}, expected $num_expected_fields "));
  }
    
  QVariant ${p.name} = arg.asVariant();

  #if p.type != p.ext_type
  get_${p.ext_type}(${p.name}, &${p.name});
  #else
  get_${p.ext_type}(${p.name}, ($interface.get_type(p.ext_type).symbol() *) &(result->${p.name}));
  #end if
  #set index = index + 1
#end for
  arg.endStructure();

#for p in struct.fields
  #if p.type != p.ext_type
  result->${p.name} = ($interface.get_type(p.type).symbol()) _${p.name};
  #end if
#end for
}


QVariant
${interface.qname}_Stub::put_${struct.qname}(const ${struct.symbol()} *result)
{
#for p in struct.fields
  #if p.type != p.ext_type
  $interface.get_type(p.ext_type).symbol() f_${p.name} = ($interface.get_type(p.ext_type).symbol())result->${p.name};
  #end if
#end for

  QDBusArgument arg;
  arg.beginStructure();

#for p in struct.fields
  #if p.type != p.ext_type
  QVariant ${p.name} = put_${p.ext_type}(&f_${p.name});
  #else
  QVariant ${p.name} = put_${p.ext_type}(($interface.get_type(p.type).symbol() *) &(result->${p.name}));
  #end if
  arg.appendVariant(${p.name});
#end for

  arg.endStructure();

  return QVariant::fromValue(arg);
}

#end for

#for seq in $interface.sequences

void
${interface.qname}_Stub::get_${seq.qname}(const QVariant &variant, ${seq.symbol()} *result)
{
  const QDBusArgument arg = variant.value<QDBusArgument>();

  if (arg.currentType() != QDBusArgument::ArrayType)
    {
      throw DBusRemoteException(DBUS_ERROR_INVALID_ARGS,
                                std::string("Incorrect parameter type"));
    }
  
  arg.beginArray();
  
  while (!arg.atEnd())
    {
      $interface.get_type(seq.data_type).symbol() tmp;
      get_${seq.data_type}(arg.asVariant(), &tmp);
      result->push_back(tmp);
    }

  arg.endArray();
}

QVariant
${interface.qname}_Stub::put_${seq.qname}(const ${seq.symbol()} *result)
{
  QDBusArgument arg;

  arg.beginArray(qMetaTypeId<$interface.get_type(seq.data_type).symbol_int()>());

  for (auto &i : *result)
    {
      arg.appendVariant(put_${seq.data_type}(&i));
    }

  arg.endArray();

  return QVariant::fromValue(arg);
}
#end for


#for dict in $interface.dictionaries

void
${interface.qname}_Stub::get_${dict.qname}(const QVariant &variant, ${dict.symbol()} *result)
{
  const QDBusArgument arg = variant.value<QDBusArgument>();

  if (arg.currentType() != QDBusArgument::MapType)
    {
      throw DBusRemoteException(DBUS_ERROR_INVALID_ARGS,
                                std::string("Incorrect parameter type"));
    }
  
  arg.beginMap();

  while (!arg.atEnd())
    {
      $interface.get_type(dict.key_type).symbol() key;
      $interface.get_type(dict.value_type).symbol() value;

      arg.beginMapEntry();
      get_${dict.key_type}(arg.asVariant(), &key);
      get_${dict.value_type}(arg.asVariant(), &value);
      arg.endMapEntry();

      (*result)[key] = value;
      arg.endMap();
    }
}


QVariant
${interface.qname}_Stub::put_${dict.qname}(const ${dict.symbol()} *result)
{
  QDBusArgument arg;

  arg.beginMap(qMetaTypeId<$interface.get_type(dict.key_type).symbol_int()>(),
               qMetaTypeId<$interface.get_type(dict.value_type).symbol_int()>());

  for (auto &it : *result)
    {
      arg.beginMapEntry();
      arg.appendVariant(put_${dict.key_type}(&(it.first)));
      arg.appendVariant(put_${dict.value_type}(&(it.second)));
      arg.endMapEntry();
    }
  arg.endMap();

  return QVariant::fromValue(arg);
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

      int num_in_args = message.arguments().size();
      if (num_in_args != $method.num_in_args)
        {
          throw DBusRemoteException(DBUS_ERROR_INVALID_ARGS,
                                    std::string("Incorrect numer of in-parameters of method ${method.name}, expected $method.num_in_args, got ") + boost::lexical_cast<std::string>(num_in_args));
        }

#set index = 0
#for arg in method.params:
  #if $arg.direction == 'in'
      QVariant ${arg.name} = message.arguments().at($index);
      std::cout << "Param " << $index << " " << ${arg.name}.typeName() << std::endl;
      get_${arg.type}(${arg.name}, &p_${arg.name});
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

     QDBusMessage reply = message.createReply();
                                                                
#if method.num_out_args > 0

  #for arg in method.params:
    #if arg.direction == 'out'
      QVariant ${arg.name} = put_${arg.type}(&p_${arg.name});
      reply << ${arg.name};
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
      $comma $interface.get_type(p.type).symbol() p_$p.name#slurp
    #else if 'ptr' in p.hint
      $comma $interface.get_type(p.type).symbol() *p_$p.name#slurp
    #else if 'ref' in p.hint
      $comma $interface.get_type(p.type).symbol() &p_$p.name#slurp
    #end if
  #set comma = ','
  #end for
)
{
  QDBusMessage sig = QDBusMessage::createSignal(QString::fromStdString(path), "${interface.name}", "${signal.name}");
                                               
#if len(signal.params) > 0
  #for arg in signal.params:
    #if 'ptr' in p.hint
  QVariant ${arg.name} = put_${arg.type}(p_${arg.name});
    #else
  QVariant ${arg.name} = put_${arg.type}(&p_${arg.name});
    #end if
  #end for
#end if

  IDBusPrivateQt5::Ptr priv = boost::dynamic_pointer_cast<IDBusPrivateQt5>(dbus);
  priv->get_connection().send(sig);
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
