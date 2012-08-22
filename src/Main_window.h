#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "hotkeys/Hotkeys.h"
#include "gio/Mount.h"
#include "gio/Volume.h"
#include "File_info.h"
#include "Task.h"
#include "Bookmarks_file_parser.h"
#include "App_info.h"
#include "Action.h"
#include <QPushButton>

namespace Ui {
  class Main_window;
}

class Pane;

class Tasks_thread;
class Action_queue;
class Action;
class Tasks_model;
class Directory_watcher;
class Question_widget;

typedef struct _GVolumeMonitor GVolumeMonitor;
typedef struct _GDrive GDrive;




class Main_window : public QMainWindow {
  Q_OBJECT
  
public:
  explicit Main_window(QWidget *parent = 0);
  ~Main_window();
  void set_active_pane(Pane* pane);
  inline Pane* get_active_pane() { return active_pane; }
  //void add_task(Task* task);

  /*! Get all currently active mounts.
    This function is thread-safe.
    */
  QList<Gio_mount> get_gio_mounts();
  QList<Gio_volume*> get_gio_volumes() { return volumes; }

  inline Bookmarks_file_parser* get_bookmarks() { return &bookmarks; }
  inline Bookmarks_file_parser* get_user_dirs() { return &user_dirs; }

  App_info_list get_apps(const QString& mime_type);
  App_info get_default_app(const QString& mime_type);

  Pane* get_destination_pane();

  Ui::Main_window* get_ui() { return ui; }



  Directory_watcher* get_watcher() { return watcher; }

  /*! Get all currently existing queues. Note that any queue is deleted
    when it becomes empty.
    */
  QList<Action_queue*> get_queues();

  /*! Return the current queue e.g. the queue for new task to be placed at.
    If there is no current queue this function calls Main_window::create_queue
    and returns a pointer to new queue.
    */
  Action_queue* get_current_queue();

  void set_current_queue(Action_queue *queue);

  /*! Create new action based on passed data. Several values in data are replaced
    accordingly to current application state:
    - recursive_fetch_option
    - targets
    - destination
    This fields should not be filled before calling this function.

    Created task is placed in the current queue (new queue is created if necessary).
    The action might be launched before or after this function is finished,
    depending on current queue state.
    */
  void create_action(Action_data data);

  Recursive_fetch_option get_recursive_fetch_option();

  /*! Add Question_widget to main window. This method
    should never be used directly. It's called by
    Question_widget constructor automatically.
    */
  void add_question(Question_widget* question);


  // direction: 1 (down) or -1 (top)
  void switch_focus_question(Question_widget* target, int direction);

private:
  Bookmarks_file_parser bookmarks, user_dirs;
  GVolumeMonitor* volume_monitor;
  Ui::Main_window *ui;

  QTimer save_settings_timer;
  Pane* active_pane;
  Hotkeys hotkeys;

  QList<Gio_volume*> volumes;
  QList<Gio_mount> mounts;
  //Tasks_thread* tasks_thread;

  File_info_list old_path_items;

  QList<gulong> gio_connects;

  Tasks_model* tasks_model;

  Action_queue* current_queue;


  Directory_watcher* watcher;
  QThread* watcher_thread;

  Action_queue* create_queue();
  void init_gio_connects();
  void fetch_gio_mounts();
  static void gio_mount_changed(GVolumeMonitor *volume_monitor, GDrive *drive, Main_window* _this);
  void resizeEvent(QResizeEvent *);
  void view_or_edit_selected(bool edit);

  void send_answer(int index);

  bool eventFilter(QObject *object, QEvent *event);

private slots:
  void on_action_hotkeys_triggered();
  void on_action_go_places_triggered();
  void on_action_go_root_triggered();
  void on_action_refresh_triggered();
  void on_action_execute_triggered();
  void on_action_general_settings_triggered();
  void on_action_view_triggered();
  void on_action_edit_triggered();
  void on_action_copy_triggered();
  void on_action_queue_choose_triggered();
  void on_action_about_triggered();

  void save_settings();
  void go_parent();
  void focus_address_line();
  void refresh_path_toolbar();
  void go_to(QString uri);
  void slot_selection_changed();
  void slot_actions_recursive_fetch_triggered();
  void fatal_error(QString message);
  void resize_tasks_table();
  void slot_queue_destroyed(QObject* object);
  void slot_action_question(Question_data data);
  void slot_focus_question();

public slots:
  void open_current();
  void switch_active_pane();

signals:
  void active_pane_changed();
  //void signal_add_task(Task task);
  void gio_mounts_changed();
  void selection_changed();
  void action_added(Action*);

};

#endif // MAIN_WINDOW_H
