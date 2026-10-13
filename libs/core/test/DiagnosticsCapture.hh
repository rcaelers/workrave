// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef WORKRAVE_DIAGNOSTICS_CAPTURE_HH
#define WORKRAVE_DIAGNOSTICS_CAPTURE_HH

#include <algorithm>
#include <string>
#include <vector>
#include "utils/Diagnostics.hh"

// Capture the public monitoring stream, including its initial snapshot.
class DiagnosticsCapture : public DiagnosticsSink
{
public:
  DiagnosticsCapture()
  {
    Diagnostics::instance().enable(this);
  }

  ~DiagnosticsCapture() override
  {
    Diagnostics::instance().disable();
  }

  void diagnostics_log(const std::string &line) override
  {
    lines.push_back(line);
  }

  size_t count(const std::string &text) const
  {
    return std::count_if(lines.begin(), lines.end(), [&text](const auto &line) { return line.find(text) != std::string::npos; });
  }

  bool contains(const std::string &text) const
  {
    return count(text) != 0;
  }

  std::vector<std::string> lines;
};

#endif
