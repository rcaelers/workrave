#import "ColorId.h"
#import <Cocoa/Cocoa.h>

@interface MacOSTimeBar : NSObject
{
  //! Color of the time-bar.
  enum ColorId color;

  //! Color of the time-bar.
  enum ColorId secondary_color;

  //! Color of the text.
  enum ColorId text_color;

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

- (void)setText:(NSString *)text;
- (void)setValue:(int)value;
- (void)setMaxValue:(int)max_value;
- (void)setColor:(enum ColorId)color;
- (void)setSecondaryValue:(int)secondary_value;
- (void)setSecondaryMaxValue:(int)secondary_max_value;
- (void)setSecondaryColor:(enum ColorId)secondary_color;

@end
