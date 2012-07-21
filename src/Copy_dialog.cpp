#include "Copy_dialog.h"
#include "ui_Copy_dialog.h"
#include <QSettings>
#include "File_action_queue.h"
#include "File_action_task.h"
#include "Main_window.h"

Copy_dialog::Copy_dialog(Main_window *mw, File_action_type p_action, QStringList p_target, QString p_destination) :
  QWidget(),
  ui(new Ui::Copy_dialog),
  main_window(mw),
  action(p_action),
  target(p_target),
  destination(p_destination)
{
  ui->setupUi(this);
  QSettings settings;
  int option = settings.value("recursive_fetch_last_used", 1).toInt();
  switch(static_cast<Recursive_fetch_option>(option)) {
    case recursive_fetch_on: ui->recursive_fetch_on->setChecked(true); break;
    case recursive_fetch_off: ui->recursive_fetch_off->setChecked(true); break;
    case recursive_fetch_auto: ui->recursive_fetch_auto->setChecked(true); break;
  }
  show();
}

Copy_dialog::~Copy_dialog() {
  delete ui;
}

void Copy_dialog::on_start_clicked() {
  QSettings settings;
  Recursive_fetch_option recursive_fetch_option;
  if (ui->recursive_fetch_on->isChecked()) recursive_fetch_option = recursive_fetch_on;
  else if (ui->recursive_fetch_off->isChecked()) recursive_fetch_option = recursive_fetch_off;
  else recursive_fetch_option = recursive_fetch_auto;
  settings.setValue("recursive_fetch_last_used", static_cast<int>(recursive_fetch_option));
  File_action_task* task = new File_action_task(action, target, destination);
  task->set_recursive_fetch(recursive_fetch_option);
  File_action_queue* queue = main_window->create_queue();
  queue->add_task(task);
  deleteLater();
}
