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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "WindowsTaskbarApplet.hh"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>
#include <commctrl.h>
#include <windowsx.h>

#include "TaskbarLayout.hh"
#include "WindowsImaging.hh"
#include "ui/GUIConfig.hh"
#include "ui/prefwidgets/Widgets.hh"
#include "commonui/Text.hh"
#include "commonui/nls.h"
#include "utils/StringUtils.hh"
#include "utils/AssetPath.hh"
#include "spdlog/spdlog.h"

using namespace workrave::windows;

namespace
{
  class DpiScope
  {
  public:
    explicit DpiScope(DPI_AWARENESS_CONTEXT context)
      : previous(check_handle(SetThreadDpiAwarenessContext(context), "Set taskbar DPI context"))
    {
    }
    DpiScope(const DpiScope &) = delete;
    DpiScope &operator=(const DpiScope &) = delete;
    DpiScope(DpiScope &&) = delete;
    DpiScope &operator=(DpiScope &&) = delete;
    ~DpiScope()
    {
      if (previous != nullptr)
        {
          SetThreadDpiAwarenessContext(previous);
        }
    }

  private:
    DPI_AWARENESS_CONTEXT previous;
  };

  constexpr auto timerbox_name = "applet";
  constexpr auto window_class = L"WorkraveTaskbarApplet";
  constexpr COLORREF timer_background = RGB(245, 245, 220);

  bool use_dark_theme()
  {
    DWORD light = 1;
    DWORD size = sizeof(light);
    // Windows system mode controls the taskbar, independently of app mode.
    const auto result = RegGetValueW(HKEY_CURRENT_USER,
                                     L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                                     L"SystemUsesLightTheme",
                                     RRF_RT_REG_DWORD,
                                     nullptr,
                                     &light,
                                     &size);
    return result == ERROR_SUCCESS && light == 0;
  }

  void fill(HDC dc, RECT rect, COLORREF color)
  {
    auto brush = create_solid_brush(color);
    FillRect(dc, &rect, brush.get());
  }

  std::wstring wide(const std::string &text)
  {
    return workrave::utils::utf8_to_utf16(text);
  }

  UniqueIcon load_timer_icon(const ImagingFactory &factory, const std::wstring &path)
  {
    const auto decoded = factory.load(path);
    const auto [width, height] = decoded.size();
    if (width == 0 || height == 0 || width > 256 || height > 256)
      {
        throw std::runtime_error("Timer icon dimensions must be between 1 and 256 pixels");
      }
    const auto image = factory.to_pbgra(decoded);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = static_cast<LONG>(width);
    info.bmiHeader.biHeight = -static_cast<LONG>(height);
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    std::vector<BYTE> mask_bits(((static_cast<size_t>(width) + 15) / 16) * 2 * height, 0);
    auto color = create_dib_section(info);
    auto mask = create_bitmap(static_cast<int>(width), static_cast<int>(height), 1, 1, mask_bits.data());
    image.copy_pixels(width * 4, {static_cast<BYTE *>(color.pixels), static_cast<size_t>(width) * height * 4});
    ICONINFO icon{};
    icon.fIcon = TRUE;
    icon.hbmColor = color.bitmap.get();
    icon.hbmMask = mask.get();
    return create_icon(icon);
  }

  int timer_grouping_value()
  {
    int micro = GUIConfig::timerbox_slot(timerbox_name, workrave::BREAK_ID_MICRO_BREAK)();
    int rest = GUIConfig::timerbox_slot(timerbox_name, workrave::BREAK_ID_REST_BREAK)();
    int daily = GUIConfig::timerbox_slot(timerbox_name, workrave::BREAK_ID_DAILY_LIMIT)();
    if (micro < rest && rest < daily)
      {
        return 0;
      }
    if (micro == rest && rest == daily)
      {
        return 3;
      }
    return micro == rest ? 1 : 2;
  }

  void set_timer_grouping(int value)
  {
    constexpr std::array<std::array<int, 3>, 4> grouping_slots{{{0, 1, 2}, {0, 0, 1}, {0, 1, 1}, {0, 0, 0}}};
    if (value < 0 || !std::cmp_less(value, grouping_slots.size()))
      {
        return;
      }
    const auto &positions = grouping_slots.at(static_cast<size_t>(value));
    for (size_t i = 0; i < positions.size(); ++i)
      {
        GUIConfig::timerbox_slot(timerbox_name, static_cast<workrave::BreakId>(i)).set(positions.at(i));
      }
  }

  int timer_visibility_value(workrave::BreakId id)
  {
    auto flags = static_cast<unsigned int>(GUIConfig::timerbox_flags(timerbox_name, id)());
    if ((flags & static_cast<unsigned int>(GUIConfig::BREAK_HIDE)) != 0U)
      {
        return 2;
      }
    return (flags & static_cast<unsigned int>(GUIConfig::BREAK_WHEN_FIRST)) != 0U ? 1 : 0;
  }

  void set_timer_visibility(workrave::BreakId id, int value)
  {
    int flags = 0;
    if (value == 2)
      {
        flags = GUIConfig::BREAK_HIDE;
      }
    else if (value == 1)
      {
        flags = GUIConfig::BREAK_WHEN_FIRST;
      }
    GUIConfig::timerbox_flags(timerbox_name, id).set(flags);
  }

  void log_cleanup_error(const char *operation, const std::exception &error) noexcept
  {
    try
      {
        spdlog::warn("Cannot {}: {}", operation, error.what());
      }
    catch (...)
      {
        OutputDebugStringA(error.what());
      }
  }

  COLORREF timer_color(TimerColorId color, bool high_contrast)
  {
    if (high_contrast)
      {
        return GetSysColor(COLOR_HIGHLIGHT);
      }
    switch (color)
      {
      case TimerColorId::Active:
        return RGB(173, 216, 230);
      case TimerColorId::Inactive:
      case TimerColorId::InactiveOverOverdue:
        return RGB(144, 238, 144);
      case TimerColorId::Overdue:
        return RGB(255, 165, 0);
      case TimerColorId::InactiveOverActive:
        return RGB(0, 212, 178);
      case TimerColorId::Bg:
        return timer_background;
      }
    return timer_background;
  }
} // namespace

WindowsTaskbarApplet::WindowsTaskbarApplet(std::shared_ptr<IPluginContext> context)
  : context(std::move(context))
{
  init_preferences();
  load_icons();
  GUIConfig::icon_theme().connect(this, [this](const std::string &) {
    load_icons();
    update_view();
  });
  timer_slots.fill(workrave::BREAK_ID_NONE);
  WNDCLASSW wc{};
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = window_class;
  wc.lpfnWndProc = window_proc;
  wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
  if (RegisterClassW(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
      spdlog::warn("Cannot register taskbar applet window: {}", GetLastError());
      return;
    }
  // A top-level observer survives Explorer restarting and owns popup menus.
  observer = CreateWindowExW(WS_EX_TOOLWINDOW,
                             window_class,
                             L"Workrave taskbar observer",
                             WS_POPUP,
                             0,
                             0,
                             0,
                             0,
                             nullptr,
                             nullptr,
                             wc.hInstance,
                             this);
  if (observer == nullptr)
    {
      return;
    }
  control = std::make_unique<TimerBoxControl>(this->context->get_core(), timerbox_name, this);
  timer_connection = this->context->get_toolkit()->signal_timer().connect([this] { tick(); });
  GUIConfig::timerbox_enabled(timerbox_name).connect(this, [this](bool) { tick(); });
  tick();
}

WindowsTaskbarApplet::~WindowsTaskbarApplet()
{
  try
    {
      timer_connection.disconnect();
      context->get_preferences_registry()->remove(preferences);
    }
  catch (const std::exception &error)
    {
      log_cleanup_error("unregister taskbar applet", error);
    }
  layout.reset();
  detach();
  try
    {
      sync_hold();
    }
  catch (const std::exception &error)
    {
      log_cleanup_error("release taskbar applet hold", error);
    }
  if (observer != nullptr)
    {
      DestroyWindow(observer);
    }
}

void
WindowsTaskbarApplet::load_icons()
{
  using namespace workrave::utils;
  try
    {
      ComApartment apartment(COINIT_APARTMENTTHREADED, ComApartment::ExistingApartment::AllowDifferentMode);
      ImagingFactory factory;
      constexpr std::array names{"timer-micro-break.png", "timer-rest-break.png", "timer-daily.png"};
      auto theme = GUIConfig::icon_theme()();
      for (size_t i = 0; i < icons.size(); ++i)
        {
          try
            {
              std::string path;
              if (theme.empty() || !AssetPath::complete_directory(theme + "/" + names.at(i), SearchPathId::Images, path))
                {
                  AssetPath::complete_directory(names.at(i), SearchPathId::Images, path);
                }
              // Replace only after loading succeeds, keeping any previous usable icon.
              icons.at(i) = load_timer_icon(factory, wide(path));
            }
          catch (const std::exception &error)
            {
              spdlog::warn("Cannot load taskbar timer icon '{}': {}", names.at(i), error.what());
            }
        }
    }
  catch (const std::exception &error)
    {
      spdlog::warn("Cannot load taskbar applet icons: {}", error.what());
    }
}
void
WindowsTaskbarApplet::init_preferences()
{
  using namespace ui::prefwidgets;
  auto &enabled = GUIConfig::timerbox_enabled(timerbox_name);
  auto grouping =
    Choice::create(N_("Timer grouping"))
      ->presentation({.name = "workrave.timer-grouping"})
      ->assign({
        N_("Show timers separately"),
        N_("Group micro-break and rest break"),
        N_("Group rest break and daily limit"),
        N_("Group all timers"),
      })
      ->when(&enabled)
      ->on_save(set_timer_grouping);
  constexpr std::array break_ids{workrave::BREAK_ID_MICRO_BREAK, workrave::BREAK_ID_REST_BREAK, workrave::BREAK_ID_DAILY_LIMIT};
  for (auto id: break_ids)
    {
      grouping->connect(&GUIConfig::timerbox_slot(timerbox_name, id), timer_grouping_value);
    }
  auto timers = Frame::create(N_("Timers"))->presentation({.label = N_("Per-timer visibility")});
  constexpr std::array accent_roles{"accent", "secondary", "accentStrong"};
  constexpr std::array labels{N_("Micro-break"), N_("Rest break"), N_("Daily limit")};
  for (auto id: break_ids)
    {
      auto visibility = Choice::create(labels.at(static_cast<size_t>(id)))
                          ->presentation({
                            .control = ChoicePresentation::Control::Segmented,
                            .accent = {.role = accent_roles.at(static_cast<size_t>(id))},
                            .options = {N_("Show"), N_("When first due"), N_("Hide")},
                          })
                          ->assign({N_("Show"), N_("Show only when this timer is first due"), N_("Hide")})
                          ->connect(&GUIConfig::timerbox_flags(timerbox_name, id), [id] { return timer_visibility_value(id); })
                          ->on_save([id](int value) { set_timer_visibility(id, value); })
                          ->when(&enabled);
      *timers << visibility;
    }
  preferences = PanelDef::create("ui", "applet", N_("Applet"))
                << (Box::create()->spacing(6) << (Frame::create(N_("Taskbar"))->presentation({.label = N_("Visibility")})
                                                  << Toggle::create(N_("Show timers in taskbar"))->connect(&enabled))
                                              << (Frame::create(N_("Display"))->presentation({.label = N_("Layout")})
                                                  << grouping
                                                  << Value::create(N_("Cycle time (seconds)"), 1, 999)
                                                       ->presentation({
                                                         .control = ValuePresentation::Control::Duration,
                                                         .label = N_("Cycle time"),
                                                         .hint = N_("When timers share a slot, switch between them this often."),
                                                         .slider_min = 2,
                                                         .slider_max = 30,
                                                         .ticks = {2, 10, 30},
                                                       })
                                                       ->connect(&GUIConfig::timerbox_cycle_time(timerbox_name))
                                                       ->when(&enabled))
                                              << timers);
  context->get_preferences_registry()->add(preferences);
}
void
WindowsTaskbarApplet::tick()
{
  try
    {
      if (!GUIConfig::timerbox_enabled(timerbox_name)())
        {
          detach();
          layout.reset();
          sync_hold();
          return;
        }
      if (!layout)
        {
          layout = std::make_unique<TaskbarLayout>();
        }
      control->update();
      reposition();
      sync_hold();
    }
  catch (const std::exception &error)
    {
      spdlog::warn("Taskbar applet update failed: {}", error.what());
      detach();
      layout.reset();
      sync_hold();
    }
}

void
WindowsTaskbarApplet::reposition()
{
  auto geometry = layout->snapshot();
  if (IsWindow(geometry.taskbar) == FALSE)
    {
      detach();
      return;
    }
  // Window rectangles and UI Automation bounds must use the same physical
  // coordinate space. The scope also restores DPI context on every early return.
  DpiScope dpi_scope(GetWindowDpiAwarenessContext(geometry.taskbar));
  if (IsWindow(geometry.taskbar) == FALSE || std::chrono::steady_clock::now() - geometry.measured > std::chrono::seconds(5))
    {
      detach();
      return;
    }
  const auto current = get_window_rect(geometry.taskbar);
  if (EqualRect(&current, &geometry.bounds) == FALSE)
    {
      detach();
      return;
    }

  UINT next_dpi = GetDpiForWindow(geometry.taskbar);
  next_dpi = next_dpi == 0 ? 96 : next_dpi;
  const auto scale = [next_dpi](int value) { return MulDiv(value, static_cast<int>(next_dpi), 96); };
  int count = static_cast<int>(
    std::count_if(timer_slots.begin(), timer_slots.end(), [](auto id) { return id != workrave::BREAK_ID_NONE; }));
  int height = current.bottom - current.top;
  int width = scale(90);
  int next_font_height = std::max(1, std::min(scale(12), ((height - scale(4)) / std::max(1, count)) - scale(4)));
  auto x = workrave::taskbar::find_slot(current.left, current.right, width, scale(6), geometry.occupied);
  if (count == 0 || !x)
    {
      detach();
    }
  else if (attach(geometry.taskbar))
    {
      if (dpi != next_dpi || font_height != next_font_height)
        {
          auto next_font = create_font({
            .lfHeight = -next_font_height,
            .lfWeight = FW_NORMAL,
            .lfCharSet = DEFAULT_CHARSET,
            .lfQuality = CLEARTYPE_QUALITY,
            .lfFaceName = L"Segoe UI",
          });
          font = std::move(next_font);
          dpi = next_dpi;
          font_height = next_font_height;
        }
      POINT point{.x = *x, .y = current.top + scale(2)};
      check_win32(ScreenToClient(geometry.taskbar, &point), "Convert taskbar coordinates");
      check_win32(SetWindowPos(window, HWND_TOP, point.x, point.y, width, height - scale(4), SWP_NOACTIVATE | SWP_SHOWWINDOW),
                  "Position taskbar applet");
      InvalidateRect(window, nullptr, FALSE);
      update_tooltip();
    }
}

bool
WindowsTaskbarApplet::attach(HWND taskbar)
{
  if (window != nullptr && IsWindow(window) != FALSE && GetParent(window) == taskbar)
    {
      return true;
    }
  detach();
  // Create as a child from the outset. Changing a popup into a child with
  // SetParent can fail during cross-process attachment to Explorer.
  window = check_handle(CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_LAYERED,
                                        window_class,
                                        L"Workrave timers",
                                        WS_CHILD | WS_CLIPSIBLINGS,
                                        0,
                                        0,
                                        0,
                                        0,
                                        taskbar,
                                        nullptr,
                                        GetModuleHandleW(nullptr),
                                        this),
                        "Create taskbar applet child window");
  // Explorer's Windows 11 taskbar uses DirectComposition. A plain GDI child
  // can be visible in the window tree without its pixels being composited.
  // Give our child its own redirected surface, keeping the background opaque.
  // Layered windows must have their attributes initialized before being shown.
  if (SetLayeredWindowAttributes(window, 0, 255, LWA_ALPHA) == FALSE)
    {
      spdlog::warn("Cannot initialize Workrave taskbar applet surface: {}", GetLastError());
      detach();
      return false;
    }
  // No topmost/floating fallback: a failed embedding must never cover apps.
  tooltip = CreateWindowExW(WS_EX_TOPMOST,
                            TOOLTIPS_CLASSW,
                            nullptr,
                            WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            window,
                            nullptr,
                            GetModuleHandleW(nullptr),
                            nullptr);
  TOOLINFOW tool{};
  tool.cbSize = sizeof(tool);
  tool.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
  tool.hwnd = window;
  // Win32 stores handles and pointers in integer-sized message fields.
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  tool.uId = reinterpret_cast<UINT_PTR>(window);
  tooltip_text = L"Workrave";
  tool.lpszText = tooltip_text.data();
  // Win32 stores handles and pointers in integer-sized message fields.
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  SendMessageW(tooltip, TTM_ADDTOOLW, 0, reinterpret_cast<LPARAM>(&tool));
  SendMessageW(tooltip, TTM_SETMAXTIPWIDTH, 0, 400);
  return true;
}

void
WindowsTaskbarApplet::detach()
{
  if (tooltip != nullptr && IsWindow(tooltip) != FALSE)
    {
      DestroyWindow(tooltip);
    }
  tooltip = nullptr;
  if (window != nullptr && IsWindow(window) != FALSE)
    {
      DestroyWindow(window);
    }
  window = nullptr;
}

void
WindowsTaskbarApplet::sync_hold()
{
  // Toolkit calls run under Workrave's own DPI context, never Explorer's.
  bool visible = window != nullptr && IsWindowVisible(window) != FALSE;
  if (visible != held)
    {
      held = visible;
      if (held)
        {
          context->get_toolkit()->hold();
        }
      else
        {
          context->get_toolkit()->release();
        }
    }
}

void
WindowsTaskbarApplet::set_slot(workrave::BreakId id, int slot)
{
  if (slot >= 0 && std::cmp_less(slot, timer_slots.size()))
    {
      timer_slots.at(static_cast<size_t>(slot)) = id;
    }
}

void
WindowsTaskbarApplet::set_time_bar(workrave::BreakId id,
                                   int value,
                                   TimerColorId primary_color,
                                   int primary_value,
                                   int primary_max,
                                   TimerColorId secondary_color,
                                   int secondary_value,
                                   int secondary_max)
{
  if (id >= 0 && id < workrave::BREAK_ID_SIZEOF)
    {
      timers.at(static_cast<size_t>(id)) = {
        .value = value,
        .primary = primary_value,
        .primary_max = primary_max,
        .secondary = secondary_value,
        .secondary_max = secondary_max,
        .color = primary_color,
        .secondary_color = secondary_color,
      };
    }
}

void
WindowsTaskbarApplet::set_icon(OperationModeIcon icon)
{
  mode = icon;
}

void
WindowsTaskbarApplet::update_view()
{
  if (window != nullptr)
    {
      InvalidateRect(window, nullptr, FALSE);
    }
}

void
WindowsTaskbarApplet::update_tooltip()
{
  tooltip_text = L"Workrave";
  if (mode == OperationModeIcon::Suspended)
    {
      tooltip_text += L" — " + wide(_("Suspended"));
    }
  else if (mode == OperationModeIcon::Quiet)
    {
      tooltip_text += L" — " + wide(_("Quiet"));
    }
  const std::array names{_("Micro-break"), _("Rest break"), _("Daily limit")};
  for (auto id: timer_slots)
    {
      if (id != workrave::BREAK_ID_NONE)
        {
          auto index = static_cast<size_t>(id);
          tooltip_text += L"\n" + wide(names.at(index)) + L": " + wide(Text::time_to_string(timers.at(index).value));
        }
    }
  TOOLINFOW tool{};
  tool.cbSize = sizeof(tool);
  tool.hwnd = window;
  // Win32 stores handles and pointers in integer-sized message fields.
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  tool.uId = reinterpret_cast<UINT_PTR>(window);
  tool.lpszText = tooltip_text.data();
  // Win32 stores handles and pointers in integer-sized message fields.
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  SendMessageW(tooltip, TTM_UPDATETIPTEXTW, 0, reinterpret_cast<LPARAM>(&tool));
}

void
WindowsTaskbarApplet::paint(HDC dc, RECT rect)
{
  HIGHCONTRASTW contrast{};
  contrast.cbSize = sizeof(contrast);
  SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(contrast), &contrast, 0);
  Palette palette{
    .high_contrast = (contrast.dwFlags & HCF_HIGHCONTRASTON) != 0,
    .background = timer_background,
    .foreground = RGB(0, 0, 0),
    .fill_foreground = RGB(0, 0, 0),
    .frame = RGB(160, 160, 160),
  };
  if (use_dark_theme())
    {
      palette.background = RGB(51, 51, 51);
      palette.foreground = RGB(255, 255, 255);
      palette.frame = RGB(90, 90, 90);
    }
  if (palette.high_contrast)
    {
      palette.fill_foreground = GetSysColor(COLOR_HIGHLIGHTTEXT);
      palette.background = GetSysColor(COLOR_WINDOW);
      palette.foreground = GetSysColor(COLOR_WINDOWTEXT);
      palette.frame = GetSysColor(COLOR_WINDOWTEXT);
    }
  fill(dc, rect, palette.background);
  GdiSelection selected_font(dc, font != nullptr ? font.get() : GetStockObject(DEFAULT_GUI_FONT));
  SetBkMode(dc, TRANSPARENT);
  int count = static_cast<int>(std::ranges::count_if(timer_slots, [](auto id) { return id != workrave::BREAK_ID_NONE; }));
  int index = 0;
  for (auto id: timer_slots)
    {
      if (id == workrave::BREAK_ID_NONE)
        {
          continue;
        }
      RECT cell = rect;
      cell.top = rect.top + ((index * (rect.bottom - rect.top)) / count);
      cell.bottom = rect.top + (((index + 1) * (rect.bottom - rect.top)) / count);
      ++index;
      paint_timer(dc, cell, id, palette);
    }
}

void
WindowsTaskbarApplet::paint_timer(HDC dc, RECT cell, workrave::BreakId id, const Palette &palette)
{
  auto index = static_cast<size_t>(id);
  const auto scale = [this](int value) { return MulDiv(value, static_cast<int>(dpi), 96); };
  const auto &timer = timers.at(index);
  cell.left += scale(3);
  cell.right -= scale(3);
  int row_height = std::max(1, std::min(static_cast<int>(cell.bottom - cell.top) - scale(2), scale(20)));
  cell.top += (cell.bottom - cell.top - row_height) / 2;
  cell.bottom = cell.top + row_height;
  int icon_size = std::min(scale(16), row_height);
  if (icons.at(index) != nullptr)
    {
      DrawIconEx(dc,
                 cell.left,
                 cell.top + ((row_height - icon_size) / 2),
                 icons.at(index).get(),
                 icon_size,
                 icon_size,
                 0,
                 nullptr,
                 DI_NORMAL);
    }
  else
    {
      constexpr std::array prefixes{L"M", L"R", L"D"};
      RECT label{.left = cell.left, .top = cell.top, .right = cell.left + icon_size, .bottom = cell.bottom};
      SetTextColor(dc, palette.foreground);
      DrawTextW(dc, prefixes.at(index), -1, &label, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }
  cell.left += icon_size + scale(4);
  fill(dc, cell, palette.frame);
  RECT bar = cell;
  InflateRect(&bar, -1, -1);
  fill(dc, bar, palette.background);
  auto progress_width = [&](int value, int maximum) {
    return maximum > 0 ? static_cast<int>(static_cast<int64_t>(bar.right - bar.left) * std::clamp(value, 0, maximum) / maximum)
                       : 0;
  };
  int primary = progress_width(timer.primary, timer.primary_max);
  int secondary = progress_width(timer.secondary, timer.secondary_max);
  int overlap = std::min(primary, secondary);
  auto segment = [&](int left, int right, TimerColorId color) {
    if (right > left)
      {
        RECT part{.left = bar.left + left, .top = bar.top, .right = bar.left + right, .bottom = bar.bottom};
        fill(dc, part, timer_color(color, palette.high_contrast));
      }
  };
  segment(0,
          overlap,
          timer.color == TimerColorId::Overdue ? TimerColorId::InactiveOverOverdue : TimerColorId::InactiveOverActive);
  segment(overlap, primary, timer.color);
  segment(overlap, secondary, timer.secondary_color);
  auto text = wide(Text::time_to_string(timer.value));
  if (mode == OperationModeIcon::Suspended)
    {
      text = L"|| " + text;
    }
  else if (mode == OperationModeIcon::Quiet)
    {
      text = L"~ " + text;
    }
  RECT text_rect = bar;
  text_rect.right -= scale(4);
  SetTextColor(dc, palette.foreground);
  DrawTextW(dc, text.c_str(), -1, &text_rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
  if (palette.foreground != palette.fill_foreground)
    {
      int saved = SaveDC(dc);
      IntersectClipRect(dc, bar.left, bar.top, bar.left + std::max(primary, secondary), bar.bottom);
      SetTextColor(dc, palette.fill_foreground);
      DrawTextW(dc, text.c_str(), -1, &text_rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
      RestoreDC(dc, saved);
    }
}

void
WindowsTaskbarApplet::append_menu(HMENU menu, const menus::Node::Ptr &node, std::vector<menus::Node::Ptr> &commands)
{
  if (!node->is_visible())
    {
      return;
    }
  if (auto group = std::dynamic_pointer_cast<menus::RadioGroupNode>(node))
    {
      for (const auto &child: group->get_children())
        {
          append_menu(menu, child, commands);
        }
    }
  else if (auto container = std::dynamic_pointer_cast<menus::ContainerNode>(node))
    {
      HMENU target = menu;
      if (std::dynamic_pointer_cast<menus::SubMenuNode>(node))
        {
          target = append_submenu(menu, create_popup_menu(), wide(node->get_dynamic_text_no_accel()));
        }
      for (const auto &child: container->get_children())
        {
          append_menu(target, child, commands);
        }
    }
  else if (std::dynamic_pointer_cast<menus::SeparatorNode>(node))
    {
      append_menu_separator(menu);
    }
  else
    {
      UINT flags = MF_STRING;
      if (auto toggle = std::dynamic_pointer_cast<menus::ToggleNode>(node); toggle && toggle->is_checked())
        {
          flags |= MF_CHECKED;
        }
      if (auto radio = std::dynamic_pointer_cast<menus::RadioNode>(node); radio && radio->is_checked())
        {
          flags |= MF_CHECKED;
        }
      commands.push_back(node);
      append_menu_item(menu, flags, commands.size(), wide(node->get_dynamic_text_no_accel()));
    }
}

void
WindowsTaskbarApplet::show_menu(POINT point)
{
  std::vector<menus::Node::Ptr> commands;
  auto menu = create_popup_menu();
  for (const auto &node: context->get_menu_model()->get_root()->get_children())
    {
      append_menu(menu.get(), node, commands);
    }
  SetForegroundWindow(observer);
  UINT command = TrackPopupMenu(menu.get(), TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, observer, nullptr);
  PostMessageW(observer, WM_NULL, 0, 0);
  if (command > 0 && command <= commands.size())
    {
      auto node = commands.at(static_cast<size_t>(command - 1));
      context->get_toolkit()->create_oneshot_timer(0, [node] { node->activate(); });
    }
}
LRESULT CALLBACK
WindowsTaskbarApplet::window_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
  // Win32 stores handles and pointers in integer-sized message fields.
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto *self = reinterpret_cast<WindowsTaskbarApplet *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (message == WM_NCCREATE)
    {
      // Win32 stores handles and pointers in integer-sized message fields.
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      self = static_cast<WindowsTaskbarApplet *>(reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams);
      // Win32 stores handles and pointers in integer-sized message fields.
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
  if (self != nullptr)
    {
      try
        {
          switch (message)
            {
            case WM_SETTINGCHANGE:
            case WM_THEMECHANGED:
            case WM_SYSCOLORCHANGE:
              self->update_view();
              return 0;
            case WM_ERASEBKGND:
              return 1;
            case WM_MOUSEACTIVATE:
              return MA_NOACTIVATE;
            case WM_PAINT:
              {
                PaintScope paint(hwnd);
                HDC dc = paint.get();
                RECT rect{};
                GetClientRect(hwnd, &rect);
                // Buffered painting avoids flicker during the one-second refresh.
                auto buffer = create_compatible_dc(dc);
                auto bitmap = create_compatible_bitmap(dc, std::max(1L, rect.right), std::max(1L, rect.bottom));
                GdiSelection selected_bitmap(buffer.get(), bitmap.get());
                self->paint(buffer.get(), rect);
                BitBlt(dc, 0, 0, rect.right, rect.bottom, buffer.get(), 0, 0, SRCCOPY);
                return 0;
              }
            case WM_LBUTTONUP:
              self->context->get_toolkit()->show_window(IToolkit::WindowType::Main);
              return 0;
            case WM_MBUTTONUP:
              self->control->force_cycle();
              self->control->update();
              return 0;
            case WM_CONTEXTMENU:
              {
                POINT point{.x = GET_X_LPARAM(lp), .y = GET_Y_LPARAM(lp)};
                if (point.x == -1 && point.y == -1)
                  {
                    GetCursorPos(&point);
                  }
                self->show_menu(point);
                return 0;
              }
            case WM_NCDESTROY:
              if (hwnd == self->window)
                {
                  self->window = nullptr;
                  self->tooltip = nullptr;
                }
              SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
              break;
            default:
              break;
            }
        }
      catch (const std::exception &error)
        {
          spdlog::warn("Taskbar applet event failed: {}", error.what());
        }
    }
  return DefWindowProcW(hwnd, message, wp, lp);
}
