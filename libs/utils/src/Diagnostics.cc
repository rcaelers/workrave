#include "utils/Diagnostics.hh"

#include <ctime>

bool TracedFieldBase::debug = false;

void
Diagnostics::enable(DiagnosticsSink *sink)
{
  this->sink = sink;
  enabled = true;
  TracedFieldBase::debug = true;
  for (const auto &kv: topics)
    {
      kv.second();
    }
}

void
Diagnostics::disable()
{
  enabled = false;
  sink = nullptr;
  TracedFieldBase::debug = false;
}

void
Diagnostics::register_topic(const std::string &name, request_t func)
{
  topics[name] = func;
}

void
Diagnostics::unregister_topic(const std::string &name)
{
  topics.erase(name);
}

std::string
Diagnostics::trace_get_time()
{
  char logtime[128];
  time_t ltime;

  time(&ltime);
  struct tm *tmlt = localtime(&ltime);
  strftime(logtime, 128, "%d %b %Y %H:%M:%S ", tmlt);
  return logtime;
}
