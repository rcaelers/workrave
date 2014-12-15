//
//

\#ifndef DBUS_${model.guard}_HH
\#define DBUS_${model.guard}_HH

\#include "dbus/DBusBindingGio.hh"

using namespace std;

#for imp in $model.imports
#if $imp.condition != ''
\#if $imp.condition
#end if
#for include, condition in imp.includes
#if condition != ''
\#if $condition
#end if
\#include "${include}"
#if condition != ''
\#endif // $condition
#end if
#end for

#for ns, condition in imp.namespaces
#if condition != ''
\#if $condition
#end if
using namespace $ns;
#if condition != ''
\#endif // $condition
#end if
#end for

#if $imp.condition != ''
\#endif // $imp.condition
#end if
#end for
 
  
#for $interface in $model.interfaces

#if interface.condition != ''
\#if $interface.condition
#end if

#for $ns in $interface.namespace_list
namespace $ns // interface $interface.name namespace
{
#end for

class $interface.qname
{
public:
  virtual ~${interface.qname}() {}

  static $interface.qname *instance(const workrave::dbus::IDBus::Ptr dbus);

#for $m in interface.signals
  virtual void ${m.qname}(const string &path #slurp
  #for p in m.params
    #if p.hint == [] 
      , $interface.get_type(p.type).symbol() $p.name#slurp
    #else if 'ptr' in p.hint
      , $interface.get_type(p.type).symbol() *$p.name#slurp
    #else if 'ref' in p.hint
      , $interface.get_type(p.type).symbol() &$p.name#slurp
    #end if
  #end for
  ) = 0;
#end for
};

#for $ns in reversed($interface.namespace_list)
} // namespace $ns
#end for

#if interface.condition != ''
\#endif // $interface.condition
#end if

#end for

#endif // DBUS_${model.guard}_HH
