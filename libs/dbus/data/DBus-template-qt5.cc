\#ifdef HAVE_CONFIG_H
\#include "config.h"
\#endif

\#include <string>
\#include <list>
\#include <map>

\#include <stdlib.h>
\#include <boost/lexical_cast.hpp>

\#include "dbus/DBusBindingQt5.hh"
\#include "dbus/DBusException.hh"
\#include "${model.include_filename}"

using namespace std;
using namespace workrave::dbus;

//
// Marshaller
//

class ${model.name}_Marshall : public DBusMarshallQt5
{
public:
#for enum in $model.enums
#if enum.condition != ''
 \#if $enum.condition
#end if
  void get_${enum.qname}(const QVariant &variant, ${enum.symbol()} &result);
  QVariant put_${enum.qname}(const ${enum.symbol()} &result);
#if enum.condition
 \#endif // $enum.condition
#end if
#end for

#for struct in $model.structs
#if struct.condition
 \#if $struct.condition
#end if
  void get_${struct.qname}(const QVariant &variant, ${struct.symbol()} &result);
  QVariant put_${struct.qname}(const ${struct.symbol()} &result);
#if struct.condition
 \#endif // $struct.condition
#end if
#end for

#for seq in $model.sequences
#if seq.condition
 \#if $seq.condition
#end if
  void get_${seq.qname}(const QVariant &variant, ${seq.symbol()} &result);
  QVariant put_${seq.qname}(const ${seq.symbol()} &result);
#if seq.condition
 \#endif // $seq.condition
#end if
#end for

#for dict in $model.dictionaries
#if dict.condition
 \#if $dict.condition
#end if
  void get_${dict.qname}(const QVariant &variant, ${dict.symbol()} &result);
  QVariant put_${dict.qname}(const ${dict.symbol()} &result);
#if dict.condition
 \#endif // $dict.condition
#end if
#end for
};

//
// Enums
//

#for enum in $model.enums

#if enum.condition != ''
 \#if $enum.condition
#end if

void
${model.name}_Marshall::get_${enum.qname}(const QVariant &variant, ${enum.symbol()} &result)
{
  std::string value;
  try
    {
      get_string(variant, value);
    }
  catch (DBusRemoteException &e)
    {
      e << field_info("${enum.name}");
      throw;
    }

#set if_stm = 'if'
#for e in enum.values
  $if_stm ("$e.name" == value)
    {
      result = $e.symbol();
    }
  #set $if_stm = 'else if'
#end for
  else
    {
      throw DBusRemoteException()
        << message_info("Type error in enum")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << actual_type_info("${enum.name}");
    }
}

QVariant
${model.name}_Marshall::put_${enum.qname}(const ${enum.symbol()} &result)
{
  string value;
  switch (result)
    {
    #for e in enum.values
    case $e.symbol():
      value = "$e.name";
      break;
    #end for
    default:
      throw DBusRemoteException()
        << message_info("Type error in enum")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << actual_type_info("${enum.name}");
    }

  return put_string(value);
}

#if enum.condition
 \#endif // $enum.condition
#end if

#end for

//
// Structs
//
 
#for struct in $model.structs

#if struct.condition
 \#if $struct.condition
#end if
 
void
${model.name}_Marshall::get_${struct.qname}(const QVariant &variant, ${struct.symbol()} &result)
{
#set num_expected_fields = len($struct.fields)

  const QDBusArgument arg = variant.value<QDBusArgument>();

  if (arg.currentType() != QDBusArgument::StructureType)
    {
      throw DBusRemoteException()
        << message_info("Incorrect type")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info("${struct.name}");
    }
  
  arg.beginStructure();
#set index = 0
#for p in struct.fields
  if (arg.atEnd())
  {
      throw DBusRemoteException()
        << message_info("Incorrect number of member in struct")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << actual_type_info("${struct.name}");
  }

  try
    {
      QVariant ${p.name} = arg.asVariant();
      get_${p.type}(${p.name}, result.${p.name});
    }
  catch (DBusRemoteException &e)
    {
      e << field_info("${p.name}");
      throw;
    }
  #set index = index + 1
#end for
  arg.endStructure();
}


QVariant
${model.name}_Marshall::put_${struct.qname}(const ${struct.symbol()} &result)
{
  QDBusArgument arg;

  arg.beginStructure();
#for p in struct.fields
  arg.appendVariant(put_${p.type}(result.${p.name}));
#end for
  arg.endStructure();

  return QVariant::fromValue(arg);
}

#if struct.condition
 \#endif // $struct.condition
#end if

#end for

//
// Sequences
//
 
#for seq in $model.sequences

#if seq.condition
 \#if $seq.condition
#end if
 
void
${model.name}_Marshall::get_${seq.qname}(const QVariant &variant, ${seq.symbol()} &result)
{
  const QDBusArgument arg = variant.value<QDBusArgument>();

  if (arg.currentType() != QDBusArgument::ArrayType)
    {
      throw DBusRemoteException()
        << message_info("Incorrect type")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info("${seq.name}");
    }
  
  arg.beginArray();

  while (!arg.atEnd())
    {
      $model.get_type(seq.data_type).symbol() tmp;
      try
        {
          get_${seq.data_type}(arg.asVariant(), tmp);
        }
      catch (DBusRemoteException &e)
        {
          e << field_info(string("[") + boost::lexical_cast<string>(result.size()) + "]");
          throw;
        }
      
      result.push_back(tmp);
    }

  arg.endArray();
}

QVariant
${model.name}_Marshall::put_${seq.qname}(const ${seq.symbol()} &result)
{
  QDBusArgument arg;

  arg.beginArray(qMetaTypeId<$model.get_type(seq.data_type).symbol_int()>());
  for (auto &i : result)
    {
      arg.appendVariant(put_${seq.data_type}(i));
    }
  arg.endArray();

  return QVariant::fromValue(arg);
}

#if seq.condition
 \#endif // $seq.condition
#end if
#end for

//
// Dictionaries
//

#for dict in $model.dictionaries

#if dict.condition
 \#if $dict.condition
#end if
void
${model.name}_Marshall::get_${dict.qname}(const QVariant &variant, ${dict.symbol()} &result)
{
  const QDBusArgument arg = variant.value<QDBusArgument>();

  if (arg.currentType() != QDBusArgument::MapType)
    {
      throw DBusRemoteException()
        << message_info("Incorrect type")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info("${dict.name}");
    }
  
  arg.beginMap();

  while (!arg.atEnd())
    {
      $model.get_type(dict.key_type).symbol() key;
      $model.get_type(dict.value_type).symbol() value;

      arg.beginMapEntry();
      try
        {
          get_${dict.key_type}(arg.asVariant(), key);
          get_${dict.value_type}(arg.asVariant(), value);
        }
      catch (DBusRemoteException &e)
        {
          e << field_info(key);
          throw;
        }
      arg.endMapEntry();

      (result)[key] = value;
    }

  arg.endMap();
}


QVariant
${model.name}_Marshall::put_${dict.qname}(const ${dict.symbol()} &result)
{
  QDBusArgument arg;

  arg.beginMap(qMetaTypeId<$model.get_type(dict.key_type).symbol_int()>(),
               qMetaTypeId<$model.get_type(dict.value_type).symbol_int()>());

  for (auto &it : result)
    {
      arg.beginMapEntry();
      arg.appendVariant(put_${dict.key_type}(it.first));
      arg.appendVariant(put_${dict.value_type}(it.second));
      arg.endMapEntry();
    }
  arg.endMap();

  return QVariant::fromValue(arg);
}

#if dict.condition
 \#endif // $dict.condition
#end if

#end for

//
// Interfaces
//

#for interface in $model.interfaces

//
// Interface $interface.name
//
 
#if interface.condition != ''
\#if $interface.condition
#end if

#for $ns in $interface.namespace_list
namespace $ns // interface $interface.name namespace
{
#end for
  
class ${interface.qname}_Stub : public DBusBindingQt5, public ${interface.qname}, public ${model.name}_Marshall
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
  void ${m.qname}(void *object, const QDBusMessage &message, const QDBusConnection &connection);
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
  throw DBusRemoteException()
    << message_info("Unknown method")
    << error_code_info(DBUS_ERROR_UNKNOWN_METHOD)
    << method_info(method_name)
    << interface_info("${interface.name}");
}

//
// Interface $interface.name methods
//
  
#for method in $interface.methods

//
// Interface $interface.name method $method.name
//
  
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
          throw DBusRemoteException()
            << message_info("Incorrecy number of in-paraeters")
            << error_code_info(DBUS_ERROR_INVALID_ARGS)
            << method_info("${method.name}")
            << interface_info("${interface.name}");
        }

#set index = 0
#for arg in method.params:
  #if $arg.direction == 'in'
      try
        {
          get_${arg.type}(message.arguments().at($index), p_${arg.name});
        }
      catch (DBusRemoteException &e)
        {
          e << parameter_info("${arg.name}");
          throw;
        }
      #set index = index + 1
  #end if
#end for

#if method.symbol() != ""
  #if method.return_type() != 'void'
      p_$method.return_name() = #slurp
  #end if
  dbus_object->${method.symbol()}( #slurp

  #set comma = ''
  #for p in method.params
    #if 'return' not in p.hint
      #if 'ptr' in p.hint
      $comma &p_$p.name#slurp
      #else
      $comma p_$p.name#slurp
      #end if
      #set comma = ','
    #end if
  #end for
      );
#end if

     QDBusMessage reply = message.createReply();
                                                                
#if method.num_out_args > 0
  #for arg in method.params:
    #if arg.direction == 'out'
      reply << put_${arg.type}(p_${arg.name});
    #end if
  #end for
#end if

      connection.send(reply);
    }
  catch (DBusRemoteException &e)
    {
      e << method_info("${method.name}")
        << interface_info("${interface.name}");
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

//
// Interface $interface.name signals
//
 
#for signal in interface.signals

//
// Interface $interface.name signal $signal.name
//

void ${interface.qname}_Stub::${signal.qname}(const string &path #slurp
  #for p in signal.params
    #if p.hint == []
      , $interface.get_type(p.type).symbol() p_$p.name#slurp
    #else if 'ptr' in p.hint
      , $interface.get_type(p.type).symbol() *p_$p.name#slurp
    #else if 'ref' in p.hint
      , $interface.get_type(p.type).symbol() &p_$p.name#slurp
    #end if
  #end for
)
{
  QDBusMessage sig = QDBusMessage::createSignal(QString::fromStdString(path), "${interface.name}", "${signal.name}");
                                               
#if len(signal.params) > 0
  #for arg in signal.params:
    #if 'ptr' in p.hint
      sig << = put_${arg.type}(* p_${arg.name});
    #else
      sig <<  put_${arg.type}(p_${arg.name});
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

#for $ns in reversed($model.namespace_list)
} // namespace $ns
#end for
