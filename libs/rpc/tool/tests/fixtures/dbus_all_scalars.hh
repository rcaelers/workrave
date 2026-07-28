#pragma once

#include <cstdint>
#include <string>

#include <boost/signals2/signal.hpp>

// @rpc(service="workrave.test.DBusAllScalarsService")
// @rpc.dbus(interface="org.workrave.AllScalarsInterface")
class RpcDBusAllScalarsFixture
{
public:
  // @rpc(name="SetAll")
  // @rpc.param(byte_value, dir=in, dbus_type="byte")
  // @rpc.param(boolean_value, dir=in, dbus_type="boolean")
  // @rpc.param(int16_value, dir=in, dbus_type="int16")
  // @rpc.param(uint16_value, dir=in, dbus_type="uint16")
  // @rpc.param(int32_value, dir=in, dbus_type="int32")
  // @rpc.param(uint32_value, dir=in, dbus_type="uint32")
  // @rpc.param(int64_value, dir=in, dbus_type="int64")
  // @rpc.param(uint64_value, dir=in, dbus_type="uint64")
  // @rpc.param(double_value, dir=in, dbus_type="double")
  // @rpc.param(string_value, dir=in, dbus_type="string")
  // @rpc.param(object_path_value, dir=in, dbus_type="object_path")
  // @rpc.param(signature_value, dir=in, dbus_type="signature")
  // @rpc.param(unix_fd_value, dir=in, dbus_type="unix_fd")
  // @rpc.param(variant_value, dir=in, dbus_type="variant")
  void set_all(int64_t byte_value,
               bool boolean_value,
               int64_t int16_value,
               uint64_t uint16_value,
               int64_t int32_value,
               uint64_t uint32_value,
               int64_t int64_value,
               uint64_t uint64_value,
               float double_value,
               std::string string_value,
               std::string object_path_value,
               std::string signature_value,
               int unix_fd_value,
               int64_t variant_value);

  // @rpc(name="GetObjectPath")
  // @rpc.dbus(return_type="object_path")
  std::string get_object_path();

  // Exercise scalar overrides on output parameters as well as inputs,
  // returns, and signal fields.
  // @rpc(name="GetSpecial")
  // @rpc.param(signature_value, dir=out, dbus_type="signature")
  // @rpc.param(unix_fd_value, dir=out, dbus_type="unix_fd")
  void get_special(std::string &signature_value, int &unix_fd_value);

  // Keep each fixture signal below the arity limit of older Boost.Signals2.
  // @rpc.signal(name="NumericChanged", fields="byte_value,boolean_value,int16_value,uint16_value,int32_value,uint32_value,int64_value,uint64_value,double_value", dbus_types="byte,boolean,int16,uint16,int32,uint32,int64,uint64,double")
  boost::signals2::signal<void(int64_t,
                              bool,
                              int64_t,
                              uint64_t,
                              int64_t,
                              uint64_t,
                              int64_t,
                              uint64_t,
                              float)> &signal_numeric_changed()
  {
    return numeric_changed_;
  }

  // @rpc.signal(name="SpecialChanged", fields="string_value,object_path_value,signature_value,unix_fd_value,variant_value", dbus_types="string,object_path,signature,unix_fd,variant")
  boost::signals2::signal<void(std::string, std::string, std::string, int, int64_t)> &signal_special_changed()
  {
    return special_changed_;
  }

private:
  boost::signals2::signal<void(int64_t,
                               bool,
                               int64_t,
                               uint64_t,
                               int64_t,
                               uint64_t,
                               int64_t,
                               uint64_t,
                               float)> numeric_changed_;
  boost::signals2::signal<void(std::string, std::string, std::string, int, int64_t)> special_changed_;
};
