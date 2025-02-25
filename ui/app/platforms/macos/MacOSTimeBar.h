#import "ColorId.h"
#import <Cocoa/Cocoa.h>

@interface MacOSTimeBar : NSObject
{
  //! Color of the time-bar.
  ColorId color;

  //! Color of the time-bar.
  ColorId secondary_color;

  //! Color of the text.
  ColorId text_color;

  //! The current value.
  int value;

  //! The maximum value.
  int max_value;

  //! The current value.
  int secondary_value;

  //! The maximum value.
  int secondary_max_value;

  //! Text to show;
  NSString *text;
}

- (void)drawRect:(NSRect)rect;
- (void)dealloc;

- (void)setText:(NSString *)text;
- (void)setValue:(int)value;
- (void)setMaxValue:(int)max_value;
- (void)setColor:(ColorId)color;
- (void)setSecondaryValue:(int)secondary_value;
- (void)setSecondaryMaxValue:(int)secondary_max_value;
- (void)setSecondaryColor:(ColorId)secondary_color;

@end
