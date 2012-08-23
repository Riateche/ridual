#ifndef ACTION_STATE_WIDGET_H
#define ACTION_STATE_WIDGET_H

#include <QWidget>
#include "types.h"

namespace Ui {
  class Action_state_widget;
}

class Action_state_widget : public QWidget {
  Q_OBJECT
  
public:
  explicit Action_state_widget(Action* action);
  ~Action_state_widget();
  
private:
  Ui::Action_state_widget *ui;

signals:
  void show_requested();

private slots:
  void state_changed(Action_state state);

};

#endif // ACTION_STATE_WIDGET_H
