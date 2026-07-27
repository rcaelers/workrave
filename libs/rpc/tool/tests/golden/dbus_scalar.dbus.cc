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
#include <vector>

#include "dbus/DBusBindingQt.hh"
#include "dbus/DBusException.hh"

#include "RpcDbusScalarDBus.hh"

using namespace workrave::dbus;


namespace workrave::dbus
{
template<>
struct DBusMarshall<TestMode>
{
  static TestMode convert(const QVariant &variant)
  {
    QString arg = variant.value<QString>();
    TestMode value{};

    if ("idle" == arg) { value = TestMode::Idle; }

    if ("active" == arg) { value = TestMode::Active; }

    return value;
  }
  static void marshall(QDBusArgument &arg, const TestMode &value)
  {
    std::string str;
    switch (value)
      {

      case TestMode::Idle: str = "idle"; break;

      case TestMode::Active: str = "active"; break;

      default:
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }
    arg << QString::fromStdString(str);
  }
  static QVariant convert(const TestMode &value)
  {
    std::string str;
    switch (value)
      {

      case TestMode::Idle: str = "idle"; break;

      case TestMode::Active: str = "active"; break;

      default:
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }
    return QVariant::fromValue(QString::fromStdString(str));
  }
};
} // namespace workrave::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::dbus, where they'd only be found via TestMode's
// own namespace (which may not be workrave::dbus at all), silently breaks
// qDBusRegisterMetaType<TestMode>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const TestMode &data)
{
  workrave::dbus::DBusMarshall<TestMode>::marshall(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, TestMode &data)
{
  QString value;
  arg >> value;
  data = workrave::dbus::DBusMarshall<TestMode>::convert(QVariant::fromValue(value));
  return arg;
}


class org_workrave_TestInterface_Stub : public ::workrave::dbus::DBusBindingQt, public org_workrave_TestInterface
{
private:
  using DBusMethodPointer = void (org_workrave_TestInterface_Stub::*)(void *object, const QDBusMessage &message, const QDBusConnection &connection);

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
  explicit org_workrave_TestInterface_Stub(std::shared_ptr<::workrave::dbus::IDBus> dbus) : ::workrave::dbus::DBusBindingQt(std::move(dbus)) {}
  ~org_workrave_TestInterface_Stub() override = default;


  void ModeChanged(const std::string &path, TestMode value) override;


private:

  void Ping(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetMode(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void SetMode(void *object, const QDBusMessage &message, const QDBusConnection &connection);


  static constexpr std::array<DBusMethod, 3> method_table =
  { {

      {.name = "Ping", .fn = &org_workrave_TestInterface_Stub::Ping},

      {.name = "GetMode", .fn = &org_workrave_TestInterface_Stub::GetMode},

      {.name = "SetMode", .fn = &org_workrave_TestInterface_Stub::SetMode},

  } };

  static constexpr std::string_view interface_introspect =

  "  <interface name=\"org.workrave.TestInterface\">\n"

  "\n"

  "    <method name=\"Ping\">\n"

  "\n"

  "      <arg type=\"s\" name=\"message\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetMode\">\n"

  "\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"SetMode\">\n"

  "\n"

  "      <arg type=\"s\" name=\"mode\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "\n"

  "    <signal name=\"ModeChanged\">\n"

  "\n"

  "      <arg type=\"s\" name=\"value\" />\n"

  "\n"

  "    </signal>\n"

  "\n"

  "  </interface>\n"

;
};

org_workrave_TestInterface *org_workrave_TestInterface::instance(std::shared_ptr<::workrave::dbus::IDBus> dbus)
{
  org_workrave_TestInterface_Stub *iface = nullptr;
  ::workrave::dbus::DBusBinding *binding = dbus->find_binding("org.workrave.TestInterface");
  if (binding != nullptr)
    {
      iface = dynamic_cast<org_workrave_TestInterface_Stub *>(binding);
    }
  return iface;
}

bool
org_workrave_TestInterface_Stub::call(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  std::string method_name = message.member().toStdString();
  constexpr std::array<DBusMethod, 3> table = method_table;
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
    << ::workrave::dbus::interface_info("org.workrave.TestInterface");
}


void
org_workrave_TestInterface_Stub::Ping(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<RpcDBusFixture *>(object);


      std::string p_message{};


      std::string p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 1)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("Ping")
            << workrave::dbus::interface_info("org.workrave.TestInterface");
        }



      p_message = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));




      p_result = dbus_object->ping(p_message);


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<std::string>::convert(p_result);





      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("Ping")
            << workrave::dbus::interface_info("org.workrave.TestInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("Ping") << interface_info("org.workrave.TestInterface");
      throw;
    }
}


void
org_workrave_TestInterface_Stub::GetMode(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<RpcDBusFixture *>(object);



      TestMode p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetMode")
            << workrave::dbus::interface_info("org.workrave.TestInterface");
        }




      p_result = dbus_object->get_mode();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<TestMode>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetMode")
            << workrave::dbus::interface_info("org.workrave.TestInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetMode") << interface_info("org.workrave.TestInterface");
      throw;
    }
}


void
org_workrave_TestInterface_Stub::SetMode(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<RpcDBusFixture *>(object);


      TestMode p_mode{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 1)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("SetMode")
            << workrave::dbus::interface_info("org.workrave.TestInterface");
        }



      p_mode = ::workrave::dbus::DBusMarshall<TestMode>::convert(message.arguments().at(0));




      dbus_object->set_mode(p_mode);


      QDBusMessage reply = message.createReply();





      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("SetMode")
            << workrave::dbus::interface_info("org.workrave.TestInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("SetMode") << interface_info("org.workrave.TestInterface");
      throw;
    }
}



void
org_workrave_TestInterface_Stub::ModeChanged(const std::string &path, TestMode value)
{
  QDBusMessage sig = QDBusMessage::createSignal(QString::fromStdString(path), "org.workrave.TestInterface", "ModeChanged");

  sig << ::workrave::dbus::DBusMarshall<TestMode>::convert(value);

  ::workrave::dbus::IDBusPrivateQt::Ptr priv = std::dynamic_pointer_cast<::workrave::dbus::IDBusPrivateQt>(dbus);
  priv->get_connection().send(sig);
}


void init_org_workrave_TestInterface(std::shared_ptr<::workrave::dbus::IDBus> dbus)
{
  dbus->register_binding("org.workrave.TestInterface", new org_workrave_TestInterface_Stub(dbus));


  qDBusRegisterMetaType<TestMode>();

}
