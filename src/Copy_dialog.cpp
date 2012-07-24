#include "Copy_dialog.h"
#include "ui_Copy_dialog.h"
#include <QSettings>
#include "File_action_queue.h"
#include "File_action_task.h"
#include "Main_window.h"
#include "Pane.h"
#include <QMessageBox>
Copy_dialog::Copy_dialog(Main_window *mw, File_action_type p_action) :
  QWidget(),
  ui(new Ui::Copy_dialog),
  main_window(mw),
  action(p_action)
{
  ui->setupUi(this);
  QSettings settings;
  int option = settings.value("recursive_fetch_last_used", 1).toInt();
  switch(static_cast<Recursive_fetch_option>(option)) {
    case recursive_fetch_on: ui->recursive_fetch_on->setChecked(true); break;
    case recursive_fetch_off: ui->recursive_fetch_off->setChecked(true); break;
    case recursive_fetch_auto: ui->recursive_fetch_auto->setChecked(true); break;
  }
  target = main_window->get_active_pane()->get_selected_files();
  if (target.isEmpty()) {
    QMessageBox::critical(0, "", tr("No files selected."));
    deleteLater();
    return;
  }

  destination = main_window->get_destination_pane()->get_uri();

  QString description;
  QStringList filenames;
  QString shared_parent;
  if (target.count() > 1) {
    shared_parent = target.first().parent_folder;
    for(int i = 0; i < target.count(); i++) {
      if (target[i].parent_folder.isEmpty()) {
        qWarning("Error: Copy_dialog: parent_folder is empty");
        QMessageBox::warning(0, "", tr("Unexpected failure."));
        deleteLater();
        return;
      }
      if (shared_parent != target[i].parent_folder) {
        //parent folder mismatch
        shared_parent.clear();
        break;
      }
      filenames << target[i].full_name;
      if (filenames.count() >= max_files_in_description) break;
    }
  }

  if (shared_parent.isEmpty()) {
    //parent folders are different, we need to display full paths
    filenames.clear();
    for(int i = 0; i < target.count(); i++) {
      filenames << target[i].full_path;
      if (filenames.count() >= max_files_in_description) break;
    }
  }

  if (target.count() > filenames.count()) {
    filenames << "...";
  }

  QString list;
  if (filenames.count() == 1) {
    list = tr("%1<br>").arg(filenames[0]);
  } else {
    list = tr("<ul><li>%1</li></ul>").arg(filenames.join("</li><li>"));
  }

  if (action == file_action_copy) {
    if (target.count() == 1) {
      description = tr("Copy <b>%1</b> <br>to <b>%2</b>")
          .arg(list).arg(destination);
    } else {
      if (shared_parent.isEmpty()) {
        description = tr("Copy %2 files: <b>%1</b> to <b>%3</b>")
            .arg(list)
            .arg(target.count())
            .arg(destination);
      } else {
        description = tr("Copy %2 files: <b>%1</b> from <b>%3</b> <br>to <b>%4</b>")
            .arg(list)
            .arg(target.count())
            .arg(shared_parent)
            .arg(destination);

      }
    }
  } else {
    description = tr("Unsupported action");
  }
  ui->description->setHtml(description);

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
  File_action_task* task = new File_action_task(main_window, action, target, destination);
  task->set_recursive_fetch(recursive_fetch_option);
  File_action_queue* queue = main_window->create_queue();
  queue->add_task(task);
  deleteLater();
}
