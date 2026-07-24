#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "CoreShadowTypes.hh"

#include "config/ConfiguratorFactory.hh"
#include "core/CoreConfig.hh"
#include "core/IApp.hh"
#include "core/IBreak.hh"
#include "core/ICore.hh"
#include "core/ICoreEventListener.hh"
#include "utils/Enum.hh"

using namespace workrave;
using namespace workrave::config;
using namespace workrave::core_shadow;

namespace
{
  auto
  to_bool(const std::string &value) -> bool
  {
    return value == "1" || value == "true";
  }

  auto
  to_break_hint(const std::string &value) -> workrave::utils::Flags<BreakHint>
  {
    workrave::utils::Flags<BreakHint> flags;
    flags.set(static_cast<std::underlying_type_t<BreakHint>>(std::stoi(value)));
    return flags;
  }

  void
  emit_event(int64_t tick, const std::string &source, int break_id, const std::string &name, const std::string &detail = {})
  {
    std::cout << serialize_event(EventObservation{.backend = Backend::Core,
                                                  .tick = tick,
                                                  .source = source,
                                                  .break_id = break_id,
                                                  .name = name,
                                                  .detail = detail})
              << std::endl;
  }

  class ShadowApp
    : public IApp
    , public ICoreEventListener
  {
  public:
    void set_tick(int64_t new_tick)
    {
      tick = new_tick;
    }

    void create_prelude_window(BreakId break_id) override
    {
      emit_event(tick, "app-callback", break_id, "create_prelude_window");
    }

    void create_break_window(BreakId break_id, workrave::utils::Flags<BreakHint> break_hint) override
    {
      emit_event(tick, "app-callback", break_id, "create_break_window", std::to_string(break_hint.get()));
    }

    void hide_break_window() override
    {
      emit_event(tick, "app-callback", BREAK_ID_NONE, "hide_break_window");
    }

    void show_break_window() override
    {
      emit_event(tick, "app-callback", BREAK_ID_NONE, "show_break_window");
    }

    void refresh_break_window() override
    {
      emit_event(tick, "app-callback", BREAK_ID_NONE, "refresh_break_window");
    }

    void set_break_progress(int value, int max_value) override
    {
      emit_event(tick, "app-callback", BREAK_ID_NONE, "set_break_progress", std::to_string(value) + "/" + std::to_string(max_value));
    }

    void set_prelude_stage(PreludeStage stage) override
    {
      emit_event(tick, "app-callback", BREAK_ID_NONE, "set_prelude_stage", std::string{workrave::utils::enum_to_string(stage)});
    }

    void set_prelude_progress_text(PreludeProgressText text) override
    {
      emit_event(tick, "app-callback", BREAK_ID_NONE, "set_prelude_progress_text", std::string{workrave::utils::enum_to_string(text)});
    }

    void core_event_notify(const CoreEvent event) override
    {
      emit_event(tick, "core-event", BREAK_ID_NONE, std::string{workrave::utils::enum_to_string(event)});
    }

  private:
    int64_t tick{0};
  };

  class Helper
  {
  public:
    Helper()
    {
      auto fmt = ConfigFileFormat::Native;
      configurator = ConfiguratorFactory::create(fmt);
      if (!configurator)
        {
          configurator = ConfiguratorFactory::create(ConfigFileFormat::Ini);
        }

      CoreConfig::init(configurator);
      core = CoreFactory::create(configurator);
      core->init(0, nullptr, &app, "");
      core->set_core_events_listener(&app);

      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          auto break_id = BreakId(i);
          core->get_break(break_id)->signal_break_event().connect([this, break_id](BreakEvent event) {
            emit_event(tick, "break-signal", break_id, std::string{workrave::utils::enum_to_string(event)});
          });
        }
    }

    auto handle(const std::string &line) -> bool
    {
      auto fields = split_line(line);
      if (fields.empty())
        {
          return true;
        }

      const auto &command = fields[0];
      try
        {
          if (command == "quit")
            {
              std::cout << "done" << std::endl;
              std::cout.flush();
              std::_Exit(EXIT_SUCCESS);
            }
          if (command == "tick" && fields.size() >= 2)
            {
              tick = std::stoll(fields[1]);
              app.set_tick(tick);
            }
          else if (command == "heartbeat" && fields.size() >= 2)
            {
              tick = std::stoll(fields[1]);
              app.set_tick(tick);
              core->heartbeat();
              write_snapshots();
            }
          else if (command == "force_break" && fields.size() >= 3)
            {
              core->force_break(static_cast<BreakId>(std::stoi(fields[1])), to_break_hint(fields[2]));
            }
          else if (command == "set_operation_mode" && fields.size() >= 2)
            {
              core->set_operation_mode(static_cast<OperationMode>(std::stoi(fields[1])));
            }
          else if (command == "set_operation_mode_for" && fields.size() >= 3)
            {
              core->set_operation_mode_for(static_cast<OperationMode>(std::stoi(fields[1])), std::chrono::minutes(std::stoi(fields[2])));
            }
          else if (command == "set_operation_mode_override" && fields.size() >= 3)
            {
              core->set_operation_mode_override(static_cast<OperationMode>(std::stoi(fields[1])), fields[2]);
            }
          else if (command == "remove_operation_mode_override" && fields.size() >= 2)
            {
              core->remove_operation_mode_override(fields[1]);
            }
          else if (command == "set_usage_mode" && fields.size() >= 2)
            {
              core->set_usage_mode(static_cast<UsageMode>(std::stoi(fields[1])));
            }
          else if (command == "set_powersave" && fields.size() >= 2)
            {
              core->set_powersave(to_bool(fields[1]));
            }
          else if (command == "set_insist_policy" && fields.size() >= 2)
            {
              core->set_insist_policy(static_cast<InsistPolicy>(std::stoi(fields[1])));
            }
          else if (command == "force_idle")
            {
              core->force_idle();
            }
          else if (command == "postpone" && fields.size() >= 2)
            {
              core->get_break(static_cast<BreakId>(std::stoi(fields[1])))->postpone_break();
            }
          else if (command == "skip" && fields.size() >= 2)
            {
              core->get_break(static_cast<BreakId>(std::stoi(fields[1])))->skip_break();
            }
          else if (command == "snapshot")
            {
              write_snapshots();
            }
          else
            {
              std::cout << "error\tunknown-command\t" << escape_field(line) << std::endl;
            }
        }
      catch (const std::exception &e)
        {
          std::cout << "error\texception\t" << escape_field(e.what()) << std::endl;
        }

      std::cout << "done" << std::endl;
      return true;
    }

  private:
    void write_snapshots()
    {
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          auto break_id = BreakId(i);
          auto *b = core->get_break(break_id);
          std::cout << serialize_snapshot(TimerSnapshot{.backend = Backend::Core,
                                                        .tick = tick,
                                                        .break_id = i,
                                                        .elapsed = b->get_elapsed_time(),
                                                        .idle = b->get_elapsed_idle_time(),
                                                        .limit = b->get_limit(),
                                                        .auto_reset = b->get_auto_reset(),
                                                        .enabled = b->is_enabled(),
                                                        .running = b->is_running(),
                                                        .taking = b->is_taking(),
                                                        .active = b->is_active(),
                                                        .user_active = core->is_user_active()})
                    << std::endl;
        }
    }

  private:
    IConfigurator::Ptr configurator;
    ICore::Ptr core;
    ShadowApp app;
    int64_t tick{0};
  };
} // namespace

int
main()
{
  Helper helper;
  std::string line;
  while (std::getline(std::cin, line))
    {
      if (!helper.handle(line))
        {
          break;
        }
    }
  return 0;
}
