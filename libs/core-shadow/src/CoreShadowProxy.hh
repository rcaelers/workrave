#ifndef WORKRAVE_CORE_SHADOW_PROXY_HH
#define WORKRAVE_CORE_SHADOW_PROXY_HH

#include <array>
#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include <boost/signals2.hpp>

#include "CoreShadowClient.hh"
#include "CoreShadowTypes.hh"
#include "config/IConfigurator.hh"
#include "core-shadow/CoreShadow.hh"
#include "core/IApp.hh"
#include "core/IBreak.hh"
#include "core/ICore.hh"
#include "utils/Signals.hh"

namespace workrave::core_shadow
{
  class CoreShadowComparator
  {
  public:
    void record_live_event(EventObservation event);
    void record_shadow_events(const ObservationBatch &shadow_batch);
    void compare(int64_t tick, const std::vector<TimerSnapshot> &live_snapshots, const ObservationBatch &shadow_batch);
    [[nodiscard]] std::string get_event_debug_state_html() const;

  private:
    struct EventHistory
    {
      std::string source;
      int break_id{BREAK_ID_NONE};
      std::string name;
      std::string core_detail;
      std::string corenext_detail;
      std::optional<int64_t> core_tick;
      std::optional<int64_t> corenext_tick;
    };

    void record_event_history(const EventObservation &event);

    std::vector<EventObservation> live_events;
    std::map<std::string, EventHistory> event_history;
    int64_t max_timer_delta{0};
    int64_t last_activity_warning_tick{-1};
  };

  class RecordingApp : public workrave::IApp
  {
  public:
    RecordingApp(workrave::IApp *delegate, CoreShadowComparator &comparator, int64_t &tick);

    void create_prelude_window(BreakId break_id) override;
    void create_break_window(BreakId break_id, workrave::utils::Flags<BreakHint> break_hint) override;
    void hide_break_window() override;
    void show_break_window() override;
    void refresh_break_window() override;
    void set_break_progress(int value, int max_value) override;
    void set_prelude_stage(PreludeStage stage) override;
    void set_prelude_progress_text(PreludeProgressText text) override;

  private:
    void record(const std::string &name, int break_id = BREAK_ID_NONE, const std::string &detail = {});

  private:
    workrave::IApp *delegate{nullptr};
    CoreShadowComparator &comparator;
    int64_t &tick;
  };

  class CoreShadowProxy;

  class BreakShadowProxy : public workrave::IBreak
  {
  public:
    BreakShadowProxy(workrave::BreakId id, workrave::IBreak::Ptr live_break, CoreShadowProxy &core);

    boost::signals2::signal<void(workrave::BreakEvent)> &signal_break_event() override;

    [[nodiscard]] std::string get_name() const override;
    [[nodiscard]] bool is_enabled() const override;
    [[nodiscard]] bool is_running() const override;
    [[nodiscard]] bool is_taking() const override;
    [[nodiscard]] bool is_max_preludes_reached() const override;
    [[nodiscard]] bool is_active() const override;
    [[nodiscard]] int64_t get_elapsed_time() const override;
    [[nodiscard]] int64_t get_elapsed_idle_time() const override;
    [[nodiscard]] int64_t get_auto_reset() const override;
    [[nodiscard]] bool is_auto_reset_enabled() const override;
    [[nodiscard]] int64_t get_limit() const override;
    [[nodiscard]] bool is_limit_enabled() const override;
    [[nodiscard]] int64_t get_total_overdue_time() const override;

    void postpone_break() override;
    void skip_break() override;

    [[nodiscard]] auto snapshot(int64_t tick, bool user_active) const -> TimerSnapshot;

  private:
    workrave::BreakId id;
    workrave::IBreak::Ptr live_break;
    CoreShadowProxy &core;
    boost::signals2::signal<void(workrave::BreakEvent)> break_event_signal;
    boost::signals2::scoped_connection live_break_connection;
  };

  class CoreShadowProxy
    : public workrave::ICore
    , public ICoreShadowDebug
  {
  public:
    CoreShadowProxy(workrave::ICore::Ptr live_core, workrave::config::IConfigurator::Ptr configurator);
    ~CoreShadowProxy() override;

    boost::signals2::signal<void(workrave::OperationMode)> &signal_operation_mode_changed() override;
    boost::signals2::signal<void(workrave::UsageMode)> &signal_usage_mode_changed() override;

    void init(workrave::IApp *app, const char *display) override;
    void heartbeat() override;
    void force_break(BreakId id, workrave::utils::Flags<BreakHint> break_hint) override;
    [[nodiscard]] IBreak::Ptr get_break(BreakId id) const override;
    [[nodiscard]] IStatistics::Ptr get_statistics() const override;
    [[nodiscard]] bool is_user_active() const override;
    [[nodiscard]] bool is_taking() const override;
    [[nodiscard]] OperationMode get_active_operation_mode() override;
    [[nodiscard]] OperationMode get_regular_operation_mode() override;
    void set_operation_mode(OperationMode mode) override;
    void set_operation_mode_for(OperationMode mode, std::chrono::minutes duration) override;
    void set_operation_mode_override(OperationMode mode, const std::string &id) override;
    void remove_operation_mode_override(const std::string &id) override;
    [[nodiscard]] bool is_operation_mode_an_override() override;
    [[nodiscard]] UsageMode get_usage_mode() override;
    void set_usage_mode(UsageMode mode) override;
    void set_powersave(bool down) override;
    void set_insist_policy(InsistPolicy p) override;
    void force_idle() override;
    [[nodiscard]] ICoreHooks::Ptr get_hooks() const override;
    [[nodiscard]] std::shared_ptr<workrave::dbus::IDBus> get_dbus() const override;
    [[nodiscard]] std::string get_shadow_debug_state_html() const override;

    void mirror_break_command(const std::string &command, workrave::BreakId id);
    void record_live_break_event(workrave::BreakId id, workrave::BreakEvent event);

  private:
    void shadow_command(const std::string &command);
    [[nodiscard]] auto live_snapshots() const -> std::vector<TimerSnapshot>;

  private:
    workrave::ICore::Ptr live_core;
    workrave::config::IConfigurator::Ptr configurator;
    std::unique_ptr<RecordingApp> recording_app;
    std::array<IBreak::Ptr, BREAK_ID_SIZEOF> breaks;
    CoreShadowClient shadow_client;
    CoreShadowComparator comparator;
    int64_t tick{0};
    bool shadow_available{false};
    std::vector<TimerSnapshot> last_live_snapshots;
    std::vector<TimerSnapshot> last_shadow_snapshots;
    int64_t last_shadow_tick{0};
  };
} // namespace workrave::core_shadow

#endif // WORKRAVE_CORE_SHADOW_PROXY_HH
