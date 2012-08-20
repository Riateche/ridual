#ifndef FILE_ACTION_TASK_H
#define FILE_ACTION_TASK_H

#include <QObject>
#include <QStringList>
#include <QVariant>
#include <QElapsedTimer>
#include <QTimer>
#include "File_info.h"
#include "gio/Mount.h"
#include "Error_reaction.h"
#include "types.h"

#define BUFFER_SIZE 65536

class Action_queue;
class Main_window;








class Action: public QObject {
  Q_OBJECT
public:
  explicit Action(Main_window* mw, const Action_data& p_data);
  ~Action();
  void set_queue(Action_queue* q);
  void run();


private:
  Main_window* main_window;
  Action_data data;
  QString normalized_destination;
  Action_queue* queue;
  QList<Gio_mount> mounts;

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

  Action_state state;

  Error_reaction ask_question(Question_data data);
  void process_events();



  void iterate_all(bool prepare);

  void prepare_one(const QString& path, const QString& root_path, bool is_dir);
  void process_one(const QString& path, const QString& root_path, bool is_dir);


  class Abort_exception { };
  class Retry_exception { };
  class Skip_exception { };
  class Prepare_finished_exception {};

public slots:
  void question_answered(Error_reaction reaction);
  void toggle_pause();
  void abort();

signals:
  //void error(QString message);
  void state_changed(Action_state state);
  void question(Question_data data);



};


#endif // FILE_ACTION_TASK_H
