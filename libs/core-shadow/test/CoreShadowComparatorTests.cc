#include <memory>
#include <sstream>
#include <string>

#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

#include "CoreShadowProxy.hh"

namespace workrave::core_shadow
{
  namespace
  {
    auto event(Backend backend, int64_t tick, std::string detail = {}) -> EventObservation
    {
      return EventObservation{.backend = backend,
                              .tick = tick,
                              .source = "break-signal",
                              .break_id = BREAK_ID_MICRO_BREAK,
                              .name = "ShowPrelude",
                              .detail = std::move(detail)};
    }

    auto snapshot(Backend backend, int64_t tick, int64_t elapsed, int64_t idle = 0) -> TimerSnapshot
    {
      return TimerSnapshot{.backend = backend,
                           .tick = tick,
                           .break_id = BREAK_ID_MICRO_BREAK,
                           .elapsed = elapsed,
                           .idle = idle,
                           .enabled = true,
                           .running = true,
                           .user_active = true};
    }

    auto occurrences(const std::string &text, const std::string &needle) -> std::size_t
    {
      std::size_t result = 0;
      for (auto position = text.find(needle); position != std::string::npos;
           position = text.find(needle, position + needle.size()))
        {
          result++;
        }
      return result;
    }
  } // namespace

  class CoreShadowComparatorTest : public ::testing::Test
  {
  protected:
    void SetUp() override
    {
      previous_logger = spdlog::default_logger();
      sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(log);
      auto logger = std::make_shared<spdlog::logger>("core-shadow-comparator-test", sink);
      logger->set_level(spdlog::level::debug);
      spdlog::set_default_logger(std::move(logger));
    }

    void TearDown() override
    {
      spdlog::set_default_logger(previous_logger);
    }

    std::ostringstream log;
    std::shared_ptr<spdlog::sinks::ostream_sink_mt> sink;
    std::shared_ptr<spdlog::logger> previous_logger;
  };

  TEST_F(CoreShadowComparatorTest, EventsMatchAcrossNearbyHeartbeats)
  {
    CoreShadowComparator comparator;
    comparator.record_live_event(event(Backend::CoreNext, 10));
    comparator.compare(10, {}, {});
    comparator.compare(12, {}, ObservationBatch{.events = {event(Backend::Core, 12)}});

    const auto html = comparator.get_event_debug_state_html();
    EXPECT_NE(html.find("matched with skew"), std::string::npos);
    EXPECT_NE(html.find("<td align=\"right\">-2</td>"), std::string::npos);
    EXPECT_EQ(log.str().find("missing from"), std::string::npos);
  }

  TEST_F(CoreShadowComparatorTest, EventsReturnedByCommandsParticipateInMatching)
  {
    CoreShadowComparator comparator;
    comparator.record_shadow_events(ObservationBatch{.events = {event(Backend::Core, 20)}});
    comparator.record_live_event(event(Backend::CoreNext, 22));
    comparator.compare(22, {}, {});

    const auto html = comparator.get_event_debug_state_html();
    EXPECT_NE(html.find("matched with skew"), std::string::npos);
    EXPECT_EQ(log.str().find("missing from"), std::string::npos);
  }

  TEST_F(CoreShadowComparatorTest, MissingEventIsReportedOnceAfterGracePeriodWithCorrectBackend)
  {
    CoreShadowComparator comparator;
    comparator.compare(10, {}, ObservationBatch{.events = {event(Backend::Core, 10)}});
    comparator.compare(13, {}, {});
    EXPECT_EQ(log.str().find("missing from"), std::string::npos);

    comparator.compare(14, {}, {});
    comparator.compare(15, {}, {});

    EXPECT_EQ(occurrences(log.str(), "event missing from corenext"), 1U);
    EXPECT_NE(log.str().find("observed_on=core tick=10"), std::string::npos);
    EXPECT_NE(comparator.get_event_debug_state_html().find("missing from corenext"), std::string::npos);
  }

  TEST_F(CoreShadowComparatorTest, EventDetailDifferenceIsKeptAsDiagnosticEvidence)
  {
    CoreShadowComparator comparator;
    comparator.record_live_event(event(Backend::CoreNext, 30, "next-detail"));
    comparator.compare(30, {}, ObservationBatch{.events = {event(Backend::Core, 30, "core-detail")}});

    const auto html = comparator.get_event_debug_state_html();
    EXPECT_NE(html.find("matched; detail differs"), std::string::npos);
    EXPECT_NE(html.find("core-detail"), std::string::npos);
    EXPECT_NE(html.find("next-detail"), std::string::npos);
    EXPECT_EQ(occurrences(log.str(), "event detail differs"), 1U);
  }

  TEST_F(CoreShadowComparatorTest, StableTimerDeviationDoesNotLogEveryHeartbeat)
  {
    CoreShadowComparator comparator;
    comparator.compare(1, {snapshot(Backend::CoreNext, 1, 12)}, ObservationBatch{.snapshots = {snapshot(Backend::Core, 1, 10)}});
    comparator.compare(2, {snapshot(Backend::CoreNext, 2, 13)}, ObservationBatch{.snapshots = {snapshot(Backend::Core, 2, 11)}});
    comparator.compare(3, {snapshot(Backend::CoreNext, 3, 15)}, ObservationBatch{.snapshots = {snapshot(Backend::Core, 3, 11)}});
    comparator.compare(4, {snapshot(Backend::CoreNext, 4, 12)}, ObservationBatch{.snapshots = {snapshot(Backend::Core, 4, 11)}});

    EXPECT_EQ(occurrences(log.str(), "timer deviation started"), 1U);
    EXPECT_EQ(occurrences(log.str(), "timer deviation worsened"), 1U);
    EXPECT_EQ(occurrences(log.str(), "timer deviation resolved"), 1U);
  }

  TEST_F(CoreShadowComparatorTest, StableStateDeviationLogsOnlyEpisodeChanges)
  {
    CoreShadowComparator comparator;
    const auto core = snapshot(Backend::Core, 1, 10);
    auto next = snapshot(Backend::CoreNext, 1, 10);
    next.enabled = false;
    comparator.compare(1, {next}, ObservationBatch{.snapshots = {core}});
    comparator.compare(2, {next}, ObservationBatch{.snapshots = {core}});
    next.running = false;
    comparator.compare(3, {next}, ObservationBatch{.snapshots = {core}});
    next.enabled = true;
    comparator.compare(4, {next}, ObservationBatch{.snapshots = {core}});
    next.running = true;
    comparator.compare(5, {next}, ObservationBatch{.snapshots = {core}});

    EXPECT_EQ(occurrences(log.str(), "state deviation started"), 1U);
    EXPECT_EQ(occurrences(log.str(), "state deviation worsened"), 1U);
    EXPECT_EQ(occurrences(log.str(), "state deviation resolved"), 1U);
  }

  TEST_F(CoreShadowComparatorTest, StableActivityDeviationLogsOnlyStartAndResolution)
  {
    CoreShadowComparator comparator;
    const auto core = snapshot(Backend::Core, 1, 10);
    auto next = snapshot(Backend::CoreNext, 1, 10);
    next.user_active = false;
    comparator.compare(1, {next}, ObservationBatch{.snapshots = {core}});
    comparator.compare(2, {next}, ObservationBatch{.snapshots = {core}});
    next.user_active = true;
    comparator.compare(3, {next}, ObservationBatch{.snapshots = {core}});

    EXPECT_EQ(occurrences(log.str(), "activity deviation started"), 1U);
    EXPECT_EQ(occurrences(log.str(), "activity deviation resolved"), 1U);
  }
} // namespace workrave::core_shadow
