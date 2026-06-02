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
//

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#import <AppKit/AppKit.h>

#include "MacDockTile.hh"
#include "utils/AssetPath.hh"

using namespace workrave;
using namespace workrave::utils;

// ---------------------------------------------------------------------------
// MacDockTileView — the NSView set as the dock tile's content view
// ---------------------------------------------------------------------------

@interface MacDockTileView : NSView
{
  MacDockTile *tile;
  NSImage *icon;
}
- (instancetype)initWithFrame:(NSRect)frame tile:(MacDockTile *)t icon:(NSImage *)img;
@end

@implementation MacDockTileView

- (instancetype)initWithFrame:(NSRect)frame tile:(MacDockTile *)t icon:(NSImage *)img
{
  self = [super initWithFrame:frame];
  if (self != nil)
    {
      tile = t;
      icon = img;
    }
  return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
  NSRect bounds = [self bounds];
  CGFloat w = NSWidth(bounds);
  CGFloat h = NSHeight(bounds);

  // --- Icon (top ~62% of the tile) ---
  if (icon != nil)
    {
      CGFloat iconSize = h * 0.65;
      NSRect iconRect = NSMakeRect((w - iconSize) / 2.0, h - iconSize, iconSize, iconSize);
      [icon drawInRect:iconRect fromRect:NSZeroRect operation:NSCompositingOperationSourceOver fraction:1.0];
    }

  // --- Timer bars (bottom portion, 3 bars stacked bottom-up) ---
  const int kNumBars = static_cast<int>(workrave::BREAK_ID_SIZEOF);
  const CGFloat hPad = 6.0;
  const CGFloat vPad = 8.0;
  const CGFloat barH = 10.0;
  const CGFloat gap = 3.0;
  const CGFloat barW = w - 2.0 * hPad;
  const CGFloat cornerR = 3.0;

  const auto &bars = tile->bars();

  for (int i = 0; i < kNumBars; i++)
    {
      // top bar = index 0 (micro), middle = 1 (rest), bottom = 2 (daily)
      CGFloat y = vPad + static_cast<CGFloat>(kNumBars - 1 - i) * (barH + gap);
      NSRect bgRect = NSMakeRect(hPad, y, barW, barH);

      // Track background
      [[NSColor colorWithWhite:0.22 alpha:0.75] setFill];
      [[NSBezierPath bezierPathWithRoundedRect:bgRect xRadius:cornerR yRadius:cornerR] fill];

      const auto &bar = bars[static_cast<std::size_t>(i)];

      // Filled portion
      CGFloat fillW = barW * std::max(0.0, std::min(1.0, bar.remaining));
      if (fillW > 1.0)
        {
          NSRect fillRect = NSMakeRect(hPad, y, fillW, barH);
          NSColor *fillColor;
          if (bar.overdue)
            {
              fillColor = [NSColor colorWithRed:0.90 green:0.18 blue:0.18 alpha:0.95];
            }
          else if (bar.remaining < 0.20)
            {
              fillColor = [NSColor colorWithRed:1.00 green:0.58 blue:0.00 alpha:0.95];
            }
          else if (bar.remaining < 0.50)
            {
              fillColor = [NSColor colorWithRed:0.95 green:0.85 blue:0.10 alpha:0.95];
            }
          else
            {
              fillColor = [NSColor colorWithRed:0.565 green:0.933 blue:0.565 alpha:0.95]; // lightgreen #90EE90
            }
          [fillColor setFill];
          [[NSBezierPath bezierPathWithRoundedRect:fillRect xRadius:cornerR yRadius:cornerR] fill];
        }
    }
}

@end

// ---------------------------------------------------------------------------
// MacDockTilePrivate — owns the Obj-C objects
// ---------------------------------------------------------------------------

class MacDockTilePrivate
{
public:
  MacDockTilePrivate() = default;
  ~MacDockTilePrivate()
  {
    if (tileView != nil)
      {
        [[NSApp dockTile] setContentView:nil];
      }
  }

  MacDockTileView *tileView{nil};
};

// ---------------------------------------------------------------------------
// MacDockTile
// ---------------------------------------------------------------------------

MacDockTile::MacDockTile(std::shared_ptr<IApplicationContext> app)
{
  priv_ = new MacDockTilePrivate;

  NSDockTile *dock = [[NSApplication sharedApplication] dockTile];
  NSRect frame = NSMakeRect(0, 0, dock.size.width, dock.size.height);

  std::string iconPath = AssetPath::complete_directory("workrave.png", SearchPathId::Images);
  NSImage *iconImg = [[NSImage alloc] initWithContentsOfFile:[NSString stringWithUTF8String:iconPath.c_str()]];

  priv_->tileView = [[MacDockTileView alloc] initWithFrame:frame tile:this icon:iconImg];
  [dock setContentView:priv_->tileView];

  control_ = std::make_unique<TimerBoxControl>(app->get_core(), "dock", this);
}

MacDockTile::~MacDockTile()
{
  delete priv_;
}

void
MacDockTile::tick()
{
  if (control_ != nullptr)
    {
      control_->update();
    }
}

void
MacDockTile::set_time_bar(workrave::BreakId id,
                          int /*value*/,
                          TimerColorId primary_color,
                          int primary_value,
                          int primary_max,
                          TimerColorId /*secondary_color*/,
                          int /*secondary_value*/,
                          int /*secondary_max*/)
{
  auto idx = static_cast<std::size_t>(id);
  auto &bar = bars_[idx];
  bar.remaining = (primary_max > 0) ? 1.0 - (static_cast<double>(primary_value) / primary_max) : 0.0;
  bar.overdue = (primary_color == TimerColorId::Overdue || primary_color == TimerColorId::InactiveOverOverdue);
}

void
MacDockTile::update_view()
{
  [[NSApp dockTile] display];
}
