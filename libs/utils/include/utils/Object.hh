#ifndef OBJECT_HH
#define OBJECT_HH

#include "debug.hh"

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace workrave
{
  namespace utils
  {
    class Object : public boost::enable_shared_from_this<Object>
    {
    public:
      Object() : count(0) { }
      
      virtual ~Object() {}

    protected:

      void ref()
      {
        if (count++ == 0)
          {
            me = boost::enable_shared_from_this<Object>::shared_from_this();
          }
      }

      void unref()
      {
        if (--count == 0)
          {
            me.reset();
          }
      }
      
    private:
      int count;
      boost::shared_ptr<Object> me;
    };
  }
}
#endif
