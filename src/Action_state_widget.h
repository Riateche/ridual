#ifndef ACTION_STATE_WIDGET_H
#define ACTION_STATE_WIDGET_H

#include <QWidget>
#include "types.h"

namespace Ui {
  class Action_state_widget;
}

/*! This widget displays the state of one Action. Objects of this class are embedded in
  Main_window.

  */
class Action_state_widget : public QWidget {
  Q_OBJECT
  
public:
  explicit Action_state_widget(Action* action);
  ~Action_state_widget();
  
private:
  Ui::Action_state_widget *ui;
  bool paused;
  QIcon icon_pause, icon_resume;

signals:
  void show_requested();
  void set_paused(bool paused);
  void state_received();

private slots:
  void state_changed(Action_state state);

  void on_pause_clicked();
};

#endif // ACTION_STATE_WIDGET_H
