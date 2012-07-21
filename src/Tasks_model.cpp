#include "Tasks_model.h"
#include "Main_window.h"
#include "File_action_queue.h"

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
  if (i < 0 || i >= items.count()) return QVariant();
  //return QVariant();
  if (role == Qt::DisplayRole) {
    switch(index.column()) {
      case 0:
        if (items[i].first->get_queue()) {
          return items[i].first->get_queue()->get_id();
        }
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
  return QVariant();
}

QVariant Tasks_model::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    switch(section) {
      case 0: return tr("Queue");
      case 1: return tr("Current action");
      case 2: return tr("Current progress");
      case 3: return tr("Total progress");
      case 4: return tr("Error count");
    }
  }
  return QVariant();
}

void Tasks_model::task_added(File_action_task *task) {
  emit layoutAboutToBeChanged();
  connect(task, SIGNAL(state_changed(File_action_state)), this, SLOT(task_state_changed(File_action_state)));
  connect(task, SIGNAL(destroyed()), this, SLOT(task_destroyed()));
  items << qMakePair(task, File_action_state());
  emit layoutChanged();
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

void Tasks_model::task_destroyed() {
  for(int i = 0; i < items.count(); i++) {
    if (items[i].first == sender()) {
      emit layoutAboutToBeChanged();
      items.removeAt(i);
      emit layoutChanged();
      return;
    }
  }
}
