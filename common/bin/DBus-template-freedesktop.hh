//
//

\#ifndef DBUS__HH
\#define DBUS__HH

\#ifdef HAVE_CONFIG_H
\#include "config.h"
\#endif

\#include "DBusBinding-freedesktop.hh"

#for $interface in $model.interfaces

#if interface.condition != ''
\#if $interface.condition
#end if

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

class $interface.qname
{
public:
  virtual ~${interface.qname}() {}

  static $interface.qname *instance(const DBus *dbus);

  #for $m in interface.signals
  virtual void ${m.qname}(const string &path, #slurp
  #set comma = ''
  #for p in m.params
  $comma $interface.type2csymbol(p.type) $p.name#slurp
  #set comma = ','
  #end for
  ) = 0;
  #end for
};

#if interface.condition != ''
\#endif // $interface.condition
#end if

#end for

#endif // DBUS__HH
