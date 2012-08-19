#ifndef TASKS_MODEL_H
#define TASKS_MODEL_H

#include <QAbstractTableModel>
#include "Action.h"

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
  void action_added(Action* task);
  void task_state_changed(Action_state state);
  void task_destroyed(QObject *obj);


private:
  Main_window* main_window;
  QList< QPair<Action*, Action_state> > items;
  
};

#endif // TASKS_MODEL_H
