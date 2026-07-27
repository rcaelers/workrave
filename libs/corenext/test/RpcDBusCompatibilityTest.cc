// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "RpcDBusNames.hh"

#ifndef NEW_CORE_DBUS_CC
#  error "NEW_CORE_DBUS_CC must name the generated Core binding source"
#endif
#ifndef NEW_BREAK_DBUS_CC
#  error "NEW_BREAK_DBUS_CC must name the generated Break binding source"
#endif
#ifndef NEW_CONFIG_DBUS_CC
#  error "NEW_CONFIG_DBUS_CC must name the generated Config binding source"
#endif

namespace
{
  struct Arg
  {
    std::string direction;
    std::string type;
  };

  struct Member
  {
    std::string name;
    std::vector<Arg> args;
  };

  struct Contract
  {
    std::string interface_name;
    std::vector<std::string> methods;
    std::vector<std::string> signals;
  };

  std::string read_file(const std::string &path)
  {
    std::ifstream in(path);
    EXPECT_TRUE(in) << "Could not open " << path;
    std::ostringstream text;
    text << in.rdbuf();
    return text.str();
  }

  std::string unescape_cxx_string_literal(const std::string &literal)
  {
    std::string out;
    for (size_t i = 0; i < literal.size(); ++i)
      {
        if (literal[i] == '\\' && i + 1 < literal.size())
          {
            const char next = literal[++i];
            out += next == 'n' ? '\n' : next;
          }
        else
          {
            out += literal[i];
          }
      }
    return out;
  }

  std::string extract_introspection(const std::string &source, const std::string &interface_name)
  {
    const std::regex block_re(
      R"RX(static constexpr std::string_view interface_introspect\s*=\s*((?:\s*"(?:[^"\\]|\\.)*"\s*)+);)RX");
    const std::regex literal_re(R"RX("((?:[^"\\]|\\.)*)")RX");

    for (auto block = std::sregex_iterator(source.begin(), source.end(), block_re); block != std::sregex_iterator(); ++block)
      {
        std::string xml;
        const std::string literals = (*block)[1].str();
        for (auto literal = std::sregex_iterator(literals.begin(), literals.end(), literal_re); literal != std::sregex_iterator();
             ++literal)
          {
            xml += unescape_cxx_string_literal((*literal)[1].str());
          }
        if (xml.find("<interface name=\"" + interface_name + "\">") != std::string::npos)
          {
            return xml;
          }
      }
    return {};
  }

  std::vector<Member> parse_members(const std::string &xml, const std::string &tag)
  {
    std::vector<Member> members;
    const std::regex member_re("<" + tag + R"RX(\s+name="([^"]*)"\s*>([\s\S]*?)</)RX" + tag + ">");
    const std::regex arg_re(R"RX(<arg\s+type="([^"]*)"\s+name="[^"]*"(?:\s+direction="([^"]*)")?\s*/>)RX");

    for (auto match = std::sregex_iterator(xml.begin(), xml.end(), member_re); match != std::sregex_iterator(); ++match)
      {
        Member member{.name = (*match)[1].str()};
        const std::string body = (*match)[2].str();
        for (auto arg = std::sregex_iterator(body.begin(), body.end(), arg_re); arg != std::sregex_iterator(); ++arg)
          {
            member.args.push_back({.direction = (*arg)[2].matched ? (*arg)[2].str() : "out", .type = (*arg)[1].str()});
          }
        members.push_back(std::move(member));
      }
    return members;
  }

  std::string signature(const Member &member)
  {
    std::ostringstream text;
    text << member.name << '(';
    for (size_t i = 0; i < member.args.size(); ++i)
      {
        if (i != 0)
          {
            text << ',';
          }
        text << member.args[i].direction << ':' << member.args[i].type;
      }
    text << ')';
    return text.str();
  }

  std::vector<std::string> signatures(const std::vector<Member> &members)
  {
    std::vector<std::string> result;
    std::transform(members.begin(), members.end(), std::back_inserter(result), signature);
    return result;
  }

  void expect_contract(const std::string &source, const Contract &contract)
  {
    const std::string xml = extract_introspection(source, contract.interface_name);
    ASSERT_FALSE(xml.empty()) << contract.interface_name;
    EXPECT_EQ(signatures(parse_members(xml, "method")), contract.methods) << contract.interface_name;
    EXPECT_EQ(signatures(parse_members(xml, "signal")), contract.signals) << contract.interface_name;
  }

  const std::vector<Contract> contracts{
    {.interface_name = "org.workrave.CoreInterface",
     .methods = {"SetOperationMode(in:s)",
                 "GetOperationMode(out:s)",
                 "SetUsageMode(in:s)",
                 "GetUsageMode(out:s)",
                 "ReportActivity(in:s,in:b)",
                 "IsActive(out:b)"},
     .signals = {"OperationModeChanged(out:s)", "UsageModeChanged(out:s)"}},
    {.interface_name = "org.workrave.BreakInterface",
     .methods = {"IsTimerRunning(out:b)",
                 "GetTimerIdle(out:i)",
                 "GetTimerElapsed(out:i)",
                 "GetTimerRemaining(out:i)",
                 "GetTimerOverdue(out:i)",
                 "PostponeBreak()",
                 "SkipBreak()",
                 "GetBreakState(out:s)"},
     .signals = {"BreakStateChanged(out:s)", "BreakEvent(out:s)"}},
    {.interface_name = "org.workrave.ConfigInterface",
     .methods = {"SetString(in:s,in:s)",
                 "SetInt(in:s,in:i)",
                 "SetInt64(in:s,in:x)",
                 "SetBool(in:s,in:b)",
                 "SetDouble(in:s,in:d)",
                 "GetString(in:s,out:b,out:s)",
                 "GetInt(in:s,out:i,out:b)",
                 "GetBool(in:s,out:b,out:b)",
                 "GetDouble(in:s,out:d,out:b)"},
     .signals = {}},
  };
} // namespace

TEST(RpcDBusCompatibility, generated_interfaces_match_the_legacy_wire_contract_exactly)
{
  const std::vector<std::string> generated_sources{read_file(NEW_CORE_DBUS_CC),
                                                   read_file(NEW_BREAK_DBUS_CC),
                                                   read_file(NEW_CONFIG_DBUS_CC)};
  ASSERT_EQ(generated_sources.size(), contracts.size());
  for (size_t i = 0; i < contracts.size(); ++i)
    {
      expect_contract(generated_sources[i], contracts[i]);
    }

#ifdef OLD_DBUS_CC
  const std::string legacy_source = read_file(OLD_DBUS_CC);
  for (const auto &contract: contracts)
    {
      expect_contract(legacy_source, contract);
    }
#endif
}

TEST(RpcDBusCompatibility, naming_is_legacy_when_alone_and_collision_free_in_dual_mode)
{
  constexpr RpcDBusNames standalone = RpcDBusNames::select(false);
  EXPECT_EQ(standalone.service, "org.workrave.Workrave");
  EXPECT_EQ(standalone.root_path, "/org/workrave/Workrave");

  constexpr RpcDBusNames dual = RpcDBusNames::select(true);
  EXPECT_EQ(dual.service, "org.workrave.Workrave.Rpc");
  EXPECT_EQ(dual.root_path, "/org/workrave/Workrave/Rpc");
}
