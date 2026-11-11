// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Rob Caelers

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "WindowsTaskbarApplet.hh"

#include <algorithm>
#include <utility>
#include <vector>
#include <commctrl.h>
#include <windowsx.h>
#include <wincodec.h>
#include <wrl/client.h>

#include "TaskbarLayout.hh"
#include "ui/GUIConfig.hh"
#include "ui/prefwidgets/Widgets.hh"
#include "commonui/Text.hh"
#include "commonui/nls.h"
#include "utils/StringUtils.hh"
#include "utils/AssetPath.hh"
#include "spdlog/spdlog.h"

namespace
{
  class DpiScope
  {
  public:
    explicit DpiScope(DPI_AWARENESS_CONTEXT context)
      : previous(SetThreadDpiAwarenessContext(context))
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
    switch (GUIConfig::light_dark_mode()())
      {
      case LightDarkTheme::Light:
        return false;
      case LightDarkTheme::Dark:
        return true;
      case LightDarkTheme::Auto:
        {
          DWORD light = 1;
          DWORD size = sizeof(light);
          RegGetValueW(HKEY_CURRENT_USER,
                       L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                       L"AppsUseLightTheme",
                       RRF_RT_REG_DWORD,
                       nullptr,
                       &light,
                       &size);
          return light == 0;
        }
      }
    return false;
  }

  void fill(HDC dc, RECT rect, COLORREF color)
  {
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(dc, &rect, brush);
    DeleteObject(brush);
  }

  std::wstring wide(const std::string &text)
  {
    return workrave::utils::utf8_to_utf16(text);
  }

  HICON load_timer_icon(IWICImagingFactory *factory, const std::wstring &path)
  {
    using Microsoft::WRL::ComPtr;
    ComPtr<IWICBitmapDecoder> decoder;
    ComPtr<IWICBitmapFrameDecode> frame;
    ComPtr<IWICFormatConverter> converter;
    UINT width = 0;
    UINT height = 0;
    if (FAILED(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder))
        || FAILED(decoder->GetFrame(0, &frame)) || FAILED(frame->GetSize(&width, &height)) || width == 0 || height == 0
        || width > 256 || height > 256 || FAILED(factory->CreateFormatConverter(&converter))
        || FAILED(converter->Initialize(frame.Get(),
                                        GUID_WICPixelFormat32bppPBGRA,
                                        WICBitmapDitherTypeNone,
                                        nullptr,
                                        0,
                                        WICBitmapPaletteTypeCustom)))
      {
        return nullptr;
      }
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = static_cast<LONG>(width);
    info.bmiHeader.biHeight = -static_cast<LONG>(height);
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void *pixels = nullptr;
    std::vector<BYTE> mask_bits(((static_cast<size_t>(width) + 15) / 16) * 2 * height, 0);
    HBITMAP color = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);
    HBITMAP mask = CreateBitmap(static_cast<int>(width), static_cast<int>(height), 1, 1, mask_bits.data());
    HICON result = nullptr;
    if (color != nullptr && mask != nullptr
        && SUCCEEDED(converter->CopyPixels(nullptr, width * 4, width * height * 4, static_cast<BYTE *>(pixels))))
      {
        ICONINFO icon{};
        icon.fIcon = TRUE;
        icon.hbmColor = color;
        icon.hbmMask = mask;
        result = CreateIconIndirect(&icon);
      }
    if (color != nullptr)
      {
        DeleteObject(color);
      }
    if (mask != nullptr)
      {
        DeleteObject(mask);
      }
    return result;
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
    constexpr std::array<std::array<int, 3>, 4> slots{{{0, 1, 2}, {0, 0, 1}, {0, 1, 1}, {0, 0, 0}}};
    if (value < 0 || !std::cmp_less(value, slots.size()))
      {
        return;
      }
    const auto &positions = slots.at(static_cast<size_t>(value));
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
  GUIConfig::light_dark_mode().connect(this, [this](LightDarkTheme) { update_view(); });
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
  if (font != nullptr)
    {
      DeleteObject(font);
    }
  for (auto *icon: icons)
    {
      if (icon != nullptr)
        {
          DestroyIcon(icon);
        }
    }
}

void
WindowsTaskbarApplet::load_icons()
{
  using Microsoft::WRL::ComPtr;
  using namespace workrave::utils;
  HRESULT initialized = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  if (FAILED(initialized) && initialized != RPC_E_CHANGED_MODE)
    {
      return;
    }
  try
    {
      ComPtr<IWICImagingFactory> factory;
      if (SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory))))
        {
          constexpr std::array names{"timer-micro-break.png", "timer-rest-break.png", "timer-daily.png"};
          auto theme = GUIConfig::icon_theme()();
          for (size_t i = 0; i < icons.size(); ++i)
            {
              auto &icon = icons.at(i);
              if (icon != nullptr)
                {
                  DestroyIcon(icon);
                  icon = nullptr;
                }
              std::string path;
              if (theme.empty() || !AssetPath::complete_directory(theme + "/" + names.at(i), SearchPathId::Images, path))
                {
                  AssetPath::complete_directory(names.at(i), SearchPathId::Images, path);
                }
              icon = load_timer_icon(factory.Get(), wide(path));
            }
        }
    }
  catch (const std::exception &error)
    {
      spdlog::warn("Cannot load taskbar applet icons: {}", error.what());
    }
  if (SUCCEEDED(initialized))
    {
      CoUninitialize();
    }
}
void
WindowsTaskbarApplet::init_preferences()
{
  using namespace ui::prefwidgets;
  auto &enabled = GUIConfig::timerbox_enabled(timerbox_name);
  auto grouping = Choice::create(N_("Timer grouping"))
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
  auto timers = Frame::create(N_("Timers"));
  constexpr std::array labels{N_("Micro-break"), N_("Rest break"), N_("Daily limit")};
  for (auto id: break_ids)
    {
      auto visibility = Choice::create(labels.at(static_cast<size_t>(id)))
                          ->assign({N_("Show"), N_("Show only when this timer is first due"), N_("Hide")})
                          ->connect(&GUIConfig::timerbox_flags(timerbox_name, id), [id] { return timer_visibility_value(id); })
                          ->on_save([id](int value) { set_timer_visibility(id, value); })
                          ->when(&enabled);
      *timers << visibility;
    }
  preferences = PanelDef::create("ui", "applet", N_("Applet"))
                << (Box::create()->spacing(6)
                    << (Frame::create(N_("Taskbar")) << Toggle::create(N_("Show timers in taskbar"))->connect(&enabled))
                    << (Frame::create(N_("Display")) << grouping
                                                     << Value::create(N_("Cycle time (seconds)"), 1, 999)
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
  RECT current{};
  if (IsWindow(geometry.taskbar) == FALSE || GetWindowRect(geometry.taskbar, &current) == FALSE
      || EqualRect(&current, &geometry.bounds) == FALSE
      || std::chrono::steady_clock::now() - geometry.measured > std::chrono::seconds(5))
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
          dpi = next_dpi;
          font_height = next_font_height;
          if (font != nullptr)
            {
              DeleteObject(font);
            }
          font = CreateFontW(-font_height,
                             0,
                             0,
                             0,
                             FW_NORMAL,
                             FALSE,
                             FALSE,
                             FALSE,
                             DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY,
                             DEFAULT_PITCH,
                             L"Segoe UI");
        }
      POINT point{.x = *x, .y = current.top + scale(2)};
      ScreenToClient(geometry.taskbar, &point);
      SetWindowPos(window, HWND_TOP, point.x, point.y, width, height - scale(4), SWP_NOACTIVATE | SWP_SHOWWINDOW);
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
  window = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_LAYERED,
                           window_class,
                           L"Workrave timers",
                           WS_POPUP,
                           0,
                           0,
                           0,
                           0,
                           nullptr,
                           nullptr,
                           GetModuleHandleW(nullptr),
                           this);
  if (window == nullptr)
    {
      return false;
    }
  SetWindowLongPtrW(window, GWL_STYLE, WS_CHILD | WS_CLIPSIBLINGS);
  SetLastError(ERROR_SUCCESS);
  auto *old_parent = SetParent(window, taskbar);
  if ((old_parent == nullptr && GetLastError() != ERROR_SUCCESS) || GetParent(window) != taskbar)
    {
      spdlog::warn("Cannot attach Workrave taskbar applet: {}", GetLastError());
      detach();
      return false;
    }
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
  auto *old_font = SelectObject(dc, font != nullptr ? font : GetStockObject(DEFAULT_GUI_FONT));
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
  SelectObject(dc, old_font);
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
                 icons.at(index),
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
          target = CreatePopupMenu();
          // Win32 stores handles and pointers in integer-sized message fields.
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(target), wide(node->get_dynamic_text_no_accel()).c_str());
        }
      for (const auto &child: container->get_children())
        {
          append_menu(target, child, commands);
        }
    }
  else if (std::dynamic_pointer_cast<menus::SeparatorNode>(node))
    {
      AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
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
      AppendMenuW(menu, flags, commands.size(), wide(node->get_dynamic_text_no_accel()).c_str());
    }
}

void
WindowsTaskbarApplet::show_menu(POINT point)
{
  std::vector<menus::Node::Ptr> commands;
  HMENU menu = CreatePopupMenu();
  for (const auto &node: context->get_menu_model()->get_root()->get_children())
    {
      append_menu(menu, node, commands);
    }
  SetForegroundWindow(observer);
  UINT command = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, observer, nullptr);
  PostMessageW(observer, WM_NULL, 0, 0);
  DestroyMenu(menu);
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
                PAINTSTRUCT ps{};
                HDC dc = BeginPaint(hwnd, &ps);
                RECT rect{};
                GetClientRect(hwnd, &rect);
                // Buffered painting avoids flicker during the one-second refresh.
                HDC buffer = CreateCompatibleDC(dc);
                HBITMAP bitmap = CreateCompatibleBitmap(dc, std::max(1L, rect.right), std::max(1L, rect.bottom));
                auto *old = SelectObject(buffer, bitmap);
                self->paint(buffer, rect);
                BitBlt(dc, 0, 0, rect.right, rect.bottom, buffer, 0, 0, SRCCOPY);
                SelectObject(buffer, old);
                DeleteObject(bitmap);
                DeleteDC(buffer);
                EndPaint(hwnd, &ps);
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
