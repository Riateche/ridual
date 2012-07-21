#ifndef COPY_DIALOG_H
#define COPY_DIALOG_H

#include <QWidget>
#include "File_action_task.h"
#include "File_info.h"

namespace Ui {
  class Copy_dialog;
}



class Main_window;

class Copy_dialog : public QWidget {
  Q_OBJECT
  
public:
  explicit Copy_dialog(Main_window* mw,
                       File_action_type p_action);
  ~Copy_dialog();
  
  static const int max_files_in_description = 10;


private slots:
  void on_start_clicked();

private:
  Ui::Copy_dialog *ui;
  Main_window* main_window;
  File_action_type action;
  File_info_list target;
  QString destination;
};

#endif // COPY_DIALOG_H
