// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's QtDBus backend from an annotated C++ header.
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

#include "rpc/dbus/Error.hh"
#include "rpc/dbus/QtCodec.hh"

#include "RpcDbusStructSeqDBus.hh"


namespace workrave::rpc::dbus
{
template<>
struct QtCodec<Point>
{
  static Point decode(const QVariant &variant)
  {
    const auto arg = variant.value<QDBusArgument>();
    Point result{};
    arg.beginStructure();

    result.x = QtCodec<int32_t>::decode(arg.asVariant());

    result.y = QtCodec<int32_t>::decode(arg.asVariant());

    arg.endStructure();
    return result;
  }
  static void append(QDBusArgument &arg, const Point &value)
  {
    arg.beginStructure();

    QtCodec<int32_t>::append(arg, value.x);

    QtCodec<int32_t>::append(arg, value.y);

    arg.endStructure();
  }
  static QVariant encode(const Point &value)
  {
    QDBusArgument arg;
    append(arg, value);
    return QVariant::fromValue(arg);
  }
};
} // namespace workrave::rpc::dbus

// See the enum case above for why these are at global scope, not nested in
// workrave::rpc::dbus: ADL needs to find them from Point's own associated
// namespace, not this library's.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const Point &data)
{
  workrave::rpc::dbus::QtCodec<Point>::append(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, Point &data)
{
  data = workrave::rpc::dbus::QtCodec<Point>::decode(QVariant::fromValue(arg));
  return arg;
}

namespace workrave::rpc::dbus
{
template<>
struct QtCodec<std::vector<int>>
{
  static std::vector<int> decode(const QVariant &variant)
  {
    const auto arg = variant.value<QDBusArgument>();
    std::vector<int> result;
    arg.beginArray();
    while (!arg.atEnd())
      {
        result.push_back(QtCodec<int32_t>::decode(arg.asVariant()));
      }
    arg.endArray();
    return result;
  }
  static void append(QDBusArgument &arg, const std::vector<int> &value)
  {
    arg.beginArray(qMetaTypeId<QVariant>());
    for (const auto &item : value)
      {
        QtCodec<int32_t>::append(arg, item);
      }
    arg.endArray();
  }
  static QVariant encode(const std::vector<int> &value)
  {
    QDBusArgument arg;
    append(arg, value);
    return QVariant::fromValue(arg);
  }
};
} // namespace workrave::rpc::dbus

namespace workrave::rpc::dbus
{
template<>
struct QtCodec<std::vector<Point>>
{
  static std::vector<Point> decode(const QVariant &variant)
  {
    const auto arg = variant.value<QDBusArgument>();
    std::vector<Point> result;
    arg.beginArray();
    while (!arg.atEnd())
      {
        result.push_back(QtCodec<Point>::decode(arg.asVariant()));
      }
    arg.endArray();
    return result;
  }
  static void append(QDBusArgument &arg, const std::vector<Point> &value)
  {
    arg.beginArray(qMetaTypeId<QVariant>());
    for (const auto &item : value)
      {
        QtCodec<Point>::append(arg, item);
      }
    arg.endArray();
  }
  static QVariant encode(const std::vector<Point> &value)
  {
    QDBusArgument arg;
    append(arg, value);
    return QVariant::fromValue(arg);
  }
};
} // namespace workrave::rpc::dbus


org_workrave_TestInterface2::org_workrave_TestInterface2(::workrave::rpc::dbus::QtServer &server,
                         std::string path,
                         RpcDBusFixture2 &implementation)

  : path_(std::move(path))

  , implementation_(implementation)
{

  (void)server;


}

std::string_view
org_workrave_TestInterface2::name() const noexcept
{
  return "org.workrave.TestInterface2";
}

std::string_view
org_workrave_TestInterface2::introspection() const noexcept
{
  static constexpr std::string_view xml =

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

  "  </interface>\n";
  return xml;
}

bool
org_workrave_TestInterface2::dispatch(const QDBusMessage &message, const QDBusConnection &connection)
{
  using Method = void (org_workrave_TestInterface2::*)(const QDBusMessage &, const QDBusConnection &);
  struct Entry
  {
    std::string_view name;
    Method method;
  };
  static constexpr std::array<Entry, 4> methods =
  { {

      {.name = "SetPoint", .method = &org_workrave_TestInterface2::dispatch_SetPoint},

      {.name = "GetPoint", .method = &org_workrave_TestInterface2::dispatch_GetPoint},

      {.name = "SetTags", .method = &org_workrave_TestInterface2::dispatch_SetTags},

      {.name = "GetPoints", .method = &org_workrave_TestInterface2::dispatch_GetPoints},

  } };

  const std::string method_name = message.member().toStdString();
  for (const auto &entry: methods)
    {
      if (entry.name == method_name)
        {
          (this->*entry.method)(message, connection);
          return true;
        }
    }
  return false;
}


void
org_workrave_TestInterface2::dispatch_SetPoint(const QDBusMessage &message, const QDBusConnection &connection)
{

  Point p_p{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.TestInterface2.SetPoint");
    }



  p_p = ::workrave::rpc::dbus::QtCodec<Point>::decode(message.arguments().at(0));




  implementation_.set_point(p_p);


  QDBusMessage reply = message.createReply();





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.TestInterface2.SetPoint");
    }
}


void
org_workrave_TestInterface2::dispatch_GetPoint(const QDBusMessage &message, const QDBusConnection &connection)
{


  Point p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.TestInterface2.GetPoint");
    }




  p_result = implementation_.get_point();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<Point>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.TestInterface2.GetPoint");
    }
}


void
org_workrave_TestInterface2::dispatch_SetTags(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::vector<int> p_tags{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.TestInterface2.SetTags");
    }



  p_tags = ::workrave::rpc::dbus::QtCodec<std::vector<int>>::decode(message.arguments().at(0));




  implementation_.set_tags(p_tags);


  QDBusMessage reply = message.createReply();





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.TestInterface2.SetTags");
    }
}


void
org_workrave_TestInterface2::dispatch_GetPoints(const QDBusMessage &message, const QDBusConnection &connection)
{


  std::vector<Point> p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.TestInterface2.GetPoints");
    }




  p_result = implementation_.get_points();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<std::vector<Point>>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.TestInterface2.GetPoints");
    }
}
