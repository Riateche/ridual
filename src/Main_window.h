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
#include "File_action_task.h"

namespace Ui {
  class Main_window;
}

class Pane;

class Tasks_thread;
class File_action_queue;
class File_action_task;
class Tasks_model;

typedef struct _GVolumeMonitor GVolumeMonitor;
typedef struct _GDrive GDrive;


class Main_window : public QMainWindow {
  Q_OBJECT
  
public:
  explicit Main_window(QWidget *parent = 0);
  ~Main_window();
  void set_active_pane(Pane* pane);
  inline Pane* get_active_pane() { return active_pane; }
  void add_task(Task* task);

  /*! Get all currently active mounts.
    This function is thread-safe.
    */
  QList<gio::Mount> get_gio_mounts();
  QList<gio::Volume*> get_gio_volumes() { return volumes; }

  inline Bookmarks_file_parser* get_bookmarks() { return &bookmarks; }
  inline Bookmarks_file_parser* get_user_dirs() { return &user_dirs; }

  App_info_list get_apps(const QString& mime_type);
  App_info get_default_app(const QString& mime_type);

  Pane* get_destination_pane();

  Ui::Main_window* get_ui() { return ui; }

  File_action_queue* create_queue();
  QList<File_action_queue*> get_queues();

  Recursive_fetch_option get_recursive_fetch_option();

private:
  Bookmarks_file_parser bookmarks, user_dirs;
  GVolumeMonitor* volume_monitor;
  Ui::Main_window *ui;

  QTimer save_settings_timer;
  Pane* active_pane;
  Hotkeys hotkeys;

  QList<gio::Volume*> volumes;
  QList<gio::Mount> mounts;
  Tasks_thread* tasks_thread;

  File_info_list old_path_items;

  QList<gulong> gio_connects;

  Tasks_model* tasks_model;

  void init_gio_connects();
  void fetch_gio_mounts();
  static void gio_mount_changed(GVolumeMonitor *volume_monitor, GDrive *drive, Main_window* _this);
  void resizeEvent(QResizeEvent *);
  void view_or_edit_selected(bool edit);

private slots:
  void save_settings();
  void on_action_hotkeys_triggered();
  void go_parent();
  void focus_address_line();
  void refresh_path_toolbar();
  void go_to(QString uri);
  void slot_selection_changed();
  void slot_actions_recursive_fetch_triggered();

  void fatal_error(QString message);

  void resize_tasks_table();


  void on_action_go_places_triggered();

  void on_action_go_root_triggered();

  void on_action_refresh_triggered();

  void on_action_execute_triggered();

  void on_action_general_settings_triggered();

  void on_action_view_triggered();

  void on_action_edit_triggered();

  void on_action_copy_triggered();

public slots:
  void open_current();
  void switch_active_pane();

signals:
  void active_pane_changed();
  //void signal_add_task(Task task);
  void gio_mounts_changed();
  void selection_changed();
  void file_action_task_added(File_action_task*);
};

#endif // MAIN_WINDOW_H
