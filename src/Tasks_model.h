#ifndef TASKS_MODEL_H
#define TASKS_MODEL_H

#include <QAbstractTableModel>
#include "File_action_task.h"

class Main_window;

class Tasks_model : public QAbstractTableModel {
  Q_OBJECT
public:
  explicit Tasks_model(Main_window* mw);
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

signals:
  
private slots:
  void task_added(File_action_task* task);
  void task_state_changed(File_action_state state);
  void task_destroyed();


private:
  Main_window* main_window;
  QList< QPair<File_action_task*, File_action_state> > items;
  
};

#endif // TASKS_MODEL_H
