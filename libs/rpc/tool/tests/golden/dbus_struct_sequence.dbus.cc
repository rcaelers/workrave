// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's DBus backend from an annotated C++ header.
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <array>
#include <chrono>
#include <list>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "dbus/DBusBindingQt.hh"
#include "dbus/DBusException.hh"

#include "RpcDbusStructSeqDBus.hh"

using namespace workrave::dbus;

template<typename To, typename From>
To dbus_checked_integer_cast(From value)
{
  static_assert(std::is_integral_v<To> && std::is_integral_v<From>);
  if (!std::in_range<To>(value))
    {
      throw ::workrave::dbus::DBusRemoteException()
        << ::workrave::dbus::message_info("DBus integer conversion is out of range")
        << ::workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
    }
  return static_cast<To>(value);
}


namespace workrave::dbus
{
template<>
struct DBusMarshall<Point>
{
  static Point convert(const QVariant &variant)
  {
    const auto arg = variant.value<QDBusArgument>();
    Point result{};
    arg.beginStructure();

    result.x = DBusMarshall<int32_t>::convert(arg.asVariant());

    result.y = DBusMarshall<int32_t>::convert(arg.asVariant());

    arg.endStructure();
    return result;
  }
  static void marshall(QDBusArgument &arg, const Point &value)
  {
    arg.beginStructure();

    DBusMarshall<int32_t>::marshall(arg, value.x);

    DBusMarshall<int32_t>::marshall(arg, value.y);

    arg.endStructure();
  }
  static QVariant convert(const Point &value)
  {
    QDBusArgument arg;
    marshall(arg, value);
    return QVariant::fromValue(arg);
  }
};
} // namespace workrave::dbus

// See the enum case above for why these are at global scope, not nested in
// workrave::dbus: ADL needs to find them from Point's own associated
// namespace, not this library's.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const Point &data)
{
  workrave::dbus::DBusMarshall<Point>::marshall(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, Point &data)
{
  data = workrave::dbus::DBusMarshall<Point>::convert(QVariant::fromValue(arg));
  return arg;
}

namespace workrave::dbus
{
template<>
struct DBusMarshall<std::vector<int>>
{
  static std::vector<int> convert(const QVariant &variant)
  {
    const auto arg = variant.value<QDBusArgument>();
    std::vector<int> result;
    arg.beginArray();
    while (!arg.atEnd())
      {
        result.push_back(DBusMarshall<int32_t>::convert(arg.asVariant()));
      }
    arg.endArray();
    return result;
  }
  static void marshall(QDBusArgument &arg, const std::vector<int> &value)
  {
    arg.beginArray(qMetaTypeId<QVariant>());
    for (const auto &item : value)
      {
        DBusMarshall<int32_t>::marshall(arg, item);
      }
    arg.endArray();
  }
  static QVariant convert(const std::vector<int> &value)
  {
    QDBusArgument arg;
    marshall(arg, value);
    return QVariant::fromValue(arg);
  }
};
} // namespace workrave::dbus

namespace workrave::dbus
{
template<>
struct DBusMarshall<std::vector<Point>>
{
  static std::vector<Point> convert(const QVariant &variant)
  {
    const auto arg = variant.value<QDBusArgument>();
    std::vector<Point> result;
    arg.beginArray();
    while (!arg.atEnd())
      {
        result.push_back(DBusMarshall<Point>::convert(arg.asVariant()));
      }
    arg.endArray();
    return result;
  }
  static void marshall(QDBusArgument &arg, const std::vector<Point> &value)
  {
    arg.beginArray(qMetaTypeId<QVariant>());
    for (const auto &item : value)
      {
        DBusMarshall<Point>::marshall(arg, item);
      }
    arg.endArray();
  }
  static QVariant convert(const std::vector<Point> &value)
  {
    QDBusArgument arg;
    marshall(arg, value);
    return QVariant::fromValue(arg);
  }
};
} // namespace workrave::dbus


class org_workrave_TestInterface2_Stub : public ::workrave::dbus::DBusBindingQt, public org_workrave_TestInterface2
{
private:
  using DBusMethodPointer = void (org_workrave_TestInterface2_Stub::*)(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  struct DBusMethod
  {
    std::string_view name;
    DBusMethodPointer fn;
  };

  bool call(void *object, const QDBusMessage &message, const QDBusConnection &connection) override;

  std::string_view get_interface_introspect() override
  {
    return interface_introspect;
  }

public:
  explicit org_workrave_TestInterface2_Stub(std::shared_ptr<::workrave::dbus::IDBus> dbus) : ::workrave::dbus::DBusBindingQt(std::move(dbus)) {}
  ~org_workrave_TestInterface2_Stub() override = default;



private:

  void SetPoint(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetPoint(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void SetTags(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetPoints(void *object, const QDBusMessage &message, const QDBusConnection &connection);


  static constexpr std::array<DBusMethod, 4> method_table =
  { {

      {.name = "SetPoint", .fn = &org_workrave_TestInterface2_Stub::SetPoint},

      {.name = "GetPoint", .fn = &org_workrave_TestInterface2_Stub::GetPoint},

      {.name = "SetTags", .fn = &org_workrave_TestInterface2_Stub::SetTags},

      {.name = "GetPoints", .fn = &org_workrave_TestInterface2_Stub::GetPoints},

  } };

  static constexpr std::string_view interface_introspect =

  "  <interface name=\"org.workrave.TestInterface2\">\n"

  "\n"

  "    <method name=\"SetPoint\">\n"

  "\n"

  "      <arg type=\"(ii)\" name=\"p\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetPoint\">\n"

  "\n"

  "      <arg type=\"(ii)\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"SetTags\">\n"

  "\n"

  "      <arg type=\"ai\" name=\"tags\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetPoints\">\n"

  "\n"

  "      <arg type=\"a(ii)\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "\n"

  "  </interface>\n"

;
};

org_workrave_TestInterface2 *org_workrave_TestInterface2::instance(std::shared_ptr<::workrave::dbus::IDBus> dbus)
{
  org_workrave_TestInterface2_Stub *iface = nullptr;
  ::workrave::dbus::DBusBinding *binding = dbus->find_binding("org.workrave.TestInterface2");
  if (binding != nullptr)
    {
      iface = dynamic_cast<org_workrave_TestInterface2_Stub *>(binding);
    }
  return iface;
}

bool
org_workrave_TestInterface2_Stub::call(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  std::string method_name = message.member().toStdString();
  constexpr std::array<DBusMethod, 4> table = method_table;
  for (const auto &method : table)
    {
      if (method_name == method.name)
        {
          DBusMethodPointer ptr = method.fn;
          if (ptr != nullptr)
            {
              (this->*ptr)(object, message, connection);
            }
          return true;
        }
    }
  throw ::workrave::dbus::DBusRemoteException()
    << ::workrave::dbus::message_info("Unknown method")
    << ::workrave::dbus::error_code_info(DBUS_ERROR_UNKNOWN_METHOD)
    << ::workrave::dbus::method_info(method_name)
    << ::workrave::dbus::interface_info("org.workrave.TestInterface2");
}


void
org_workrave_TestInterface2_Stub::SetPoint(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<RpcDBusFixture2 *>(object);


      Point p_p{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 1)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("SetPoint")
            << workrave::dbus::interface_info("org.workrave.TestInterface2");
        }



      p_p = ::workrave::dbus::DBusMarshall<Point>::convert(message.arguments().at(0));




      dbus_object->set_point(p_p);


      QDBusMessage reply = message.createReply();





      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("SetPoint")
            << workrave::dbus::interface_info("org.workrave.TestInterface2");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("SetPoint") << interface_info("org.workrave.TestInterface2");
      throw;
    }
}


void
org_workrave_TestInterface2_Stub::GetPoint(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<RpcDBusFixture2 *>(object);



      Point p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetPoint")
            << workrave::dbus::interface_info("org.workrave.TestInterface2");
        }




      p_result = dbus_object->get_point();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<Point>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetPoint")
            << workrave::dbus::interface_info("org.workrave.TestInterface2");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetPoint") << interface_info("org.workrave.TestInterface2");
      throw;
    }
}


void
org_workrave_TestInterface2_Stub::SetTags(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<RpcDBusFixture2 *>(object);


      std::vector<int> p_tags{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 1)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("SetTags")
            << workrave::dbus::interface_info("org.workrave.TestInterface2");
        }



      p_tags = ::workrave::dbus::DBusMarshall<std::vector<int>>::convert(message.arguments().at(0));




      dbus_object->set_tags(p_tags);


      QDBusMessage reply = message.createReply();





      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("SetTags")
            << workrave::dbus::interface_info("org.workrave.TestInterface2");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("SetTags") << interface_info("org.workrave.TestInterface2");
      throw;
    }
}


void
org_workrave_TestInterface2_Stub::GetPoints(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<RpcDBusFixture2 *>(object);



      std::vector<Point> p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetPoints")
            << workrave::dbus::interface_info("org.workrave.TestInterface2");
        }




      p_result = dbus_object->get_points();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<std::vector<Point>>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetPoints")
            << workrave::dbus::interface_info("org.workrave.TestInterface2");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetPoints") << interface_info("org.workrave.TestInterface2");
      throw;
    }
}



void init_org_workrave_TestInterface2(std::shared_ptr<::workrave::dbus::IDBus> dbus)
{
  dbus->register_binding("org.workrave.TestInterface2", new org_workrave_TestInterface2_Stub(dbus));


}
