// BreakWindow.cc --- base class for the break windows
//
// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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
#include "config.h"
#endif

#include "BreakWindow.hh"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/make_shared.hpp>

#include <QtGui>
#include <QStyle>
#include <QDesktopWidget>
#include <QApplication>
#include <QtMacExtras>

#include "debug.hh"
#include "nls.h"

// #include "config/IConfigurator.hh"

// #include "GUI.hh"
// #include "IBreak.hh"
// #include "GtkUtil.hh"
// #include "WindowHints.hh"
// #include "Frame.hh"
#include "System.hh"
#include "utils/AssetPath.hh"
#include "ICore.hh"
#include "CoreFactory.hh"

// #if defined(PLATFORM_OS_WIN32)
// #include "DesktopWindow.hh"
// #elif defined(PLATFORM_OS_UNIX)
// #include "desktop-window.h"
// #endif

#ifdef PLATFORM_OS_OSX
#import <Cocoa/Cocoa.h>

class BreakWindow::Private
{
public:
  NSRunningApplication *active_app;

public:
  Private() : active_app(nil)
  {
  }
};
#endif

using namespace workrave;
using namespace workrave::utils;

//! Constructor
/*!
 *  \param control The controller.
 */
BreakWindow::BreakWindow(int screen,
                         BreakId break_id,
                         BreakFlags break_flags,
                         GUIConfig::BlockMode mode)
  : QWidget(0, Qt::Window
            | Qt::WindowStaysOnTopHint
            | Qt::FramelessWindowHint),
    break_id(break_id),
    screen(screen),
    block_mode(mode),
    break_flags(break_flags),
    frame(NULL),
    is_flashing(false),
    gui(NULL),
    postpone_button(NULL),
    skip_button(NULL),
    lock_button(NULL),
    shutdown_button(NULL),
    block_window(NULL)
{
  TRACE_ENTER("BreakWindow::BreakWindow");
  priv = boost::make_shared<Private>();
  TRACE_EXIT();
}


//! Init GUI
void
BreakWindow::init()
{
  gui = create_gui();

  // if (mode != GUIConfig::BLOCK_MODE_NONE)
  //   {
  //     // Disable titlebar to appear like a popup
  //     set_decorated(false);
  //     set_skip_taskbar_hint(true);
  //     set_skip_pager_hint(true);
  //     window->set_functions(Gdk::FUNC_MOVE);
  //   }

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(1, 1, 1, 1);
  setLayout(layout);

  if (block_mode == GUIConfig::BLOCK_MODE_NONE)
    {
      layout->addWidget(gui);
    }
  else
    {
      frame = new Frame;
      frame->set_frame_style(Frame::STYLE_SOLID);
      frame->set_frame_width(6, 6);
      frame->set_frame_visible(false);

      QVBoxLayout *frameLayout = new QVBoxLayout;
      frame->setLayout(frameLayout);
      frameLayout->addWidget(gui);

      layout->addWidget(frame);
    }
}


//! Destructor.
BreakWindow::~BreakWindow()
{
  TRACE_ENTER("BreakWindow::~BreakWindow");

  if (frame != NULL)
    {
      frame->set_frame_flashing(0);
    }

  TRACE_EXIT();
}


//! Centers the window.
void
BreakWindow::center()
{
  qDebug() << "c: " << size();

  QDesktopWidget *dw = QApplication::desktop();
  const QRect rect = dw->screenGeometry(screen);

  const QRect arect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), rect);
  qDebug() << "a: " << arect;
  setGeometry(arect);
}


//! Creates the lock button
QAbstractButton *
BreakWindow::create_lock_button()
{
  QPushButton *button = NULL;
  if (System::is_lockable())
    {
      std::string file = AssetPath::complete_directory("lock.png", AssetPath::SEARCH_PATH_IMAGES);
      QPixmap pixmap(file.c_str());
      QIcon icon(pixmap);

      button = new QPushButton; 
      button->setIcon(icon);
      button->setIconSize(pixmap.rect().size());

      connect(button, &QPushButton::click, this, &BreakWindow::on_lock_button_clicked);
    }
  return button;
}

//! Creates the lock button
QAbstractButton *
BreakWindow::create_shutdown_button()
{
  QPushButton *button = NULL;
  if (false) // FIXME: System::is_shutdown_supported())
    {
      std::string file = AssetPath::complete_directory("shutdown.png", AssetPath::SEARCH_PATH_IMAGES);
      QPixmap pixmap(file.c_str());
      QIcon icon(pixmap);

      button = new QPushButton(_("Shut down"));
      button->setIcon(icon);
      button->setIconSize(pixmap.rect().size());

      connect(button, &QPushButton::clicked, this, &BreakWindow::on_shutdown_button_clicked);
    }
  return button;
}


//! Creates the skip button.
QAbstractButton *
BreakWindow::create_skip_button()
{
  QPushButton *button = new QPushButton(_("Skip"));
  connect(button, &QPushButton::clicked, this, &BreakWindow::on_skip_button_clicked);
  return button;
}


//! Creates the postpone button.
QAbstractButton *
BreakWindow::create_postpone_button()
{
  QPushButton *button = new QPushButton(_("Postpone"));
  connect(button, &QPushButton::clicked, this, &BreakWindow::on_postpone_button_clicked);
  return button;
}


//! The lock button was clicked.
void
BreakWindow::on_lock_button_clicked()
{
  if (System::is_lockable())
    {
      // IGUI *gui = GUI::get_instance();
      // gui->interrupt_grab();
      // FIXME: System::lock();
    }
}

//! The lock button was clicked.
void
BreakWindow::on_shutdown_button_clicked()
{
  // IGUI *gui = GUI::get_instance();
  // gui->interrupt_grab();

  // FIXME: System::shutdown();
}


//! User has closed the main window.
// bool
// BreakWindow::on_delete_event(GdkEventAny *)
// {
//   if (block_mode == GUIConfig::BLOCK_MODE_NONE)
//     {
//       on_postpone_button_clicked();
//     }
//   return TRUE;
// }


//! The postpone button was clicked.
void
BreakWindow::on_postpone_button_clicked()
{
  TRACE_ENTER("BreakWindow::on_postpone_button_clicked");
  ICore::Ptr core = CoreFactory::get_core();
  IBreak::Ptr b = core->get_break(break_id);

  b->postpone_break();
  resume_non_ignorable_break();

  TRACE_EXIT();
}



//! The skip button was clicked.
void
BreakWindow::on_skip_button_clicked()
{
  TRACE_ENTER("BreakWindow::on_postpone_button_clicked");
  ICore::Ptr core = CoreFactory::get_core();
  IBreak::Ptr b = core->get_break(break_id);

  b->skip_break();
  resume_non_ignorable_break();

  TRACE_EXIT();
}

// TODO: move to backend.
void
BreakWindow::resume_non_ignorable_break()
{
  TRACE_ENTER("BreakWindow::resume_non_ignorable_break");
  ICore::Ptr core = CoreFactory::get_core();
  OperationMode mode = core->get_operation_mode();

  TRACE_MSG("break flags " << break_flags);

  if (! (break_flags & BREAK_FLAGS_USER_INITIATED) &&
      mode == OperationMode::Normal)
    {
      for (int id = break_id - 1; id >= 0; id--)
        {
          TRACE_MSG("Break " << id << ": check ignorable");

          bool ignorable = GUIConfig::break_ignorable(BreakId(id))();
          if (!ignorable)
            {
              TRACE_MSG("Break " << id << " not ignorable");

              ICore::Ptr core = CoreFactory::get_core();
              IBreak::Ptr b = core->get_break(BreakId(id));

              if (b->get_elapsed_time() > b->get_limit())
                {
                  TRACE_MSG("Break " << id << " not ignorable and overdue");

                  core->force_break(BreakId(id), BREAK_HINT_NONE);
                  break;
                }
            }
        }
    }
}

//! Control buttons.
QHBoxLayout *
BreakWindow::create_break_buttons(bool lockable,
                                  bool shutdownable)
{
  QHBoxLayout *box = NULL;

  if ((break_flags != BREAK_FLAGS_NONE) || lockable || shutdownable)
    {
      box = new QHBoxLayout;

      if (shutdownable)
        {
          shutdown_button = create_shutdown_button();
          if (shutdown_button != NULL)
            {
              box->addWidget(shutdown_button);
            }
        }

      if (lockable)
        {
          lock_button = create_lock_button();
          if (lock_button != NULL)
            {
              box->addWidget(lock_button);
            }
        }

      if ((break_flags & BREAK_FLAGS_SKIPPABLE) != 0)
        {
          skip_button = create_skip_button();
          box->addWidget(skip_button);
        }

      if ((break_flags & BREAK_FLAGS_POSTPONABLE) != 0)
        {
          postpone_button = create_postpone_button();
          box->addWidget(postpone_button);
        }
    }

  return box;
}


//! Starts the daily limit.
void
BreakWindow::start()
{
  TRACE_ENTER("BreakWindow::start");

#ifdef PLATFORM_OS_OSX
  NSLog(@"%@", [[NSWorkspace sharedWorkspace] frontmostApplication].bundleIdentifier);
  priv->active_app = [[NSWorkspace sharedWorkspace] frontmostApplication];
  [NSApp activateIgnoringOtherApps:YES];
#endif

  refresh();
  show();
  center();

  if (block_mode != GUIConfig::BLOCK_MODE_NONE)
    {
      block_window = new QWidget();
      block_window->setParent(0);
      block_window->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
      block_window->setAutoFillBackground(true);
      block_window->setPalette(QPalette(Qt::black));
      block_window->setWindowOpacity(block_mode == GUIConfig::BLOCK_MODE_INPUT ? 0.2: 1);
      // block_window->setAttribute(Qt::WA_PaintOnScreen);

#ifdef PLATFORM_OS_OSX

      NSWorkspace *mainWorkspace = [NSWorkspace sharedWorkspace];
      NSView *nsview = (__bridge NSView *)reinterpret_cast<void *>(block_window->winId());
      NSWindow *nswindow = [nsview window];
      //[NSScreen screens] objectAtIndex:0]]
      NSScreen *desktopScreen = [nswindow screen];
    
      NSURL *desktopImageURL = [mainWorkspace desktopImageURLForScreen:desktopScreen];
      NSImage *desktopImage = [[NSImage alloc] initWithContentsOfURL:desktopImageURL];
      //desktopImage = [desktopImage imageCroppedToFitSize:[nsview bounds].size]

      CGImageRef cgImage = [desktopImage CGImageForProposedRect:NULL context:NULL hints:NULL];
      QPixmap pixmap = QtMac::fromCGImageRef(cgImage);

      QPalette palette;
      palette.setBrush(block_window->backgroundRole(), QBrush(pixmap));
      block_window->setPalette(palette);

      block_window->showFullScreen();
      block_window->raise();

      NSApplicationPresentationOptions options =
        (NSApplicationPresentationHideDock |
         NSApplicationPresentationHideMenuBar |
         NSApplicationPresentationDisableAppleMenu |
         NSApplicationPresentationDisableProcessSwitching |
         NSApplicationPresentationDisableForceQuit |
         NSApplicationPresentationDisableSessionTermination |
         NSApplicationPresentationDisableHideApplication);
      [NSApp setPresentationOptions:options];
#endif
    }
  // Set window hints.
  // set_skip_pager_hint(true);
  // set_skip_taskbar_hint(true);
  // WindowHints::set_always_on_top(this, true);

    // Alternative 1:

    // int windowLevel = CGShieldingWindowLevel();
    // NSRect windowRect = [[NSScreen mainScreen] frame];
    // NSWindow *overlayWindow = [[NSWindow alloc]
    //                            initWithContentRect:windowRect
    //                            styleMask:NSBorderlessWindowMask
    //                            backing:NSBackingStoreBuffered
    //                            defer:NO
    //                            screen:[NSScreen mainScreen]];

    // [overlayWindow setReleasedWhenClosed:YES];
    // [overlayWindow setLevel:windowLevel];
    // [overlayWindow setBackgroundColor:[NSColor colorWithCalibratedRed:0.0
    //                                    green:0.0
    //                                    blue:0.0
    //                                    alpha:0.5]];

    // [overlayWindow setAlphaValue:1.0];
    // [overlayWindow setOpaque:NO];
    // [overlayWindow setIgnoresMouseEvents:NO];
    // [overlayWindow makeKeyAndOrderFront:nil];

    //  [self.window addChildWindow:overlayWindow ordered:NSWindowAbove];

    // Alternative 2:

    // NSView *view = [[NSView alloc] initWithFrame:CGRectZero];
    // NSDictionary *options = @{NSFullScreenModeAllScreens: @(YES),
    //                           NSFullScreenModeWindowLevel: @(NSScreenSaverWindowLevel)};
    // [view enterFullScreenMode:[NSScreen mainScreen] withOptions:options];



  raise();

  TRACE_EXIT();
}


//! Stops the daily limit.
void
BreakWindow::stop()
{
  TRACE_ENTER("BreakWindow::stop");

  if (frame != NULL)
    {
      frame->set_frame_flashing(0);
    }

  if (block_window != NULL)
    {
      block_window->hide();

#ifdef PLATFORM_OS_OSX
      [NSApp setPresentationOptions: NSApplicationPresentationDefault];
#endif
    }

  hide();

#ifdef PLATFORM_OS_OSX
  if (priv->active_app != nil)
    {
      [priv->active_app activateWithOptions:NSApplicationActivateIgnoringOtherApps];
      priv->active_app = nil;
    }
#endif

  TRACE_EXIT();
}


//! Refresh
void
BreakWindow::refresh()
{
  TRACE_ENTER("BreakWindow::refresh");
  update_break_window();

  ICore::Ptr core = CoreFactory::get_core();
  bool user_active = core->is_user_active();
  Frame *frame = get_frame();
  if (frame != NULL)
    {
      if (user_active && !is_flashing)
        {
          frame->set_frame_color(QColor("orange"));
          frame->set_frame_visible(true);
          frame->set_frame_flashing(500);
          is_flashing = true;
        }
      else if (!user_active && is_flashing)
        {
          frame->set_frame_flashing(0);
          frame->set_frame_visible(false);
          is_flashing = false;
        }
    }

  TRACE_EXIT();
}

void
BreakWindow::update_break_window()
{
}
