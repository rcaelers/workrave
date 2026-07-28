// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rpc/dbus/QtServer.hh"

#include <map>
#include <memory>
#include <stdexcept>
#include <utility>

#include <QtCore/QString>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusServiceWatcher>
#include <QtDBus/QDBusVirtualObject>

#include "rpc/dbus/Error.hh"
#include "rpc/dbus/QtInterface.hh"

namespace workrave::rpc::dbus
{
  class QtServer::State : public std::enable_shared_from_this<QtServer::State>
  {
  public:
    class Object final : public QDBusVirtualObject
    {
    public:
      Object() = default;

      QString introspect(const QString &path) const override
      {
        (void)path;
        std::string xml;
        for (const auto &[name, interface]: interfaces_)
          {
            (void)name;
            xml.append(interface->introspection());
          }
        return QString::fromStdString(xml);
      }

      bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override
      {
        const auto interface_name = message.interface().toStdString();
        const auto found = interfaces_.find(interface_name);
        if (found == interfaces_.end())
          {
            return send_error(connection,
                              message,
                              error_names::unknown_method,
                              "No registered interface named " + interface_name);
          }

        try
          {
            if (!found->second->dispatch(message, connection))
              {
                return send_error(connection,
                                  message,
                                  error_names::unknown_method,
                                  "Unknown method " + message.member().toStdString());
              }
            return true;
          }
        catch (const Error &error)
          {
            return send_error(connection, message, error.name(), error.what());
          }
        catch (const std::exception &error)
          {
            return send_error(connection, message, error_names::failed, error.what());
          }
      }

      void add(std::shared_ptr<QtInterface> interface)
      {
        const std::string name(interface->name());
        if (interfaces_.contains(name))
          {
            throw Error(std::string(error_names::failed), "DBus interface is already registered: " + name);
          }
        interfaces_.emplace(name, std::move(interface));
      }

      void remove(std::string_view name)
      {
        interfaces_.erase(std::string(name));
      }

      [[nodiscard]] bool empty() const noexcept
      {
        return interfaces_.empty();
      }

    private:
      static bool send_error(const QDBusConnection &connection,
                             const QDBusMessage &request,
                             std::string_view name,
                             std::string_view message)
      {
        const auto reply = request.createErrorReply(QString::fromUtf8(name.data(), static_cast<qsizetype>(name.size())),
                                                    QString::fromUtf8(message.data(), static_cast<qsizetype>(message.size())));
        connection.send(reply);
        return true;
      }

      std::map<std::string, std::shared_ptr<QtInterface>, std::less<>> interfaces_;
    };

    explicit State(QDBusConnection connection)
      : connection_(std::move(connection))
    {
    }

    ~State()
    {
      for (const auto &[path, object]: objects_)
        {
          (void)object;
          connection_.unregisterObject(QString::fromStdString(path));
        }
    }

    Registration register_interface(std::string path, std::shared_ptr<QtInterface> interface)
    {
      if (!interface)
        {
          throw std::invalid_argument("Cannot register a null DBus interface");
        }

      auto object = objects_.find(path);
      if (object == objects_.end())
        {
          auto router = std::make_unique<Object>();
          if (!connection_.registerVirtualObject(QString::fromStdString(path), router.get()))
            {
              throw Error(std::string(error_names::failed), "Unable to register DBus object path " + path);
            }
          object = objects_.emplace(path, std::move(router)).first;
        }

      const std::string interface_name(interface->name());
      object->second->add(std::move(interface));
      std::weak_ptr<State> weak_state = shared_from_this();
      return Registration([weak_state, path = std::move(path), interface_name] {
        if (const auto state = weak_state.lock())
          {
            state->unregister_interface(path, interface_name);
          }
      });
    }

    void unregister_interface(std::string_view path, std::string_view interface_name) noexcept
    {
      const auto object = objects_.find(path);
      if (object == objects_.end())
        {
          return;
        }

      object->second->remove(interface_name);
      if (object->second->empty())
        {
          connection_.unregisterObject(QString::fromUtf8(path.data(), static_cast<qsizetype>(path.size())));
          objects_.erase(object);
        }
    }

    QDBusConnection connection_;
    std::map<std::string, std::unique_ptr<Object>, std::less<>> objects_;
  };

  QtServer::QtServer(QDBusConnection connection)
    : state_(std::make_shared<State>(std::move(connection)))
  {
  }

  QtServer::~QtServer() = default;

  std::unique_ptr<QtServer> QtServer::session()
  {
    return std::make_unique<QtServer>(QDBusConnection::sessionBus());
  }

  bool QtServer::is_available() const noexcept
  {
    return state_->connection_.isConnected();
  }

  void QtServer::request_name(std::string_view name)
  {
    const QString service = QString::fromUtf8(name.data(), static_cast<qsizetype>(name.size()));
    if (!state_->connection_.registerService(service))
      {
        throw Error(std::string(error_names::failed), "Unable to acquire DBus service name " + std::string(name));
      }
  }

  Registration QtServer::register_interface(std::string path, std::shared_ptr<QtInterface> interface)
  {
    return state_->register_interface(std::move(path), std::move(interface));
  }

  Registration QtServer::watch_name(std::string name, std::function<void(bool)> callback)
  {
    auto watcher = std::make_shared<QDBusServiceWatcher>(QString::fromStdString(name),
                                                        state_->connection_,
                                                        QDBusServiceWatcher::WatchForOwnerChange);
    QObject::connect(watcher.get(),
                     &QDBusServiceWatcher::serviceOwnerChanged,
                     [callback = std::move(callback)](const QString &, const QString &, const QString &new_owner) {
                       callback(!new_owner.isEmpty());
                     });
    return Registration([watcher = std::move(watcher)] {});
  }

  void QtServer::emit_signal(std::string_view path,
                             std::string_view interface_name,
                             std::string_view signal_name,
                             const QVariantList &arguments)
  {
    auto signal = QDBusMessage::createSignal(QString::fromUtf8(path.data(), static_cast<qsizetype>(path.size())),
                                             QString::fromUtf8(interface_name.data(),
                                                               static_cast<qsizetype>(interface_name.size())),
                                             QString::fromUtf8(signal_name.data(), static_cast<qsizetype>(signal_name.size())));
    signal.setArguments(arguments);
    if (!state_->connection_.send(signal))
      {
        throw Error(std::string(error_names::failed), "Unable to send DBus signal " + std::string(signal_name));
      }
  }
}
