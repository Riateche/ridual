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

class Action : public QObject {
  Q_OBJECT
public:
  explicit Action(Main_window* mw, const Action_data& p_data);
  ~Action();
  inline void set_queue_id(int v) { tmp_state.queue_id = v; }
  
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

  QElapsedTimer signal_timer;
  static const int signal_interval = 300; //ms
  static const int auto_recursive_fetch_max = 1000;


  QString get_real_dir(QString uri);

  Directory_tree_item* tree;

  QTimer iteration_timer;


  enum Run_state {
    run_state_preparing,
    run_state_main,
    run_state_copy_file
  };

  Run_state run_state;

  /* Preparing_errors_skipped indicates that there are some errors while
    reading file list. If it's true then after reaching the end of the tree
    new iteration will be preparing stage. Else it will be main stage.
    This value is cleared every time we start preparing stage.
    */
  bool preparing_errors_skipped;

  Directory_tree_item* current_item;

  QFile* file1;
  QFile* file2;
  char copy_buffer[BUFFER_SIZE];

  Directory_tree_item* questioned_item;
  Directory_tree_item* pending_answer;

  /*
    Current_iteration_errors is empty on first lookup. New errors are pushed in
    next_iteration_errors list. After we reach the end of the tree,
    if next_iteration_errors is empty, the task is considered is done. Else
    next_iteration_errors contents is moved to current_iteration_errors and we start
    from the first element of current_iteration_errors. There we iterate through the tree,
    not current_iteration_errors list. Again, new errors are pushed in
    next_iteration_errors list. Answers contained by current_iteration_errors are used
    in the operation. If we reach unanswered item of current_iteration_errors,
    this item and the following contents of current_iteration_errors are moved to the end of
    next_iteration_errors. After that we consider the iteration completed and
    repeat the actions as if we reach the end of the tree.
    If on iteration start the first item of current_iteration_errors is unanswered,
    the action is stopped until the answer is given.
    */
  QList<Directory_tree_item*> current_iteration_errors, next_iteration_errors;

  void add_error(Directory_tree_item* item);

  void file_copy_iteration();
  void preparing_iteration();
  void main_iteration();

  void delete_file_objects();
  void send_question(Directory_tree_item* item);

  Action_state tmp_state;



private slots:
  void run();
  void iteration();

public slots:
  void question_answered(Error_reaction reaction);

};

#endif // FILE_ACTION_TASK_H
