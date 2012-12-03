#ifndef FILE_ACTION_TASK_H
#define FILE_ACTION_TASK_H

#include <QStack>
#include <QObject>
#include <QStringList>
#include <QVariant>
#include <QTimer>
#include <QTimer>
#include "File_info.h"
#include "gio/Mount.h"
#include "Error_reaction.h"
#include "types.h"
#include "Elapsed_timer.h"

#define BUFFER_SIZE 65535

class Action_queue;
class File_system_engine;




/*! Action represents a long operation created by user, e.g. copying file or folder.
  Any Action is runned in non-gui thread represented by Action_queue object.
  An Action object is created in gui thread, but when it's added to Action_queue,
  it's moved to the thread represented by it. So, any slot of Action is executed
  in this thread. When Action becomes the first in the queue (for the first Action
  it happens immediately after adding to the queue), Action_queue starts executing
  of Action::run method in its thread. When Action::run method is finished,
  the job considered done, so Action_queue deletes Action object and goes to the
  next one.

  Interaction between Action::run method and GUI thread is performed through
  Action::ask_question and Action::process_events methods called by Action::run.
  These methods call QApplication::processEvents, so Qt can call Action slots
  which can modify Action private fields. Action::ask_question and
  Action::process_events behaviour depends on these fields values.

  Action fields and methods must never be accessed directly from GUI thread or
  other threads. The only way of interaction with it is through signal-slot connection.

  Control flow in Action::run and nested methods is controlled using exceptions.
  It's not ideal solution, but no other solution has been found.

  \sa Action_queue

  */
class Action: public QObject {
  Q_OBJECT
public:
  /*!
    \param p_data   This parameter contains all information about
                    type of performed action, target files and
                    destination (if any), settings.
                    \sa Action_data

    */
  explicit Action(const Action_data& p_data);
  ~Action();

  /*! This function is called when Action is added to the queue.
    Action uses this value to report queue id in Action::state_changed
    signal and to call Action_queue::msleep when waiting for user
    answer or when paused.
    */
  void set_queue(Action_queue* q);

  void set_fs_engine(File_system_engine* v);

  /*! This function executes main operations for this action.
    When Action::run is finished, Action is deleted by Action_queue.
    This function is executed in Action_queue thread.
    */
  void run();


private:
  Action_data data; //! Data passed to the constructor.
  File_system_engine* fs_engine;

  Action_queue* queue;

  /*! Total size (in bytes) and count of all target files for the operation.
    These values may be calculated before performing main operation.
    If values was not calculated or calculation was failed,
    these values will be 0.
    */
  qint64 total_size;
  int total_count;

  /*! Total size (in bytes) and count of target files that has been processed.
    */
  qint64 current_size;
  int current_count;

  Elapsed_timer signal_timer; //! The timer used for sending Action::state_changed singal sometimes.
  static const int signal_interval = 300; //! Minimum interval (in ms) before two Action::state_changed signals.

  /*! Maximum files count for Recursive_fetch_option::auto.
    If Action::total_count reaches this value, recursive fetch shuts down
    and is considered failed (total_count and total_size becomes 0).
    */
  static const int auto_recursive_fetch_max = 1000;
  static const int sleep_interval = 100; //! Sleep time (in ms) for one iteration while waiting for user answer or resume after pause.

  //QString get_real_dir(QString uri); //! Convert URI to real path based on Action::mounts value.
  //Error_reaction::Enum error_reaction;



  /*! Ask a question specified by data. This function will wait for user answer
    and return it. It can also throw one of Abort_exception, Retry_exception
    or Skip_exception if user answer was 'abort', 'retry' or 'skip',
    respectively.

    Action slots can be called by Qt event loop while this method is executing.
    Precisely, Action::ask_question waits for Action::question_answered to
    be called and set Action::error_reaction value.
    */
  void ask_question(Question_data data);


  // perform copying for one file or directory
  void process_one(const File_info& file_info);

  void postprocess_directory(const File_info& file_info);

  void process_pending_operation();

  void send_state();

  // ...... new implementation ........

  bool state_delivery_in_process;
  //bool state_constructing_requested;

  QStack<File_system_engine::Iterator*> fs_iterators_stack;
  QStringList unprocessed_targets;
  enum Phase { phase_preparing, phase_processing, phase_finished };
  Phase phase;

  bool paused; //! Indicates if action is currently paused. This value is setted by Action::toggle_pause slot.
  bool blocked; //! Indicates if action is waiting for an answer.

  File_system_engine::Operation* pending_operation;
  File_info current_file; // for state constructing only


  void end_preparing();

  QTimer iteration_timer;
  void update_iteration_timer();




public slots:
  void question_answered(Error_reaction::Enum reaction);
  void set_paused(bool v);
  void abort();

  //todo: connect to this slot
  void state_delivered();


private slots:
  void run_iteration();


signals:
  void state_changed(Action_state state);
  void question(Question_data data);
  void started();
  void finished();

  //todo: connect to this signal
  void error(QString string);



};


#endif // FILE_ACTION_TASK_H
