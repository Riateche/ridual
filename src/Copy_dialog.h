#ifndef COPY_DIALOG_H
#define COPY_DIALOG_H

#include <QWidget>
#include "File_action_task.h"

namespace Ui {
  class Copy_dialog;
}



class Main_window;

class Copy_dialog : public QWidget {
  Q_OBJECT
  
public:
  explicit Copy_dialog(Main_window* mw,
                       File_action_type p_action,
                       QStringList p_target,
                       QString p_destination);
  ~Copy_dialog();
  
private slots:
  void on_start_clicked();

private:
  Ui::Copy_dialog *ui;
  Main_window* main_window;
  File_action_type action;
  QStringList target;
  QString destination;
};

#endif // COPY_DIALOG_H
