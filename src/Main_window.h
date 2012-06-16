#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

namespace Ui {
  class Main_window;
}

class Main_window : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit Main_window(QWidget *parent = 0);
  ~Main_window();
  
private:
  Ui::Main_window *ui;
};

#endif // MAIN_WINDOW_H
