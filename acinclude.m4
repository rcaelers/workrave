dnl Macro: AC_CHECK_CXX_EH
dnl Sets $ac_cv_cxx_eh to yes or no

AC_DEFUN(AC_CHECK_CXX_EH,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_MSG_CHECKING([whether the C++ compiler ($CXX $CXXFLAGS) has correct exception handling])
AC_CACHE_VAL(ac_cv_cxx_eh,
[
AC_TRY_RUN(
[
#include <exception>
#include <string.h>

using namespace std;

struct test : public exception {
	virtual const char* what() const throw() { return "test"; }
};

static void func() { throw test(); }
int main(void)
{
	try {
		func();
	} catch(exception& e) {
		return (strcmp(e.what(),"test")!=0);
	} catch(...) { return 1; }
	return 1;
}
],
ac_cv_cxx_eh=yes,
ac_cv_cxx_eh=no,
ac_cv_cxx_eh=yes)
])
AC_MSG_RESULT([$ac_cv_cxx_eh])
if test "x$ac_cv_cxx_eh" = "xyes"
then
	AC_DEFINE(HAVE_CXX_EH,,[Do we have exception handling?])
fi
AC_LANG_RESTORE
])


dnl Macro: AC_CHECK_CXX_NS
dnl Test if the c++ compiler supports namespaces
dnl Set $ac_cv_cxx_ns to either yes or no
dnl Define HAVE_CXX_NS if yes
ss
AC_DEFUN(AC_CHECK_CXX_NS,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_MSG_CHECKING([whether the C++ compiler ($CXX $CXXFLAGS) supports namespaces])
AC_CACHE_VAL(ac_cv_cxx_ns,
[
AC_TRY_COMPILE([
namespace A {
	namespace B {
		struct X {};
	};
};
],[
	A::B::X x;
],
ac_cv_cxx_ns=yes,
ac_cv_cxx_ns=no)
])

AC_MSG_RESULT([$ac_cv_cxx_ns])

if test "x$ac_cv_cxx_ns" = "xyes"
then
	AC_DEFINE(HAVE_CXX_NS,,[Do we have namespaces?])
fi
AC_LANG_RESTORE
])


dnl Copied from LibUC++

dnl Macro: AC_CHECK_CXX_STL
dnl Sets $ac_cv_cxx_stl to yes or no
dnl defines HAVE_CXX_STL if ok

AC_DEFUN(AC_CHECK_CXX_STL,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_MSG_CHECKING([whether STL is available])
AC_CACHE_VAL(ac_cv_cxx_stl,
[
AC_TRY_COMPILE([
#include <set>

using namespace std;
],[
set<int> t;
t.insert(t.begin(),1);
set<int>::iterator i=t.find(1);
],
ac_cv_cxx_stl=yes,
ac_cv_cxx_stl=no)
])
AC_MSG_RESULT($ac_cv_cxx_stl)
if test "x$ac_cv_cxx_stl" = "xyes"
then
	AC_DEFINE(HAVE_CXX_STL,,[Do we have STL?])
fi
AC_LANG_RESTORE
])


dnl Macro: AC_CHECK_DOXYGEN
dnl Checks for doxygen and perl, sets $doxygen to path if both are found
dnl Otherwise, $doxygen will be empty
AC_DEFUN(AC_CHECK_DOXYGEN,
[
	AC_PATH_PROG(doxygen,doxygen)
	AC_PATH_PROG(perl,perl)
	if test "x$doxygen" = "x" || test "x$perl" = "x"
	then
		doxygen=""
	fi
	AC_SUBST(doxygen)
	AC_SUBST(perl)
])


dnl AS_AC_EXPAND(VAR, CONFIGURE_VAR)
dnl
dnl example
dnl AS_AC_EXPAND(SYSCONFDIR, $sysconfdir)
dnl will set SYSCONFDIR to /usr/local/etc if prefix=/usr/local

AC_DEFUN([AS_AC_EXPAND],
[
  EXP_VAR=[$1]
  FROM_VAR=[$2]

  dnl first expand prefix and exec_prefix if necessary
  prefix_save=$prefix
  exec_prefix_save=$exec_prefix

  dnl if no prefix given, then use /usr/local, the default prefix
  if test "x$prefix" = "xNONE"; then
    prefix=$ac_default_prefix
  fi
  dnl if no exec_prefix given, then use prefix
  if test "x$exec_prefix" = "xNONE"; then
    exec_prefix=$prefix
  fi

  full_var="$FROM_VAR"
  dnl loop until it doesn't change anymore
  while true; do
    new_full_var="`eval echo $full_var`"
    if test "x$new_full_var"="x$full_var"; then break; fi
    full_var=$new_full_var
  done

  dnl clean up
  full_var=$new_full_var
  AC_SUBST([$1], "$full_var")

  dnl restore prefix and exec_prefix
  prefix=$prefix_save
  exec_prefix=$exec_prefix_save
])