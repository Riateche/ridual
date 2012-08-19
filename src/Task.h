#ifndef TASK_H
#define TASK_H

#include <QMutex>
#include <QObject>
#include "File_action_task.h" //todo: remove
#include "File_leaf.h"

class Task: public QObject {
  Q_OBJECT
public:
  explicit Task(Main_window* mw, const Action_data& p_data);
  ~Task();
  inline void set_queue_id(int v) { state.queue_id = v; }


signals:
  void error(QString message);
  void state_changed(Action_state state);
  void finished();
  void question(QString message, Error_type error_type, bool is_dir);


private:
  Main_window* main_window;
  Action_data data;
  QList<gio::Mount> mounts;

  qint64 total_size, current_size;
  int total_count, current_count;

  //QList<File_leaf*> roots;


  QElapsedTimer signal_timer;
  static const int signal_interval = 300; //ms
  static const int auto_recursive_fetch_max = 1000;

  QString get_real_dir(QString uri);

  //File_leaf* current_root;
  //File_leaf* current_item;

  Action_state state;

private slots:
  void run();

public slots:
  void question_answered(Error_reaction reaction);

};


#endif // TASK_H
