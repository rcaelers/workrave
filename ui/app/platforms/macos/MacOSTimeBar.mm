#import "MacOSTimeBar.h"


@implementation MacOSTimeBar

- (void) setText: (NSString*) aText
{
  text = aText;
}

- (void) setValue: (int) aValue
{
  value = aValue;
}

- (void) setColor: (ColorId) aColor
{
  color = aColor;
}

- (void) setMaxValue: (int) aMaxValue
{
  max_value = aMaxValue;
}

- (void) setSecondaryValue: (int) aSecondaryValue
{
  secondary_value = aSecondaryValue;
}

- (void) setSecondaryMaxValue: (int) aSecondaryMaxValue
{
  secondary_max_value = aSecondaryMaxValue;
}

- (void) setSecondaryColor: (ColorId) aColor
{
  secondary_color = aColor;
}

- (void)drawRect:(NSRect)rect
{
//   [[NSColor whiteColor] set];
//   NSRectFill(NSMakeRect(1, 1, 98, 20));

  const int border_size = 2;

  int win_w = 30;
  int win_h = 22;

  // Draw background
  [[NSColor blackColor] set];
  NSFrameRect(NSMakeRect(1, 1, win_w - 2, win_h - 2));
  [[NSColor whiteColor] set];
  NSRectFill(NSMakeRect(2, 2, win_w - 4, win_h - 4));

  // Bar
  int bar_width = 0;
  if (max_value > 0)
    {
      bar_width = (value * (win_w - 2 * border_size)) / max_value;
    }

  // Secondary bar
  int sbar_width = 0;
  if (secondary_max_value >  0)
    {
      sbar_width = (secondary_value * (win_w - 2 * border_size)) / secondary_max_value;
    }

  int bar_height = win_h - 2 * border_size;

  if (sbar_width > 0)
    {
      // Overlap

      // We should assert() because this is not supported
      // but there are some weird boundary cases
      // in which this still happens.. need to check
      // this out some time.
      // assert(secondary_bar_color == COLOR_ID_INACTIVE);
      NSColor *overlap_color;
      switch (color)
        {
        case COLOR_ID_ACTIVE:
          overlap_color = [NSColor blueColor];
          break;
        case COLOR_ID_OVERDUE:
          overlap_color = [NSColor orangeColor];
          break;
        default:
          // We could abort() because this is not supported
          // but there are some weird boundary cases
          // in which this still happens.. need to check
          // this out some time.
          overlap_color = [NSColor redColor];
        }

      if (sbar_width >= bar_width)
        {
          if (bar_width)
            {
              [[NSColor redColor] set];
              NSRectFill(NSMakeRect(border_size, border_size, bar_width, bar_height));
//               window_gc->set_foreground(bar_colors[overlap_color]);
//               draw_bar(window, window_gc, true,
//                        border_size, border_size,
//                        bar_width, bar_height,
//                        win_w, win_h);
            }
          if (sbar_width > bar_width)
            {
              [[NSColor yellowColor] set];
              NSRectFill(NSMakeRect(border_size + bar_width, border_size, sbar_width - bar_width, bar_height));
//               window_gc->set_foreground(bar_colors[secondary_bar_color]);
//               draw_bar(window, window_gc, true,
//                        border_size + bar_width, border_size,
//                        sbar_width - bar_width, bar_height,
//                        win_w, win_h);
            }
        }
      else
        {
          if (sbar_width)
            {
              [[NSColor blueColor] set];
              NSRectFill(NSMakeRect(border_size, border_size, sbar_width, bar_height));
//               window_gc->set_foreground(bar_colors[overlap_color]);
//               draw_bar(window, window_gc, true,
//                        border_size, border_size,
//                        sbar_width, bar_height,
//                        win_w, win_h);
            }
          [[NSColor greenColor] set];
          NSRectFill(NSMakeRect(border_size + sbar_width, border_size, bar_width - sbar_width, bar_height));

//           window_gc->set_foreground(bar_colors[bar_color]);
//           draw_bar(window, window_gc, true,
//                    border_size + sbar_width, border_size,
//                    bar_width - sbar_width, bar_height,
//                    win_w, win_h);
        }
    }
  else
    {
      [[NSColor grayColor] set];
      NSRectFill(NSMakeRect(border_size, border_size, bar_width, bar_height));
      // No overlap
//       window_gc->set_foreground(bar_colors[bar_color]);
//       draw_bar(window, window_gc, true,
//                border_size, border_size,
//                bar_width, bar_height, win_w, win_h);
    }


  // Text
//   window_gc->set_foreground(bar_text_color);
//   Glib::RefPtr<Pango::Layout> pl1 = create_pango_layout(bar_text);
//   Glib::RefPtr<Pango::Context> pc1 = pl1->get_context();

//   Pango::Matrix matrix = PANGO_MATRIX_INIT;

//   pango_matrix_rotate(&matrix, 360 - rotation);
//   pc1->set_matrix(matrix);

//   int text_width, text_height;
//   pl1->get_pixel_size(text_width, text_height);

//   int text_x, text_y;

//   Gdk::Rectangle rect1, rect2;

//   if (rotation == 0 || rotation == 180)
//     {
//       if (win_w - text_width - MARGINX > 0)
//         {
//           if (bar_text_align > 0)
//             text_x = (win_w - text_width - MARGINX);
//           else if (bar_text_align < 0)
//             text_x = MARGINX;
//           else
//             text_x = (win_w - text_width) / 2;
//         }
//       else
//         {
//           text_x = MARGINX;
//         }
//       text_y = (win_h - text_height) / 2;

//       int left_width = (bar_width > sbar_width) ? bar_width : sbar_width;
//       left_width += border_size;

//       Gdk::Rectangle left_rect(0, 0, left_width, win_h);
//       Gdk::Rectangle right_rect(left_width, 0, win_w - left_width, win_h);

//       rect1 = left_rect;
//       rect2 = right_rect;
//     }
//   else
//     {
//       if (win_h - text_width - MARGINY > 0)
//         {
//           int a = bar_text_align;
//           if (rotation == 270)
//             {
//               a *= -1;
//             }
//           if (a > 0)
//             text_y = (win_h - text_width - MARGINY);
//           else if (a < 0)
//             text_y = MARGINY;
//           else
//             text_y = (win_h - text_width) / 2;
//         }
//       else
//         {
//           text_y = MARGINY;
//         }

//       text_x = (win_w - text_height) / 2;

//       int left_width = (bar_width > sbar_width) ? bar_width : sbar_width;
//       left_width += border_size;

//       Gdk::Rectangle up_rect(0, 0, win_w, left_width);
//       Gdk::Rectangle down_rect(0, left_width, win_w, win_h - left_width);

//       rect1 = up_rect;
//       rect2 = down_rect;
//     }

//   Gdk::RGBA textcolor = style->get_fg(Gtk::STATE_NORMAL);

//   TRACE_VAR(textcolor.get_red() << " " <<
//             textcolor.get_green() << " " <<
//             textcolor.get_blue());

//   Glib::RefPtr<Gdk::GC> window_gc1 = Gdk::GC::create(window);

//   window_gc1->set_clip_origin(0,0);
//   window_gc1->set_clip_rectangle(rect1);
//   window_gc1->set_foreground(bar_text_color);
//   window->draw_layout(window_gc1, text_x, text_y, pl1);

//   window_gc1->set_foreground(textcolor);
//   window_gc1->set_clip_rectangle(rect2);
//   window->draw_layout(window_gc1, text_x, text_y, pl1);
//   }


// void
// TimeBar::draw_bar(Glib::RefPtr<Gdk::Window> &window,
//                   const Glib::RefPtr<Gdk::GC> &gc,
//                   bool filled, int x, int y, int width, int height,
//                   int winw, int winh)
// {
//   (void) winh;

//   if (rotation == 0 || rotation == 180)
//     {
//       window->draw_rectangle(gc, filled, x, y, width, height);
//     }
//   else
//     {
//       window->draw_rectangle(gc, filled, y, winw - x - width, height, width);
//     }
// }


- (void)dealloc
{
  [super dealloc];
}

@end
