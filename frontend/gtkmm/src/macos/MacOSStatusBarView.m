#import "MacOSStatusBarView.h"


@implementation MacOSStatusBarView

- (id)initWithMenu:(NSMenu *)myMenu
{
  statusItem = [[[NSStatusBar systemStatusBar]
                  statusItemWithLength:NSVariableStatusItemLength] retain];
  self = [super initWithFrame:NSMakeRect(0, 0, 100, 22)];

  for (int i = 0; i < 3; i++)
    {
      timebars[i] = [MacOSTimeBar alloc];
    }
    
  if (self) {
    menu = [myMenu retain];
    [statusItem setView:self];
    [statusItem setTitle: @"World"];
    menuVisibility = NO;
  }
    
  return self;
}


- (void)setBreak : (int) id
             text:(NSString *)text
     primaryColor:(ColorId)primaryColor
     primaryValue:(int)primaryValue
  primaryMaxValue:(int)primaryMaxValue
   secondaryColor:(ColorId)secondaryColor
   secondaryValue:(int)secondaryValue
secondaryMaxValue:(int)secondaryMaxValue;
{
  MacOSTimeBar *timebar = timebars[id];

  [timebar setText: text];
  [timebar setValue: primaryValue];
  [timebar setColor: primaryColor];
  [timebar setMaxValue: primaryMaxValue];
  [timebar setSecondaryValue: secondaryValue];
  [timebar setSecondaryMaxValue: secondaryMaxValue];
  [timebar setSecondaryColor: secondaryColor];
}

- (void)drawRect:(NSRect)rect
{
  // invert icon if necessary
  NSColor *color;
  if (!menuVisibility) 
    {
      color = [[[NSColor blackColor] retain] autorelease];
    }
  else
    {
      color = [[[NSColor whiteColor] retain] autorelease];
    }
	
  // draw item with status as background
  [statusItem drawStatusBarBackgroundInRect:[self frame] withHighlight:menuVisibility];
  
  [timebars[0] drawRect: rect];
}


- (void)mouseDown:(NSEvent *) theEvent
{
  menuVisibility = YES;
  [self setNeedsDisplay:YES];
  [statusItem popUpStatusItemMenu:menu];
  menuVisibility = NO;
  [self setNeedsDisplay:YES];
}	


- (bool)isMenuVisible
{
  return menuVisibility;
}


- (void)dealloc
{
  [menu release];
  [statusItem release];
  [super dealloc];
}

@end
