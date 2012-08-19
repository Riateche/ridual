#ifndef FILE_ACTION_TASK_H
#define FILE_ACTION_TASK_H

#include <QObject>
#include <QStringList>
#include <QVariant>
#include <QElapsedTimer>
#include <QTimer>
#include "File_info.h"
#include "gio/Mount.h"
#include "Directory_tree_item.h"

#define BUFFER_SIZE 65536

class Action_queue;
class Main_window;


enum Action_type {
  action_copy,
  action_move,
  action_link,
  action_delete,
  action_create_folder
};

enum Recursive_fetch_option {
  recursive_fetch_on = 1,
  recursive_fetch_off = 2,
  recursive_fetch_auto = 3
};

enum Link_type {
  link_type_soft_absolute,
  link_type_soft_relative,
  link_type_hard
};

class Action_data {
public:
  Action_type type;
  Recursive_fetch_option recursive_fetch_option;
  Link_type link_type;
  File_info_list targets;
  QString destination;
};

class Action_state {
public:
  Action_state() : errors_count(0), queue_id(0) {}
  QString current_action, current_progress, total_progress;
  int errors_count;
  int queue_id;
};
Q_DECLARE_METATYPE(Action_state)

class Action_abort_exception { };
class Action_retry_exception { };
class Action_skip_exception { };

class Action: public QObject {
  Q_OBJECT
public:
  explicit Action(Main_window* mw, const Action_data& p_data);
  ~Action();
  void set_queue(Action_queue* q);


signals:
  void error(QString message);
  void state_changed(Action_state state);
  void finished();
  void question(QString message, Error_type error_type, bool is_dir);


private:
  Main_window* main_window;
  Action_data data;
  Action_queue* queue;
  QList<gio::Mount> mounts;

  qint64 total_size, current_size;
  int total_count, current_count;

  QElapsedTimer signal_timer;
  static const int signal_interval = 300; //ms
  static const int auto_recursive_fetch_max = 1000;
  static const int sleep_interval = 100; //ms

  QString get_real_dir(QString uri);

  bool paused;
  bool cancelled;
  Error_reaction error_reaction;

  Error_reaction ask_question(QString message, Error_type error_type, bool is_dir);
  void process_events();

  void process_one(const QString& path, const QString& root_path, Action_state& state);


private slots:
  void run();

public slots:
  void question_answered(Error_reaction reaction);
  void toggle_pause();
  void abort();

};


#endif // FILE_ACTION_TASK_H
