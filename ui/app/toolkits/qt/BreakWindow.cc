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
#  include "config.h"
#endif

#include "BreakWindow.hh"

#include <memory>

#include <QtGui>
#include <QStyle>
#include <QApplication>
#include <utility>

#include "core/ICore.hh"
#include "utils/AssetPath.hh"
#include "utils/Platform.hh"

#include "debug.hh"
#include "UiUtil.hh"
#include "IToolkitPrivate.hh"

using namespace workrave;
using namespace workrave::utils;

BreakWindow::BreakWindow(std::shared_ptr<IApplicationContext> app, QScreen *screen, BreakId break_id, BreakFlags break_flags)
  : QWidget(nullptr, Qt::Window)
  , app(std::move(app))
  , break_id(break_id)
  , break_flags(break_flags)
  , screen(screen)
{
  TRACE_ENTRY();
  block_mode = GUIConfig::block_mode()();
}

void
BreakWindow::init()
{
#if defined(HAVE_WAYLAND)
  if (Platform::running_on_wayland())
    {
      auto wm = std::make_shared<WaylandWindowManager>();
      bool success = wm->init();
      if (success)
        {
          window_manager = wm;
        }
    }
#endif

  if (block_mode != BlockMode::Off)
    {
      setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::SplashScreen);
    }

  auto *outer_layout = new QVBoxLayout();
  outer_layout->setContentsMargins(1, 1, 1, 1);
  setLayout(outer_layout);

  QVBoxLayout *inner_layout = outer_layout;

  if (block_mode != BlockMode::Off)
    {
      frame = new Frame;
      frame->set_frame_style(Frame::Style::Solid);
      frame->set_frame_width(6, 6);
      frame->set_frame_visible(false);

      auto *frame_layout = new QVBoxLayout;
      frame->setLayout(frame_layout);

      outer_layout->addWidget(frame);
      inner_layout = frame_layout;
    }

  size_group = std::make_shared<SizeGroup>(Qt::Horizontal);
  gui = create_gui();

  inner_layout->addWidget(gui);
  inner_layout->addLayout(create_break_buttons(true, true));
}

BreakWindow::~BreakWindow()
{
  if (frame != nullptr)
    {
      frame->set_frame_flashing(0);
    }
}

void
BreakWindow::center()
{
  QRect geometry = screen->geometry();
  setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), geometry));
}

void
BreakWindow::add_lock_button(QGridLayout *box) const
{
  if (System::is_lockable())
    {
      QPushButton *button = UiUtil::create_image_text_button("lock.png", tr("Lock"));
      box->addWidget(button, 1, box->columnCount());
      connect(button, &QPushButton::click, this, &BreakWindow::on_lock_button_clicked);
    }
}

// void
// BreakWindow::add_shutdown_button(QGridLayout *box)
// {
//   if (System::is_shutdown_supported())
//     {
//       QPushButton *button = UiUtil::create_image_text_button("shutdown.png", tr("Shut down"));
//       box->addWidget(button, 1, box->columnCount());
//       connect(button, &QPushButton::clicked, this, &BreakWindow::on_shutdown_button_clicked);
//     }
// }

void
BreakWindow::add_skip_button(QGridLayout *box, bool locked)
{
  if ((break_flags & BREAK_FLAGS_SKIPPABLE) != 0)
    {
      skip_button = new QPushButton(tr("Skip"));
      skip_button->setEnabled(!locked);
      if (locked)
        {
          QString msg = tr("You cannot skip this break while another non-skippable break is overdue.");
          skip_button->setToolTip(msg);
        }
      skip_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      size_group->add_widget(skip_button);
      box->addWidget(skip_button, 1, box->columnCount());
      connect(skip_button, &QPushButton::clicked, this, &BreakWindow::on_skip_button_clicked);
    }
}

void
BreakWindow::add_postpone_button(QGridLayout *box, bool locked)
{
  if ((break_flags & BREAK_FLAGS_POSTPONABLE) != 0)
    {
      postpone_button = new QPushButton(tr("Postpone"));
      postpone_button->setEnabled(!locked);
      if (locked)
        {
          QString msg = tr("You cannot skip this break while another non-skippable break is overdue.");
          postpone_button->setToolTip(msg);
        }
      postpone_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      size_group->add_widget(postpone_button);
      box->addWidget(postpone_button, 1, box->columnCount());
      connect(postpone_button, &QPushButton::clicked, this, &BreakWindow::on_postpone_button_clicked);
    }
}

void
BreakWindow::get_operation_name_and_icon(System::SystemOperation::SystemOperationType type, QString &name, QString &icon_name)
{
  switch (type)
    {
    case System::SystemOperation::SYSTEM_OPERATION_NONE:
      name = tr("Lock...");
      icon_name = "lock.png";
      break;
    case System::SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN:
      name = tr("Lock");
      icon_name = "lock.png";
      break;
    case System::SystemOperation::SYSTEM_OPERATION_SHUTDOWN:
      name = tr("Shutdown");
      icon_name = "shutdown.png";
      break;
    case System::SystemOperation::SYSTEM_OPERATION_SUSPEND:
      name = tr("Suspend");
      icon_name = "shutdown.png";
      break;
    case System::SystemOperation::SYSTEM_OPERATION_HIBERNATE:
      name = tr("Hibernate");
      icon_name = "shutdown.png";
      break;
    case System::SystemOperation::SYSTEM_OPERATION_SUSPEND_HYBRID:
      name = tr("Suspend hybrid");
      icon_name = "shutdown.png";
      break;
    };
}

void
BreakWindow::append_row_to_sysoper_model(System::SystemOperation::SystemOperationType type)
{
  TRACE_ENTRY();
  QString name;
  QString icon_name;
  get_operation_name_and_icon(type, name, icon_name);

  std::string file = AssetPath::complete_directory(icon_name.toStdString(), SearchPathId::Images);
  QPixmap pixmap(file.c_str());
  QIcon icon(pixmap);
  sysoper_combo->addItem(icon, name);
}

void
BreakWindow::add_sysoper_combobox(QGridLayout *box)
{
  TRACE_ENTRY();
  supported_system_operations = System::get_supported_system_operations();

  if (supported_system_operations.empty())
    {
      return;
    }

  sysoper_combo = new QComboBox;
  append_row_to_sysoper_model(System::SystemOperation::SYSTEM_OPERATION_NONE);

  for (auto &item: supported_system_operations)
    {
      append_row_to_sysoper_model(item.type);
    }

  void (QComboBox::*signal)(int) = &QComboBox::currentIndexChanged;
  QObject::connect(sysoper_combo, signal, this, &BreakWindow::on_sysoper_combobox_changed);

  size_group->add_widget(sysoper_combo);
  box->addWidget(sysoper_combo, 1, box->columnCount());
}

void
BreakWindow::on_sysoper_combobox_changed(int index)
{
  TRACE_ENTRY_PAR(index);
  if (supported_system_operations[index].type == System::SystemOperation::SYSTEM_OPERATION_NONE)
    {
      TRACE_MSG("SYSTEM_OPERATION_NONE");
      return;
    }

  // IGUI *gui = Application::get_instance();
  // gui->interrupt_grab();

  System::execute(supported_system_operations[index].type);

  // this will fire this method again with SYSTEM_OPERATION_NONE active
  sysoper_combo->setCurrentIndex(0);
}

void
BreakWindow::on_lock_button_clicked()
{
  if (System::is_lockable())
    {
      // IGUI *gui = Application::get_instance();
      // gui->interrupt_grab();
      System::lock_screen();
    }
}

// bool
// BreakWindow::on_delete_event(GdkEventAny *)
// {
//   if (block_mode == BlockMode::Off)
//     {
//       on_postpone_button_clicked();
//     }
//   return TRUE;
// }

void
BreakWindow::on_postpone_button_clicked()
{
  auto core = app->get_core();
  auto b = core->get_break(break_id);

  b->postpone_break();
}

void
BreakWindow::on_skip_button_clicked()
{
  auto core = app->get_core();
  auto b = core->get_break(break_id);

  b->skip_break();
}

void
BreakWindow::check_skip_postpone_lock(bool &skip_locked, bool &postpone_locked, BreakId &overdue_break_id)
{
  TRACE_ENTRY();
  skip_locked = false;
  postpone_locked = false;
  overdue_break_id = BREAK_ID_NONE;

  auto core = app->get_core();
  OperationMode mode = core->get_active_operation_mode();

  if (mode == OperationMode::Normal)
    {
      for (int id = break_id - 1; id >= 0; id--)
        {
          auto b = core->get_break(BreakId(id));
          bool overdue = b->get_elapsed_time() > b->get_limit();

          if (((break_flags & BREAK_FLAGS_USER_INITIATED) == 0) || b->is_max_preludes_reached())
            {
              if (!GUIConfig::break_ignorable(BreakId(id))())
                {
                  postpone_locked = overdue;
                }
              if (!GUIConfig::break_skippable(BreakId(id))())
                {
                  skip_locked = overdue;
                }
              if (skip_locked || postpone_locked)
                {
                  overdue_break_id = BreakId(id);
                  return;
                }
            }
        }
    }
}

void
BreakWindow::update_skip_postpone_lock()
{
  if ((postpone_button != nullptr && !postpone_button->isEnabled()) || (skip_button != nullptr && !skip_button->isEnabled()))
    {
      bool skip_locked = false;
      bool postpone_locked = false;
      BreakId overdue_break_id = BREAK_ID_NONE;
      check_skip_postpone_lock(skip_locked, postpone_locked, overdue_break_id);

      if (progress_bar != nullptr)
        {
          if (overdue_break_id != BREAK_ID_NONE)
            {
              auto core = app->get_core();
              IBreak::Ptr b = core->get_break(overdue_break_id);

              progress_bar->setRange(0, b->get_auto_reset());
              progress_bar->setValue(b->get_elapsed_idle_time());
            }
          else
            {
              progress_bar->hide();
            }
        }

      if (!postpone_locked && postpone_button != nullptr)
        {
          postpone_button->setToolTip("");
          postpone_button->setEnabled(true);
        }
      if (!skip_locked && skip_button != nullptr)
        {
          skip_button->setToolTip("");
          skip_button->setEnabled(true);
        }
    }
}

auto
BreakWindow::create_break_buttons(bool lockable, bool shutdownable) -> QLayout *
{
  QGridLayout *box = nullptr;

  if ((break_flags != BREAK_FLAGS_NONE) || lockable || shutdownable)
    {
      box = new QGridLayout;
      box->setVerticalSpacing(0);

      if (lockable || shutdownable)
        {
          if (shutdownable)
            {
              add_sysoper_combobox(box);
            }
          else
            {
              add_lock_button(box);
            }
        }

      box->addWidget(new QWidget, 1, box->columnCount());
      box->setColumnStretch(box->columnCount(), 100);

      if (break_flags != BREAK_FLAGS_NONE)
        {
          bool skip_locked = false;
          bool postpone_locked = false;
          BreakId overdue_break_id = BREAK_ID_NONE;
          check_skip_postpone_lock(skip_locked, postpone_locked, overdue_break_id);

          add_skip_button(box, skip_locked);
          add_postpone_button(box, postpone_locked);

          if (skip_locked || postpone_locked)
            {
              progress_bar = new QProgressBar;

              progress_bar->setOrientation(Qt::Horizontal);
              progress_bar->setTextVisible(false);
              progress_bar->setRange(0, 30);
              progress_bar->setValue(10);
              progress_bar->update();

              update_skip_postpone_lock();

              if (skip_locked && postpone_locked)
                {
                  box->addWidget(progress_bar, 0, box->columnCount() - 2, 1, 2);
                }
              else if (skip_locked)
                {
                  box->addWidget(progress_bar, 0, box->columnCount() - 2, 1, 1);
                }
              else
                {
                  box->addWidget(progress_bar, 0, box->columnCount() - 1, 1, 1);
                }
            }
        }
    }

  return box;
}

void
BreakWindow::start()
{
  TRACE_ENTRY();
  // TODO: platform->foreground();

  refresh();
  show();
  center();

  if (block_mode != BlockMode::Off)
    {
      block_window = new QWidget();
      block_window->setParent(nullptr);
      block_window->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
      block_window->setAutoFillBackground(true);
      block_window->setPalette(QPalette(Qt::black));
      block_window->setWindowOpacity(block_mode == BlockMode::Input ? 0.2 : 1);
      // block_window->setAttribute(Qt::WA_PaintOnScreen);

      if (block_mode == BlockMode::All)
        {
          QPalette palette;
          auto toolkit_priv = std::dynamic_pointer_cast<IToolkitPrivate>(app->get_toolkit());
          QPixmap pixmap = toolkit_priv->get_desktop_image();
          palette.setBrush(block_window->backgroundRole(), QBrush(pixmap));
          block_window->setPalette(palette);
        }

      // block_window->showFullScreen();
      block_window->showMaximized();
      block_window->raise();

      // TODO:
      // platform->lock();
    }

  // Set window hints.
  // set_skip_pager_hint(true);
  // set_skip_taskbar_hint(true);
  // WindowHints::set_always_on_top(this, true);
  raise();
}

void
BreakWindow::stop()
{
  TRACE_ENTRY();
  if (frame != nullptr)
    {
      frame->set_frame_flashing(0);
    }

  if (block_window != nullptr)
    {
      block_window->showNormal();
      block_window->hide();
      delete block_window;
    }

  hide();
}

void
BreakWindow::refresh()
{
  TRACE_ENTRY();
  update_skip_postpone_lock();
  update_break_window();
  update_flashing_border();
}

void
BreakWindow::update_flashing_border()
{
  auto core = app->get_core();
  bool user_active = core->is_user_active();
  if (frame != nullptr)
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
}

void
BreakWindow::update_break_window()
{
}
