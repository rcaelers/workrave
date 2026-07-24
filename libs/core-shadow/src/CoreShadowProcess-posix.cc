#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "CoreShadowClient.hh"

#if defined(PLATFORM_OS_UNIX) || defined(PLATFORM_OS_MACOS)

#  include <cerrno>
#  include <cstring>
#  include <fcntl.h>
#  include <signal.h>
#  include <sys/select.h>
#  include <sys/wait.h>
#  include <unistd.h>

#  include <spdlog/spdlog.h>

namespace workrave::core_shadow
{
  namespace
  {
    class PosixCoreShadowProcess : public CoreShadowProcess
    {
    public:
      ~PosixCoreShadowProcess() override
      {
        stop();
      }

      auto start(const std::filesystem::path &helper) -> bool override
      {
        int to_child[2]{-1, -1};
        int from_child[2]{-1, -1};
        if (pipe(to_child) != 0 || pipe(from_child) != 0)
          {
            close_pair(to_child);
            close_pair(from_child);
            return false;
          }

        child_pid = fork();
        if (child_pid < 0)
          {
            close_pair(to_child);
            close_pair(from_child);
            return false;
          }

        if (child_pid == 0)
          {
            dup2(to_child[0], STDIN_FILENO);
            dup2(from_child[1], STDOUT_FILENO);
            close_pair(to_child);
            close_pair(from_child);

            const auto helper_string = helper.string();
            execl(helper_string.c_str(), helper_string.c_str(), static_cast<char *>(nullptr));
            _exit(127);
          }

        close(to_child[0]);
        close(from_child[1]);
        write_fd = to_child[1];
        read_fd = from_child[0];
        fcntl(read_fd, F_SETFL, fcntl(read_fd, F_GETFL, 0) | O_NONBLOCK);
        return true;
      }

      void stop() override
      {
        if (write_fd >= 0)
          {
            close(write_fd);
            write_fd = -1;
          }
        if (read_fd >= 0)
          {
            close(read_fd);
            read_fd = -1;
          }
        if (child_pid > 0)
          {
            int status = 0;
            if (waitpid(child_pid, &status, WNOHANG) == 0)
              {
                kill(child_pid, SIGTERM);
                waitpid(child_pid, &status, 0);
              }
            child_pid = -1;
          }
      }

      auto exchange(const std::string &command, ObservationBatch &batch) -> bool override
      {
        if (write_fd < 0 || read_fd < 0)
          {
            return false;
          }

        const std::string line = command + "\n";
        if (!write_all(line))
          {
            return false;
          }

        return read_until_done(batch);
      }

    private:
      static void close_pair(int fds[2])
      {
        if (fds[0] >= 0)
          {
            close(fds[0]);
          }
        if (fds[1] >= 0)
          {
            close(fds[1]);
          }
      }

      auto write_all(const std::string &text) -> bool
      {
        const char *data = text.data();
        size_t remaining = text.size();
        while (remaining > 0)
          {
            ssize_t written = write(write_fd, data, remaining);
            if (written < 0)
              {
                if (errno == EINTR)
                  {
                    continue;
                  }
                return false;
              }
            data += written;
            remaining -= static_cast<size_t>(written);
          }
        return true;
      }

      auto read_until_done(ObservationBatch &batch) -> bool
      {
        while (true)
          {
            auto line = read_line();
            if (!line)
              {
                return false;
              }
            if (*line == "done")
              {
                return true;
              }
            if (line->rfind("error\t", 0) == 0)
              {
                spdlog::warn("Core shadow helper error: {}", *line);
                return false;
              }
            parse_observation(*line, batch);
          }
      }

      auto read_line() -> std::optional<std::string>
      {
        while (true)
          {
            auto pos = buffer.find('\n');
            if (pos != std::string::npos)
              {
                std::string line = buffer.substr(0, pos);
                buffer.erase(0, pos + 1);
                return line;
              }

            fd_set set;
            FD_ZERO(&set);
            FD_SET(read_fd, &set);
            timeval timeout{.tv_sec = 2, .tv_usec = 0};
            int ready = select(read_fd + 1, &set, nullptr, nullptr, &timeout);
            if (ready < 0)
              {
                if (errno == EINTR)
                  {
                    continue;
                  }
                return std::nullopt;
              }
            if (ready == 0)
              {
                return std::nullopt;
              }

            char data[4096];
            ssize_t count = read(read_fd, data, sizeof(data));
            if (count < 0)
              {
                if (errno == EINTR || errno == EAGAIN)
                  {
                    continue;
                  }
                return std::nullopt;
              }
            if (count == 0)
              {
                return std::nullopt;
              }
            buffer.append(data, static_cast<size_t>(count));
          }
      }

    private:
      pid_t child_pid{-1};
      int read_fd{-1};
      int write_fd{-1};
      std::string buffer;
    };
  } // namespace

  auto create_core_shadow_process() -> std::unique_ptr<CoreShadowProcess>
  {
    return std::make_unique<PosixCoreShadowProcess>();
  }
} // namespace workrave::core_shadow

#else

namespace workrave::core_shadow
{
  auto create_core_shadow_process() -> std::unique_ptr<CoreShadowProcess>
  {
    return nullptr;
  }
} // namespace workrave::core_shadow

#endif
