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

#include <QtGui>
#include <QStyle>
#include <QDesktopWidget>
#include <QApplication>

#ifdef PLATFORM_OS_OSX
#include <QtMacExtras>
#endif

#include "debug.hh"
#include "commonui/nls.h"

#include "commonui/Backend.hh"
#include "core/ICore.hh"
#include "session/System.hh"
#include "utils/AssetPath.hh"

#include "UiUtil.hh"

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
    block_window(NULL)
{
  TRACE_ENTER("BreakWindow::BreakWindow");
#ifdef PLATFORM_OS_OSX
  priv = std::make_shared<Private>();
#endif
  TRACE_EXIT();
}


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
      frame->set_frame_style(Frame::Style::Solid);
      frame->set_frame_width(6, 6);
      frame->set_frame_visible(false);

      QVBoxLayout *frame_layout = new QVBoxLayout;
      frame->setLayout(frame_layout);
      frame_layout->addWidget(gui);

      layout->addWidget(frame);
    }
}

BreakWindow::~BreakWindow()
{
  if (frame != NULL)
    {
      frame->set_frame_flashing(0);
    }
}

void
BreakWindow::center()
{
  QDesktopWidget *dw = QApplication::desktop();
  setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), dw->screenGeometry(screen)));
}

void
BreakWindow::add_lock_button(QLayout *box)
{
  if (System::is_lockable())
    {
      QPushButton *button = UiUtil::create_image_text_button("lock.png", tr("Lock"));
      box->addWidget(button);
      connect(button, &QPushButton::click, this, &BreakWindow::on_lock_button_clicked);
    }
}

void
BreakWindow::add_shutdown_button(QLayout *box)
{
  if (false) // FIXME: System::is_shutdown_supported())
    {
      QPushButton *button = UiUtil::create_image_text_button("shutdown.png", tr("Shut down"));
      box->addWidget(button);
      connect(button, &QPushButton::clicked, this, &BreakWindow::on_shutdown_button_clicked);
    }
}

void
BreakWindow::add_skip_button(QLayout *box)
{
  if ((break_flags & BREAK_FLAGS_SKIPPABLE) != 0)
    {
      QPushButton *button = new QPushButton(tr("Skip"));
      box->addWidget(button);
      connect(button, &QPushButton::clicked, this, &BreakWindow::on_skip_button_clicked);
    }
}

void
BreakWindow::add_postpone_button(QLayout *box)
{
  if ((break_flags & BREAK_FLAGS_POSTPONABLE) != 0)
    {
      QPushButton *button = new QPushButton(tr("Postpone"));
      box->addWidget(button);
      connect(button, &QPushButton::clicked, this, &BreakWindow::on_postpone_button_clicked);
    }
}

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


void
BreakWindow::on_postpone_button_clicked()
{
  ICore::Ptr core = Backend::get_core();
  IBreak::Ptr b = core->get_break(break_id);

  b->postpone_break();
  resume_non_ignorable_break();
}

void
BreakWindow::on_skip_button_clicked()
{
  ICore::Ptr core = Backend::get_core();
  IBreak::Ptr b = core->get_break(break_id);

  b->skip_break();
  resume_non_ignorable_break();
}

// TODO: move to backend.
void
BreakWindow::resume_non_ignorable_break()
{
  TRACE_ENTER("BreakWindow::resume_non_ignorable_break");
  ICore::Ptr core = Backend::get_core();
  OperationMode mode = core->get_operation_mode();

  TRACE_MSG("break flags " << break_flags);

  if (! (break_flags & BREAK_FLAGS_USER_INITIATED) && mode == OperationMode::Normal)
    {
      for (int id = break_id - 1; id >= 0; id--)
        {
          TRACE_MSG("Break " << id << ": check ignorable");

          bool ignorable = GUIConfig::break_ignorable(BreakId(id))();
          if (!ignorable)
            {
              TRACE_MSG("Break " << id << " not ignorable");

              ICore::Ptr core = Backend::get_core();
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

QHBoxLayout *
BreakWindow::create_break_buttons(bool lockable, bool shutdownable)
{
  QHBoxLayout *box = NULL;

  if ((break_flags != BREAK_FLAGS_NONE) || lockable || shutdownable)
    {
      box = new QHBoxLayout;

      if (shutdownable)
        {
          add_shutdown_button(box);
        }

      if (lockable)
        {
          add_lock_button(box);
        }

      add_skip_button(box);
      add_postpone_button(box);
    }

  return box;
}

#ifdef PLATFORM_OS_OSX

NSString * colorToHexString (NSColor * color) {
  NSMutableString * hexString = [[NSMutableString alloc] init];
  [hexString appendString:@"#"];
  [hexString appendFormat:@"%02x", (int)([color redComponent] * 255.0f)];
  [hexString appendFormat:@"%02x", (int)([color greenComponent] * 255.0f)];
  [hexString appendFormat:@"%02x", (int)([color blueComponent] * 255.0f)];
  return hexString;
}

#endif

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

      // NSURL *desktopImageURL = [mainWorkspace desktopImageURLForScreen:desktopScreen];
      // NSImage *desktopImage = [[NSImage alloc] initWithContentsOfURL:desktopImageURL];
      // //desktopImage = [desktopImage imageCroppedToFitSize:[nsview bounds].size]

      // CGImageRef cgImage = [desktopImage CGImageForProposedRect:NULL context:NULL hints:NULL];
      // QPixmap pixmap = QtMac::fromCGImageRef(cgImage);

      // QPalette palette;
      // palette.setBrush(block_window->backgroundRole(), QBrush(pixmap));
      // block_window->setPalette(palette);

      block_window->showFullScreen();
      block_window->raise();

      NSApplicationPresentationOptions options =
        (NSApplicationPresentationHideDock |
         NSApplicationPresentationHideMenuBar |
         NSApplicationPresentationDisableAppleMenu |
         NSApplicationPresentationDisableProcessSwitching |
         // NSApplicationPresentationDisableForceQuit |
         // NSApplicationPresentationDisableSessionTermination |
         NSApplicationPresentationDisableHideApplication);
      [NSApp setPresentationOptions:options];

      NSDictionary * dictionary = [mainWorkspace desktopImageOptionsForScreen: desktopScreen];
      //if ([dictionary objectForKey:NSWorkspaceDesktopImageAllowClippingKey])
      //{
          NSNumber * clipping = [dictionary objectForKey:NSWorkspaceDesktopImageAllowClippingKey];
          if ([clipping boolValue]) {
            printf("Clipping: YES\n");
          } else {
            printf("Clipping: NO\n");
          }
          //} else {
          //printf("Clipping: NO (not defined)\n");
          //}
      if ([dictionary objectForKey:NSWorkspaceDesktopImageFillColorKey]) {
        NSColor * color = [dictionary objectForKey:NSWorkspaceDesktopImageFillColorKey];
        printf("Background: %s\n", [colorToHexString(color) UTF8String]);
      }
      if ([dictionary objectForKey:NSWorkspaceDesktopImageScalingKey]) {
        NSImageScaling scaling = (NSImageScaling) [[dictionary objectForKey:NSWorkspaceDesktopImageScalingKey] integerValue];
        switch (scaling) {
        case NSImageScaleNone:
          printf("Scaling: none\n");
          break;
        case NSImageScaleAxesIndependently:
          printf("Scaling: stretch\n");
          break;
        case NSImageScaleProportionallyDown:
          printf("Scaling: down\n");
          break;
        case NSImageScaleProportionallyUpOrDown:
          printf("Scaling: updown\n");
          break;
        default:
          printf("Scaling: unknown\n");
          break;
        }
      } else {
        printf("Scaling: updown (not defined)\n");
      }

      /* fit screen
        Clipping: NO (not defined)
        Background: #51a0aa
        Scaling: updown (not defined)
        
        fit to screen
        Clipping: NO
        Background: #51a0aa
        Scaling: updown

        stretch to fiull screen
        Clipping: NO (not defined)
        Background: #51a0aa
        Scaling: stretch

        center
        Clipping: NO (not defined)
        Background: #51a0aa
        Scaling: none

        tile
        Clipping: NO (not defined)
        Background: #51a0aa
        Scaling: updown (not defined)
      */
    
#endif
    }
  // Set window hints.
  // set_skip_pager_hint(true);
  // set_skip_taskbar_hint(true);
  // WindowHints::set_always_on_top(this, true);
  raise();

  TRACE_EXIT();
}

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

void
BreakWindow::refresh()
{
  TRACE_ENTER("BreakWindow::refresh");
  update_break_window();

  ICore::Ptr core = Backend::get_core();
  bool user_active = core->is_user_active();
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
