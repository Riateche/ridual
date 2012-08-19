#include "Tasks_model.h"
#include "Main_window.h"
#include "Action_queue.h"
#include <QDebug>

Tasks_model::Tasks_model(Main_window *mw) :
  QAbstractTableModel(),
  main_window(mw)
{
  connect(main_window, SIGNAL(file_action_task_added(Action*)),
          this, SLOT(task_added(Action*)));
}

int Tasks_model::rowCount(const QModelIndex &parent) const {
  return items.count();
  Q_UNUSED(parent);
}

int Tasks_model::columnCount(const QModelIndex &parent) const {
  return 5;
  Q_UNUSED(parent);
}

QVariant Tasks_model::data(const QModelIndex &index, int role) const {
  int i = index.row();
  if (i < 0 || i >= items.count()) return QVariant();
  if (role == Qt::ToolTipRole && index.column() == 1) {
    return items[i].second.current_action;
  }
  //return QVariant();
  if (role == Qt::DisplayRole) {
    switch(index.column()) {
      case 0:
        if (!items[i].second.queue_id) return tr("—");
        return items[i].second.queue_id;
      case 1: {
        return items[i].second.current_action;
      }
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

void Tasks_model::task_added(Action *task) {
  emit layoutAboutToBeChanged();
  connect(task, SIGNAL(state_changed(Action_state)), this, SLOT(task_state_changed(Action_state)));
  connect(task, SIGNAL(destroyed(QObject*)), this, SLOT(task_destroyed(QObject*)));
  items << qMakePair(task, Action_state());
  emit layoutChanged();
}

void Tasks_model::task_state_changed(Action_state state) {
  for(int i = 0; i < items.count(); i++) {
    if (items[i].first == sender()) {
      items[i].second = state;
      emit dataChanged(index(i, 0), index(i, columnCount()));
      return;
    }
  }
}

void Tasks_model::task_destroyed(QObject* obj) {
  for(int i = 0; i < items.count(); i++) {
    if (items[i].first == obj) {
      emit layoutAboutToBeChanged();
      items.removeAt(i);
      emit layoutChanged();
      return;
    }
  }
}
