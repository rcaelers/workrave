#ifndef TIMEBAR_HH
#define TIMEBAR_HH

#include "ITimeBar.hh"

#include <QWidget>

// namespace Ui {
//   class TimeBar;
//}

class TimeBar : public QWidget, public ITimeBar
{
  Q_OBJECT
  
public:
  explicit TimeBar(QWidget *parent = 0);
  ~TimeBar();

  QSize minimumSizeHint() const;
  QSize sizeHint() const;
 
  virtual void set_progress(int value, int max_value);
  virtual void set_secondary_progress(int value, int max_value);

  virtual void set_text(std::string text);

  virtual void update();
  virtual void set_bar_color(ColorId color);
  virtual void set_secondary_bar_color(ColorId color);
 
protected:
  void paintEvent(QPaintEvent *event);

private:
  
private:
  static QColor bar_colors[COLOR_ID_SIZEOF];

  //! Color of the time-bar.
  ColorId bar_color;

  //! Color of the time-bar.
  ColorId secondary_bar_color;

  //! Color of the text.
  //! The current value.
  int bar_value;

  //! The maximum value.
  int bar_max_value;

  //! The current value.
  int secondary_bar_value;

  //! The maximum value.
  int secondary_bar_max_value;

  //! Text to show;
  std::string bar_text;
};

#endif // TIMEBAR_HH
