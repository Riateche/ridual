#include "Tasks_model.h"
#include "Main_window.h"

Tasks_model::Tasks_model(Main_window *mw) :
  QAbstractTableModel(),
  main_window(mw)
{
  connect(main_window, SIGNAL(file_action_task_added(File_action_task*)),
          this, SLOT(task_added(File_action_task*)));
}

int Tasks_model::rowCount(const QModelIndex &parent) const {
  return items.count();
}

int Tasks_model::columnCount(const QModelIndex &parent) const {
  return 5;
}

QVariant Tasks_model::data(const QModelIndex &index, int role) const {
  int i = index.row();
  if (role == Qt::DisplayRole) {
    switch(index.column()) {
      case 0:
        //queue id
        break;
      case 1:
        return items[i].second.current_action;
      case 2:
        return items[i].second.current_progress;
      case 3:
        return items[i].second.total_progress;
      case 4:
        return items[i].second.errors_count;
    }
  }
}

void Tasks_model::task_added(File_action_task *task) {
  connect(task, SIGNAL(state_changed(File_action_state)), this, SLOT(task_state_changed(File_action_state)));
  items << qMakePair(task, File_action_state());
}

void Tasks_model::task_state_changed(File_action_state state) {
  for(int i = 0; i < items.count(); i++) {
    if (items[i].first == sender()) {
      items[i].second = state;
      emit dataChanged(index(i, 0), index(i, columnCount()));
      return;
    }
  }
}
