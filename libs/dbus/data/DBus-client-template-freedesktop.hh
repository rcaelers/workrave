//
//

\#ifndef DBUS__${model.name}_HH
\#define DBUS__${model.name}_HH

\#include <sigc++/sigc++.h>
\#include "DBus.hh"
\#include <string>

namespace workrave
{
  class DBus;
}

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
  static $interface.qname *instance(workrave::DBus *dbus, const std::string &service, const std::string &path);

  #for $m in interface.methods
  typedef sigc::slot<void, DBusError * #slurp
  #set comma = ','
  #for p in m.params
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
  > ${m.qname}_slot;
  #slurp
  virtual $interface.type2csymbol(m.return_type()) ${m.qname}(#slurp
  #set comma = ''
  #for p in m.params
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
  ) = 0;
  virtual void ${m.qname}_async(#slurp
  #set comma = ''
  #for p in m.params
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
    $comma ${m.name}_slot slot #slurp
  ) = 0;
  #end for
};

#if interface.condition != ''
\#endif // $interface.condition
#end if

#end for

\#endif // DBUS__HH
