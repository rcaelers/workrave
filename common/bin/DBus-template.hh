//
//

\#ifndef DBUS__HH
\#define DBUS__HH

\#include "DBusBinding.hh"

#for $interface in $model.interfaces

#for imp in interface.imports
#for include in imp.includes
\#include "${include}"
#end for
#end for

#for imp in interface.imports
#for ns in imp.namespaces
using namespace $ns;
#end for
#end for
using namespace std;

class $interface.qname : public DBusBindingBase
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

#end for

\#endif // DBUS__HH
