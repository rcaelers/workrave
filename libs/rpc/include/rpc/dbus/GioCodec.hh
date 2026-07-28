// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <gio/gio.h>

#include "rpc/dbus/Error.hh"
#include "rpc/dbus/WireTypes.hh"

namespace workrave::rpc::dbus
{
  class GioVariant
  {
  public:
    explicit GioVariant(GVariant *value = nullptr) noexcept
      : value_(value)
    {
    }

    ~GioVariant()
    {
      if (value_ != nullptr)
        {
          g_variant_unref(value_);
        }
    }

    GioVariant(const GioVariant &) = delete;
    GioVariant &operator=(const GioVariant &) = delete;

    GioVariant(GioVariant &&other) noexcept
      : value_(std::exchange(other.value_, nullptr))
    {
    }

    GioVariant &operator=(GioVariant &&other) noexcept
    {
      if (this != &other)
        {
          if (value_ != nullptr)
            {
              g_variant_unref(value_);
            }
          value_ = std::exchange(other.value_, nullptr);
        }
      return *this;
    }

    [[nodiscard]] GVariant *get() const noexcept
    {
      return value_;
    }

  private:
    GVariant *value_;
  };

  template<typename>
  inline constexpr bool gio_codec_unsupported = false;

  template<typename T>
  struct GioSignature
  {
    static std::string value()
    {
      using Value = std::remove_cv_t<T>;
      if constexpr (std::is_same_v<Value, bool>)
        return "b";
      else if constexpr (std::is_same_v<Value, uint8_t>)
        return "y";
      else if constexpr (std::is_integral_v<Value> && std::is_signed_v<Value> && sizeof(Value) == 2)
        return "n";
      else if constexpr (std::is_integral_v<Value> && std::is_unsigned_v<Value> && sizeof(Value) == 2)
        return "q";
      else if constexpr (std::is_integral_v<Value> && std::is_signed_v<Value> && sizeof(Value) == 4)
        return "i";
      else if constexpr (std::is_integral_v<Value> && std::is_unsigned_v<Value> && sizeof(Value) == 4)
        return "u";
      else if constexpr (std::is_integral_v<Value> && std::is_signed_v<Value> && sizeof(Value) == 8)
        return "x";
      else if constexpr (std::is_integral_v<Value> && std::is_unsigned_v<Value> && sizeof(Value) == 8)
        return "t";
      else if constexpr (std::is_floating_point_v<Value>)
        return "d";
      else
        static_assert(gio_codec_unsupported<T>, "No GIO DBus signature for this C++ type");
    }
  };

  template<>
  struct GioSignature<std::string>
  {
    static std::string value()
    {
      return "s";
    }
  };

  inline void gio_require_type(GVariant *variant, const std::string &signature)
  {
    if (variant == nullptr)
      {
        throw Error(std::string(error_names::invalid_args), "Missing DBus argument");
      }
    GVariantType *type = g_variant_type_new(signature.c_str());
    const bool matches = g_variant_is_of_type(variant, type);
    g_variant_type_free(type);
    if (!matches)
      {
        throw Error(std::string(error_names::invalid_args),
                    "Incorrect DBus argument type; expected " + signature);
      }
  }

  template<typename T>
  struct GioCodec
  {
    static T decode(GVariant *variant)
    {
      gio_require_type(variant, GioSignature<T>::value());
      if constexpr (std::is_same_v<T, bool>)
        return g_variant_get_boolean(variant) != FALSE;
      else if constexpr (std::is_same_v<T, uint8_t>)
        return g_variant_get_byte(variant);
      else if constexpr (std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) == 2)
        return static_cast<T>(g_variant_get_int16(variant));
      else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) == 2)
        return static_cast<T>(g_variant_get_uint16(variant));
      else if constexpr (std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) == 4)
        return static_cast<T>(g_variant_get_int32(variant));
      else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) == 4)
        return static_cast<T>(g_variant_get_uint32(variant));
      else if constexpr (std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) == 8)
        return static_cast<T>(g_variant_get_int64(variant));
      else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) == 8)
        return static_cast<T>(g_variant_get_uint64(variant));
      else if constexpr (std::is_floating_point_v<T>)
        return static_cast<T>(g_variant_get_double(variant));
      else
        static_assert(gio_codec_unsupported<T>, "No GIO DBus codec for this C++ type");
    }

    static GVariant *encode(T value)
    {
      if constexpr (std::is_same_v<T, bool>)
        return g_variant_new_boolean(value);
      else if constexpr (std::is_same_v<T, uint8_t>)
        return g_variant_new_byte(value);
      else if constexpr (std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) == 2)
        return g_variant_new_int16(static_cast<gint16>(value));
      else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) == 2)
        return g_variant_new_uint16(static_cast<guint16>(value));
      else if constexpr (std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) == 4)
        return g_variant_new_int32(static_cast<gint32>(value));
      else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) == 4)
        return g_variant_new_uint32(static_cast<guint32>(value));
      else if constexpr (std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) == 8)
        return g_variant_new_int64(static_cast<gint64>(value));
      else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) == 8)
        return g_variant_new_uint64(static_cast<guint64>(value));
      else if constexpr (std::is_floating_point_v<T>)
        return g_variant_new_double(static_cast<double>(value));
      else
        static_assert(gio_codec_unsupported<T>, "No GIO DBus codec for this C++ type");
    }
  };

  template<>
  struct GioCodec<std::string>
  {
    static std::string decode(GVariant *variant)
    {
      gio_require_type(variant, "s");
      return g_variant_get_string(variant, nullptr);
    }

    static GVariant *encode(const std::string &value)
    {
      return g_variant_new_string(value.c_str());
    }
  };

  template<>
  struct GioSignature<ObjectPath>
  {
    static std::string value()
    {
      return "o";
    }
  };

  template<>
  struct GioCodec<ObjectPath>
  {
    static ObjectPath decode(GVariant *variant)
    {
      gio_require_type(variant, "o");
      return ObjectPath{g_variant_get_string(variant, nullptr)};
    }

    static GVariant *encode(const ObjectPath &value)
    {
      if (!g_variant_is_object_path(value.value.c_str()))
        {
          throw Error(std::string(error_names::invalid_args),
                      "Invalid D-Bus object path: " + value.value);
        }
      return g_variant_new_object_path(value.value.c_str());
    }
  };

  template<>
  struct GioSignature<Signature>
  {
    static std::string value()
    {
      return "g";
    }
  };

  template<>
  struct GioCodec<Signature>
  {
    static Signature decode(GVariant *variant)
    {
      gio_require_type(variant, "g");
      return Signature{g_variant_get_string(variant, nullptr)};
    }

    static GVariant *encode(const Signature &value)
    {
      if (!g_variant_is_signature(value.value.c_str()))
        {
          throw Error(std::string(error_names::invalid_args),
                      "Invalid D-Bus signature: " + value.value);
        }
      return g_variant_new_signature(value.value.c_str());
    }
  };

  template<>
  struct GioSignature<UnixFd>
  {
    static std::string value()
    {
      return "h";
    }
  };

  template<>
  struct GioCodec<UnixFd>
  {
    static UnixFd decode(GVariant *variant)
    {
      gio_require_type(variant, "h");
      return UnixFd{g_variant_get_handle(variant)};
    }

    static GVariant *encode(UnixFd value)
    {
      return g_variant_new_handle(value.value);
    }
  };

  template<typename Value>
  struct GioSignature<Variant<Value>>
  {
    static std::string value()
    {
      return "v";
    }
  };

  template<typename Value>
  struct GioCodec<Variant<Value>>
  {
    static Variant<Value> decode(GVariant *variant)
    {
      gio_require_type(variant, "v");
      GioVariant child(g_variant_get_variant(variant));
      return Variant<Value>{GioCodec<Value>::decode(child.get())};
    }

    static GVariant *encode(const Variant<Value> &value)
    {
      return g_variant_new_variant(GioCodec<Value>::encode(value.value));
    }
  };

  template<typename Value>
  struct GioSignature<std::vector<Value>>
  {
    static std::string value()
    {
      return "a" + GioSignature<Value>::value();
    }
  };

  template<typename Value>
  struct GioCodec<std::vector<Value>>
  {
    static std::vector<Value> decode(GVariant *variant)
    {
      gio_require_type(variant, GioSignature<std::vector<Value>>::value());
      std::vector<Value> result;
      const gsize count = g_variant_n_children(variant);
      result.reserve(count);
      for (gsize index = 0; index < count; ++index)
        {
          GioVariant child(g_variant_get_child_value(variant, index));
          result.push_back(GioCodec<Value>::decode(child.get()));
        }
      return result;
    }

    static GVariant *encode(const std::vector<Value> &value)
    {
      const std::string signature = GioSignature<std::vector<Value>>::value();
      GVariantType *type = g_variant_type_new(signature.c_str());
      GVariantBuilder builder;
      g_variant_builder_init(&builder, type);
      for (const auto &item: value)
        g_variant_builder_add_value(&builder, GioCodec<Value>::encode(item));
      GVariant *result = g_variant_builder_end(&builder);
      g_variant_type_free(type);
      return result;
    }
  };

  template<typename Value>
  struct GioSignature<std::list<Value>>
  {
    static std::string value()
    {
      return "a" + GioSignature<Value>::value();
    }
  };

  template<typename Value>
  struct GioCodec<std::list<Value>>
  {
    static std::list<Value> decode(GVariant *variant)
    {
      gio_require_type(variant, GioSignature<std::list<Value>>::value());
      std::list<Value> result;
      const gsize count = g_variant_n_children(variant);
      for (gsize index = 0; index < count; ++index)
        {
          GioVariant child(g_variant_get_child_value(variant, index));
          result.push_back(GioCodec<Value>::decode(child.get()));
        }
      return result;
    }

    static GVariant *encode(const std::list<Value> &value)
    {
      const std::string signature = GioSignature<std::list<Value>>::value();
      GVariantType *type = g_variant_type_new(signature.c_str());
      GVariantBuilder builder;
      g_variant_builder_init(&builder, type);
      for (const auto &item: value)
        g_variant_builder_add_value(&builder, GioCodec<Value>::encode(item));
      GVariant *result = g_variant_builder_end(&builder);
      g_variant_type_free(type);
      return result;
    }
  };

  template<typename Key, typename Value>
  struct GioSignature<std::map<Key, Value>>
  {
    static std::string value()
    {
      return "a{" + GioSignature<Key>::value() + GioSignature<Value>::value() + "}";
    }
  };

  template<typename Key, typename Value>
  struct GioCodec<std::map<Key, Value>>
  {
    static std::map<Key, Value> decode(GVariant *variant)
    {
      gio_require_type(variant, GioSignature<std::map<Key, Value>>::value());
      std::map<Key, Value> result;
      const gsize count = g_variant_n_children(variant);
      for (gsize index = 0; index < count; ++index)
        {
          GioVariant entry(g_variant_get_child_value(variant, index));
          GioVariant key(g_variant_get_child_value(entry.get(), 0));
          GioVariant value(g_variant_get_child_value(entry.get(), 1));
          result.emplace(GioCodec<Key>::decode(key.get()), GioCodec<Value>::decode(value.get()));
        }
      return result;
    }

    static GVariant *encode(const std::map<Key, Value> &value)
    {
      const std::string signature = GioSignature<std::map<Key, Value>>::value();
      GVariantType *type = g_variant_type_new(signature.c_str());
      GVariantBuilder builder;
      g_variant_builder_init(&builder, type);
      for (const auto &[key, item]: value)
        {
          g_variant_builder_add_value(
            &builder,
            g_variant_new_dict_entry(GioCodec<Key>::encode(key), GioCodec<Value>::encode(item)));
        }
      GVariant *result = g_variant_builder_end(&builder);
      g_variant_type_free(type);
      return result;
    }
  };

  template<typename T>
  T gio_decode_child(GVariant *tuple, gsize index)
  {
    if (tuple == nullptr || !g_variant_is_of_type(tuple, G_VARIANT_TYPE_TUPLE)
        || index >= g_variant_n_children(tuple))
      {
        throw Error(std::string(error_names::invalid_args), "Missing DBus tuple argument");
      }
    GioVariant child(g_variant_get_child_value(tuple, index));
    return GioCodec<T>::decode(child.get());
  }

  inline UnixFd gio_decode_unix_fd_child(GVariant *tuple,
                                         gsize index,
                                         GDBusMethodInvocation *invocation)
  {
#if defined(G_OS_UNIX)
    const UnixFd handle = gio_decode_child<UnixFd>(tuple, index);
    GDBusMessage *message = g_dbus_method_invocation_get_message(invocation);
    GUnixFDList *fd_list = message != nullptr ? g_dbus_message_get_unix_fd_list(message) : nullptr;
    if (fd_list == nullptr)
      {
        throw Error(std::string(error_names::invalid_args),
                    "D-Bus UNIX_FD argument has no attached file-descriptor list");
      }

    gint length = 0;
    const gint *fds = g_unix_fd_list_peek_fds(fd_list, &length);
    if (handle.value < 0 || handle.value >= length || fds == nullptr)
      {
        throw Error(std::string(error_names::invalid_args), "Invalid D-Bus UNIX_FD handle");
      }
    return UnixFd{fds[handle.value]};
#else
    (void)tuple;
    (void)index;
    (void)invocation;
    throw Error(std::string(error_names::failed),
                "D-Bus UNIX_FD arguments are not supported on this platform");
#endif
  }

  class GioUnixFdList
  {
  public:
    ~GioUnixFdList()
    {
      if (value_ != nullptr)
        g_object_unref(value_);
    }

    GioUnixFdList(const GioUnixFdList &) = delete;
    GioUnixFdList &operator=(const GioUnixFdList &) = delete;
    GioUnixFdList() = default;

    [[nodiscard]] GUnixFDList *get() const noexcept
    {
      return value_;
    }

    GUnixFDList *ensure()
    {
#if defined(G_OS_UNIX)
      if (value_ == nullptr)
        value_ = g_unix_fd_list_new();
      return value_;
#else
      throw Error(std::string(error_names::failed),
                  "D-Bus UNIX_FD arguments are not supported on this platform");
#endif
    }

  private:
    GUnixFDList *value_{nullptr};
  };

  inline GVariant *gio_encode_unix_fd(UnixFd fd, GioUnixFdList &fd_list)
  {
#if defined(G_OS_UNIX)
    if (fd.value < 0)
      {
        throw Error(std::string(error_names::invalid_args),
                    "Cannot send an invalid UNIX file descriptor");
      }
    GError *error = nullptr;
    const int handle = g_unix_fd_list_append(fd_list.ensure(), fd.value, &error);
    if (handle < 0)
      {
        const std::string diagnostic = error != nullptr ? error->message : "Unable to attach UNIX file descriptor";
        if (error != nullptr)
          g_error_free(error);
        throw Error(std::string(error_names::failed), diagnostic);
      }
    return GioCodec<UnixFd>::encode(UnixFd{handle});
#else
    (void)fd;
    (void)fd_list;
    throw Error(std::string(error_names::failed),
                "D-Bus UNIX_FD arguments are not supported on this platform");
#endif
  }

  inline void gio_return_method_value(GDBusMethodInvocation *invocation,
                                      GVariant *parameters,
                                      GUnixFDList *fd_list)
  {
#if defined(G_OS_UNIX)
    if (fd_list != nullptr)
      {
        g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, parameters, fd_list);
        return;
      }
#else
    if (fd_list != nullptr)
      {
        throw Error(std::string(error_names::failed),
                    "D-Bus UNIX_FD arguments are not supported on this platform");
      }
#endif
    g_dbus_method_invocation_return_value(invocation, parameters);
  }
}
