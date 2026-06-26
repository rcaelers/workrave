// Copyright (C) 2024 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "MacOSBlockingOverlay.hh"

MacOSBlockingOverlay::MacOSBlockingOverlay(QScreen *screen)
  : screen(screen)
{
}

MacOSBlockingOverlay::~MacOSBlockingOverlay()
{
  delete overlay;
}

void
MacOSBlockingOverlay::start()
{
  if (overlay == nullptr)
    {
      overlay = new QWidget(nullptr,
                            Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SplashScreen);
      overlay->setAutoFillBackground(true);
      overlay->setPalette(QPalette(Qt::black));
      overlay->setWindowOpacity(0.8);
    }
  overlay->setGeometry(screen->geometry());
  overlay->show();
  overlay->raise();
}

void
MacOSBlockingOverlay::stop()
{
  if (overlay != nullptr)
    {
      overlay->hide();
    }
}
