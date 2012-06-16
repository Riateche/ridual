#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
  class Main_window;
}

class Pane;

class Main_window : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit Main_window(QWidget *parent = 0);
  ~Main_window();
  void set_active_pane(Pane* pane);
  void switch_active_pane();
  inline Pane* get_active_pane() { return active_pane; }
  
private:
  Ui::Main_window *ui;

  QTimer save_settings_timer;
  Pane* active_pane;


private slots:
  void save_settings();

signals:
  void active_pane_changed();
};

#endif // MAIN_WINDOW_H
