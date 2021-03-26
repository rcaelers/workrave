#ifndef WORKRAVE_UTILS_OBJECT_HH
#define WORKRAVE_UTILS_OBJECT_HH

#include "debug.hh"

#include <memory>

namespace workrave
{
  namespace utils
  {
    class Object : public std::enable_shared_from_this<Object>
    {
    public:
      Object()
        : count(0)
      {
      }

      virtual ~Object() {}

    protected:
      void ref()
      {
        if (count++ == 0)
          {
            me = std::enable_shared_from_this<Object>::shared_from_this();
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
      std::shared_ptr<Object> me;
    };
  } // namespace utils
} // namespace workrave
#endif
