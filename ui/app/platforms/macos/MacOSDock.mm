// Copyright (C) 2014 Rob Caelers <robc@krandor.org>
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

#include "ui/macos/MacOSDock.hh"

#import <AppKit/NSApplication.h>
#import <AppKit/NSBezierPath.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSDockTile.h>
#import <AppKit/NSImage.h>
#import <Foundation/NSString.h>

#include "utils/AssetPath.hh"
#include "core/ICore.hh"

using namespace workrave;
using namespace workrave::utils;

@interface DockTileBackground: NSView
{
  DockPrivate *p;
  NSImage *backgroundImage;
}
- (id)initWithFrame:(NSRect)frame parent:(DockPrivate*)parent;
- (void)setBackground:(NSImage *)image;
@end

@interface DockTileTimers: NSView
{
  DockPrivate *p;
}
- (id)initWithFrame:(NSRect)frame parent:(DockPrivate*)parent;
@end

@interface DockTileView : NSView
{
  DockPrivate *p;
  DockTileBackground *dockTileBackground;
  DockTileTimers *dockTileTimers;
}
- (id)initWithParent:(DockPrivate*)parent;
- (void)destroy;
- (void)cleanup;
- (void)setBackground:(NSImage *)image;
@end

@implementation DockTileBackground
- (id)initWithFrame:(NSRect)frame parent:(DockPrivate*)parent
{
  self = [super initWithFrame:frame];

  if (self != nil)
    {
      p = parent;
    }

  return self;
}

- (void)drawRect:(NSRect)aRect;
{
  NSRect boundary = [self bounds];
  [backgroundImage drawInRect:boundary fromRect:NSZeroRect operation:NSCompositingOperationCopy fraction:1.0];
}

- (void)setBackground:(NSImage *)image
{
  backgroundImage = image;
  [[NSApp dockTile] display];
}
@end

@implementation DockTileTimers
- (id)initWithFrame:(NSRect)frame parent:(DockPrivate*)parent
{
  self = [super initWithFrame:frame];

  if (self != nil)
    {
      p = parent;
    }

  return self;
}

- (void)drawRect:(NSRect)aRect;
{
}
@end

@implementation DockTileView
- (id)initWithParent:(DockPrivate*)parent
{
  self = [super init];

  if (self != nil)
    {
      p = parent;

      NSDockTile *dock = [[NSApplication sharedApplication] dockTile];
      [dock setContentView: self];

      CGRect dockRect = CGRectMake(0, 0, dock.size.width, dock.size.height);

      dockTileBackground = [[DockTileBackground alloc] initWithFrame:NSRectFromCGRect(dockRect) parent:p];
      [self addSubview: dockTileBackground];

      dockTileTimers = [[DockTileTimers alloc] initWithFrame:NSRectFromCGRect(dockRect) parent:p];
      [self addSubview: dockTileTimers];
    }

  return self;
}

- (void)destroy
{
  NSDockTile *dock = [[NSApplication sharedApplication] dockTile];
  [dock setContentView: nil];

  [self cleanup];
}

- (void)cleanup
{
  if (dockTileBackground != nil)
    {
      [dockTileBackground removeFromSuperview];
      // [dockTileBackground release];
      dockTileBackground = nil;
    }
  if (dockTileTimers != nil)
    {
      [dockTileTimers removeFromSuperview];
      // [dockTileTimers release];
      dockTileTimers = nil;
    }
}

- (void)setBackground:(NSImage *)image
{
  [dockTileBackground setBackground:image];
}
@end

class DockPrivate
{
public:
  DockPrivate()
  {
    dockTile = [[DockTileView alloc] initWithParent:this];
  }

  inline ~DockPrivate()
  {
    [dockTile destroy];
    //[dockTile release];
  }

  DockTileView *dockTile;
};

MacOSDock::MacOSDock(std::shared_ptr<IApplicationContext> app)
: app(app)
{
  priv = std::make_shared<DockPrivate>();

  std::string filename = AssetPath::complete_directory("workrave.png", SearchPathId::Images);
  NSImage *icon = [[NSImage alloc] initWithContentsOfFile: [NSString stringWithCString:filename.c_str() encoding:[NSString defaultCStringEncoding]]];

  [priv->dockTile setBackground:icon];

  auto core = app->get_core();
  workrave::utils::connect(core->signal_operation_mode_changed(), this, [this](auto mode) { on_operation_mode_changed(mode); });
  OperationMode mode = core->get_regular_operation_mode();
  on_operation_mode_changed(mode);
}

void
MacOSDock::on_operation_mode_changed(OperationMode m)
{
  std::string name;
  switch(m)
    {
    case workrave::OperationMode::Normal:
      break;
    case workrave::OperationMode::Suspended:
      break;
    case workrave::OperationMode::Quiet:
      break;
    }
}
