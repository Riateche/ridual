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
#include "Core_ally.h"

#define BUFFER_SIZE 65535

class Action_queue;
class File_system_engine;




/*! Action represents a long operation requested by user, e.g. copying file or folder.
  Any Action is runned in non-gui thread represented by Action_queue object.
  An Action object is created in gui thread, but when it's added to Action_queue,
  it's moved to the thread represented by it. So, any slot of Action is executed
  in this thread. When Action becomes the first in the queue (for the first Action
  it happens immediately after adding to the queue), Action_queue starts executing
  of Action::run method in its thread. When Action::run method is finished,
  the job considered done, so Action_queue deletes Action object and goes to the
  next one.

  Action fields and methods must never be accessed directly from GUI thread or
  other threads. The only way of interaction with it is through signal-slot connection.

  \sa Action_queue

  */
class Action: public QObject {
  Q_OBJECT
public:
  /*!
    The constructor should be only used in Action_queue.

    \param p_data   This parameter contains all information about
                    type of performed action, target files and
                    destination (if any), settings.
                    \sa Action_data

    */
  explicit Action(Action_queue* q, File_system_engine* fs, const Action_data& p_data);

  ~Action();

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

  void ask_question(Question_data data);


  // perform copying for one file or directory
  void process_current(Error_reaction::Enum error_reaction = Error_reaction::undefined);

  void postprocess_directory(const File_info& file_info);

  void process_pending_operation();

  void send_state();

  //bool state_delivery_in_process;
  //bool state_constructing_requested;

  QStack<File_system_engine::Iterator*> fs_iterators_stack;
  QStringList unprocessed_targets;
  enum Phase { phase_queued, phase_preparing, phase_processing, phase_finished };
  Phase phase;

  bool paused; //! Indicates if action is currently paused. This value is setted by Action::toggle_pause slot.
  bool blocked; //! Indicates if action is waiting for an answer.
  bool postprocess_running;

  File_system_engine::Operation* pending_operation;
  File_info current_file; // for state constructing only

  void end_preparing();

  //void update_iteration_timer();


  QString find_autorename_path(const QString& path);




public slots:
  /*! Connect to this slot to notify Action about answers.
    */
  void question_answered(Error_reaction::Enum reaction);

  /*! Connect to this slot to pause or resume the operation.
    */
  void set_paused(bool v);

  /*! Connect to this slot to abort the operation. Action object will be
    deleted later by Action_queue.
    */
  void abort();

  /*! Connect to this slot to notify Action that Action's state has been displayed
    in GUI and Action is allowed to send another state_changed signal.
    */
  void state_delivered();

  void run();

private slots:
  void run_iterations();


signals:
  /*! Notify about action's state.
    */
  void state_changed(Action_state state);

  /*! Send a quaction to GUI.
    */
  void question(Question_data data);

  /*! Notify that the action has been started.
    */
  void started();

  /*! Notify that the action has been finished. Action object will be deleted later.
    */
  void finished();

  /*! Notify about unrecoverable error.
    */
  void error(QString string);

};


#endif // FILE_ACTION_TASK_H
