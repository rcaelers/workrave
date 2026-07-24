// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

// TEMPORARY comparison test: compares the DBus introspection XML produced by
// the existing, hand-maintained libs/dbus pipeline (libs/corenext/src/
// workrave-service.xml -> dbusgen.py -> DBusWorkraveNext.cc, unmodified and
// still the only thing the real app links) against the introspection XML
// produced by clang-rpc-gen's new DBus backend, generated from the same
// @rpc.dbus-annotated Core.hh/Break.hh/IConfigurator.hh headers as the gRPC
// output. Neither side is wired into the real app.
//
// This reads the *generated* .cc sources as plain text rather than compiling
// and running them: get_interface_introspect() is `static constexpr
// std::string_view` — a compile-time constant with no dependency on a live
// Core/Break/IConfigurator object — so nothing is gained by linking and
// executing the real glue, and doing so is actively a trap: both dbusgen.py
// and clang-rpc-gen derive C++ class names straight from the DBus interface
// name (e.g. "org.workrave.CoreInterface" -> org_workrave_CoreInterface),
// with no namespacing. That's fine in production (only one generator's
// output is ever linked into any given binary) but means linking both into
// one test binary is a guaranteed duplicate-symbol error — and worse, even
// linking just the NEW side pulls in real Core.cc (to satisfy the stub's
// calls into the real methods), and Core.cc unconditionally calls
// init_DBusWorkraveNext(), which pulls the OLD glue right back in anyway.
// Reading the generated source text sidesteps all of that.
//
// This does NOT cover ui/app/workrave-gui.xml (Menus.hh/GenericDBusApplet.hh)
// — that surface was never annotated with @rpc.dbus in this pass.

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#ifndef OLD_DBUS_WORKRAVE_NEXT_CC
#  error "OLD_DBUS_WORKRAVE_NEXT_CC must be defined to the path of the generated DBusWorkraveNext.cc"
#endif
#ifndef NEW_CORE_DBUS_CC
#  error "NEW_CORE_DBUS_CC must be defined to the path of the generated RpcCoreDBus.cc"
#endif
#ifndef NEW_BREAK_DBUS_CC
#  error "NEW_BREAK_DBUS_CC must be defined to the path of the generated RpcBreakDBus.cc"
#endif
#ifndef NEW_CONFIG_DBUS_CC
#  error "NEW_CONFIG_DBUS_CC must be defined to the path of the generated RpcConfigDBus.cc"
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

  std::string read_file(const std::string &path)
  {
    std::ifstream in(path);
    if (!in)
      {
        ADD_FAILURE() << "Could not open " << path;
        return {};
      }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
  }

  // Unescapes the small subset of C++ string-literal escapes the generators
  // actually emit into these literals (\", \n, \\).
  std::string unescape_cxx_string_literal(const std::string &literal)
  {
    std::string out;
    for (size_t i = 0; i < literal.size(); ++i)
      {
        if (literal[i] == '\\' && i + 1 < literal.size())
          {
            char next = literal[++i];
            out += (next == 'n') ? '\n' : next;
          }
        else
          {
            out += literal[i];
          }
      }
    return out;
  }

  // Finds the `static constexpr std::string_view interface_introspect = ...;`
  // statement (a run of adjacent C++ string literals, one per XML line) whose
  // reconstructed content starts with the given interface's opening tag, and
  // returns that content as real XML text. A single .cc can define more than
  // one such constant (DBusWorkraveNext.cc has three, one per interface).
  std::string extract_introspect_xml(const std::string &source, const std::string &interface_name)
  {
    std::regex block_re(
      R"RX(static constexpr std::string_view interface_introspect\s*=\s*((?:\s*"(?:[^"\\]|\\.)*"\s*)+);)RX");
    std::regex literal_re(R"RX("((?:[^"\\]|\\.)*)")RX");

    for (auto it = std::sregex_iterator(source.begin(), source.end(), block_re); it != std::sregex_iterator(); ++it)
      {
        std::string literals_blob = (*it)[1].str();
        std::string xml;
        for (auto lit = std::sregex_iterator(literals_blob.begin(), literals_blob.end(), literal_re);
             lit != std::sregex_iterator();
             ++lit)
          {
            xml += unescape_cxx_string_literal((*lit)[1].str());
          }
        if (xml.find("<interface name=\"" + interface_name + "\">") != std::string::npos)
          {
            return xml;
          }
      }
    return {};
  }

  // Extracts all top-level <method>/<signal> elements (and their <arg>
  // children) out of a "<interface>...</interface>" fragment. Regex-based
  // rather than a full XML parser: method/signal elements never nest (only
  // self-closed <arg .../> children), and both generators emit a fixed
  // attribute order (type, name, [direction]) — see
  // libs/rpc/tool/src/dbus_gen.rs's render_method_body/render_signal_emit and
  // libs/dbus/data/*.jinja for the old side.
  std::vector<Member> parse_members(const std::string &xml, const std::string &tag)
  {
    std::vector<Member> members;

    // Custom "RX" delimiter, not the default empty one: these patterns embed
    // a literal `)"` themselves (a capture group closing right before an XML
    // attribute's closing quote, e.g. `([^"]*)"`), which would otherwise be
    // read as the raw string's own terminator and truncate it silently.
    std::regex member_re("<" + tag + R"RX(\s+name="([^"]*)"\s*>([\s\S]*?)</)RX" + tag + ">");
    std::regex arg_re(R"RX(<arg\s+type="([^"]*)"\s+name="[^"]*"(?:\s+direction="([^"]*)")?\s*/>)RX");

    for (auto it = std::sregex_iterator(xml.begin(), xml.end(), member_re); it != std::sregex_iterator(); ++it)
      {
        Member member;
        member.name = (*it)[1].str();
        std::string body = (*it)[2].str();

        for (auto arg_it = std::sregex_iterator(body.begin(), body.end(), arg_re); arg_it != std::sregex_iterator();
             ++arg_it)
          {
            Arg arg;
            arg.type = (*arg_it)[1].str();
            // Signal <arg>s carry no direction attribute in this IDL — a
            // signal only ever flows out, so default to that.
            arg.direction = (*arg_it)[2].matched ? (*arg_it)[2].str() : "out";
            member.args.push_back(arg);
          }
        members.push_back(std::move(member));
      }
    return members;
  }

  std::string signature_of(const Member &m)
  {
    std::ostringstream out;
    out << m.name << "(";
    for (size_t i = 0; i < m.args.size(); ++i)
      {
        if (i != 0)
          {
            out << ", ";
          }
        out << m.args[i].direction << ":" << m.args[i].type;
      }
    out << ")";
    return out.str();
  }

  const Member *find_by_name(const std::vector<Member> &members, const std::string &name)
  {
    auto it = std::find_if(members.begin(), members.end(), [&name](const Member &m) { return m.name == name; });
    return it != members.end() ? &*it : nullptr;
  }

  // Checks that every method/signal in the OLD, hand-maintained
  // workrave-service.xml is also present in the NEW clang-rpc-gen output
  // (the new @rpc-annotated surface is known to be a strict superset — see
  // e.g. Core.hh's ForceBreak/SetOperationModeFor, which aren't in the old
  // XML at all). A missing name is a hard test failure: it would mean the
  // new generator silently dropped something the old, real interface
  // exposes. A differing signature for a name present on both sides is
  // logged (not failed) — see the two known, pre-existing divergences this
  // test documents: OLD's BreakInterface narrows int64_t Timer values to a
  // DBus "i" (int32) — dbusgen.py never picked "x" for these methods even
  // though Break::get_elapsed_time() et al. return int64_t, a latent
  // truncation bug that predates this tool entirely — and OLD's
  // ConfigInterface SetString/SetInt/... simply never exposed the
  // ConfigFlags parameter that IConfigurator::set_value() has always taken.
  void compare_interface(const std::string &interface_name, const std::string &old_xml, const std::string &new_xml)
  {
    SCOPED_TRACE(interface_name);
    ASSERT_FALSE(old_xml.empty()) << "Could not find " << interface_name << " in the old generated source";
    ASSERT_FALSE(new_xml.empty()) << "Could not find " << interface_name << " in the new generated source";

    std::vector<Member> old_methods = parse_members(old_xml, "method");
    std::vector<Member> new_methods = parse_members(new_xml, "method");
    std::vector<Member> old_signals = parse_members(old_xml, "signal");
    std::vector<Member> new_signals = parse_members(new_xml, "signal");

    for (const Member &old_m : old_methods)
      {
        const Member *new_m = find_by_name(new_methods, old_m.name);
        EXPECT_NE(new_m, nullptr) << interface_name << ": method '" << old_m.name
                                   << "' is in the old workrave-service.xml pipeline but missing from "
                                   << "clang-rpc-gen's DBus output";
        if (new_m != nullptr && signature_of(old_m) != signature_of(*new_m))
          {
            std::cout << "[ note ] " << interface_name << ": method '" << old_m.name
                      << "' signature differs -- old: " << signature_of(old_m) << "  new: " << signature_of(*new_m)
                      << '\n';
          }
      }

    for (const Member &old_s : old_signals)
      {
        const Member *new_s = find_by_name(new_signals, old_s.name);
        EXPECT_NE(new_s, nullptr) << interface_name << ": signal '" << old_s.name
                                   << "' is in the old workrave-service.xml pipeline but missing from "
                                   << "clang-rpc-gen's DBus output";
        if (new_s != nullptr && signature_of(old_s) != signature_of(*new_s))
          {
            std::cout << "[ note ] " << interface_name << ": signal '" << old_s.name
                      << "' signature differs -- old: " << signature_of(old_s) << "  new: " << signature_of(*new_s)
                      << '\n';
          }
      }

    std::cout << "[ info ] " << interface_name << ": old has " << old_methods.size() << " method(s)/"
              << old_signals.size() << " signal(s); new has " << new_methods.size() << " method(s)/"
              << new_signals.size() << " signal(s)\n";
  }
} // namespace

TEST(DBusComparison, core_break_config_introspection_is_a_superset_of_the_old_xml)
{
  std::string old_source = read_file(OLD_DBUS_WORKRAVE_NEXT_CC);
  std::string new_core_source = read_file(NEW_CORE_DBUS_CC);
  std::string new_break_source = read_file(NEW_BREAK_DBUS_CC);
  std::string new_config_source = read_file(NEW_CONFIG_DBUS_CC);

  compare_interface("org.workrave.CoreInterface",
                     extract_introspect_xml(old_source, "org.workrave.CoreInterface"),
                     extract_introspect_xml(new_core_source, "org.workrave.CoreInterface"));
  compare_interface("org.workrave.BreakInterface",
                     extract_introspect_xml(old_source, "org.workrave.BreakInterface"),
                     extract_introspect_xml(new_break_source, "org.workrave.BreakInterface"));
  compare_interface("org.workrave.ConfigInterface",
                     extract_introspect_xml(old_source, "org.workrave.ConfigInterface"),
                     extract_introspect_xml(new_config_source, "org.workrave.ConfigInterface"));
}
