#ifndef FILE_ACTION_TASK_H
#define FILE_ACTION_TASK_H

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

  /*! Set list of mounts used to convert URIs to real paths.
    This should be called from gui thread before starting the task. */
  void set_mounts(const QList<Gio_mount>& _mounts) { mounts = _mounts; }

  /*! This function executes main operations for this action.
    When Action::run is finished, Action is deleted by Action_queue.
    This function is executed in Action_queue thread.
    */
  void run();


private:
  Action_data data; //! Data passed to the constructor.

  /*! data.destination converted from URI (see Directory) to
    real filesystem path and with removed trailing slash (if any).
    */
  QString normalized_destination;
  Action_queue* queue;
  QList<Gio_mount> mounts;

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

  bool paused; //! Indicates if action is currently paused. This value is setted by Action::toggle_pause slot.

  /*! Indicates if action is requested to abort.
      This value is setted by Action::abort and Action::question_answered slots.  */
  bool cancelled;

  /*! Last error reaction sent to Action::question_answered slot.
    This field is used internally by Action::question_answered and
    Action::ask_question, so it must not be used from any other place.
    */
  Error_reaction::Enum error_reaction;

  /*! The object filled before sending Action::state_changed signal.
      In other times contents of this object is outdated. */
  Action_state state;

  /*! Ask a question specified by data. This function will wait for user answer
    and return it. It can also throw one of Abort_exception, Retry_exception
    or Skip_exception if user answer was 'abort', 'retry' or 'skip',
    respectively.

    Action slots can be called by Qt event loop while this method is executing.
    Precisely, Action::ask_question waits for Action::question_answered to
    be called and set Action::error_reaction value.
    */
  Error_reaction::Enum ask_question(Question_data data);

  /*!
    Process slot calls passed to Action and perform sleep or cancel (if neseccary).
    This function should be called frequently in Action::run method and
    methods called by it. Action slots can be called by Qt event loop
    when this method is called.

    If the action was paused using Action::toggle_pause slot, this function
    goes to loop until action is resumed. So, when Action::run calls this function,
    operations of Action::run are paused.

    If the action is cancelled by user using Action::cancel, this function
    throws Abort_exception.
    */
  void process_events();

  // call prepare_one or process_one (depending on the parameter) on each target directory or file
  // recursively.
  void iterate_all(bool prepare);

  // perform recursive fetch for one file or directory
  void prepare_one(const QString& path, const QString& root_path, bool is_dir);

  // perform copying for one file or directory
  void process_one(const QString& path, const QString& root_path, bool is_dir, bool dir_before);


  class Abort_exception { };
  class Retry_exception { };
  class Skip_exception { };
  class Prepare_finished_exception {};

public slots:
  void question_answered(Error_reaction::Enum reaction);
  void set_paused(bool v);
  void abort();

signals:
  //void error(QString message);
  void state_changed(Action_state state);
  void question(Question_data data);
  void started();



};


#endif // FILE_ACTION_TASK_H
