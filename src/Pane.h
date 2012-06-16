#ifndef PANE_H
#define PANE_H

#include <QWidget>

namespace Ui {
  class Pane;
}

class Pane : public QWidget
{
  Q_OBJECT
  
public:
  explicit Pane(QWidget *parent = 0);
  ~Pane();
  
private:
  Ui::Pane *ui;
};

#endif // PANE_H
