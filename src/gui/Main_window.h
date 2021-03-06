#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QProcess>
#include <QMainWindow>
#include <QTimer>
#include "hotkeys/Hotkeys.h"
#include "File_info.h"
#include "App_info.h"
#include "Action.h"
#include <QPushButton>
#include "Core_ally.h"
#include <QSettings>

namespace Ui {
  class Main_window;
}

class Core;
class Pane;

class Tasks_thread;
class Action_queue;
class Action;
class Question_widget;
class Action_state_widget;





class Main_window : public QMainWindow, Core_ally {
  Q_OBJECT
  
public:
  explicit Main_window(Core *c);
  ~Main_window();
  void init();

  void set_active_pane(Pane* pane);
  inline Pane* get_active_pane() { return active_pane; }

  App_info_list get_apps(const QString& mime_type);
  App_info get_default_app(const QString& mime_type);

  Pane* get_destination_pane();

  Ui::Main_window* get_ui() { return ui; }

  inline Core* get_core() { return core; }

  inline Columns get_columns() { return columns; }
  void set_columns(Columns v);

  QString get_version();

  void set_current_queue(Action_queue *queue);


  Recursive_fetch_option get_recursive_fetch_option();

  /*! Add Question_widget to main window. This method
    should never be used directly. It's called by
    Question_widget constructor automatically.
    */
  void add_question(Question_widget* question);


  // direction: 1 (down) or -1 (top)
  void switch_focus_question(Question_widget* target, int direction);

private:
  Ui::Main_window *ui;
  QTimer save_settings_timer;
  Pane* active_pane;
  Hotkeys hotkeys;
  QSettings settings;

  Action_queue* current_queue;
  QList<Question_widget*> question_widgets;
  QList<Action_state_widget*> action_state_widgets;
  Columns columns;

  void view_or_edit_selected(bool edit);
  void view_or_edit_files(const File_info_list& list, bool edit);
  void send_answer(int index);
  bool eventFilter(QObject *object, QEvent *event);

  void keyPressEvent(QKeyEvent* event);
  void closeEvent(QCloseEvent* event);

  Action_data get_auto_target_and_destination(Action_type::Enum action_type);
  void copy_or_cut_files_to_clipboard(bool cut);

  void init_hotkeys();
  QString get_free_file_name(QString prefix);

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
  void on_action_remove_triggered();
  void on_action_move_triggered();

  void save_settings();
  void go_parent();
  void focus_address_line();
  void go_to(QString uri);
  void slot_selection_changed();
  void actions_recursive_fetch_triggered();
  void queue_destroyed(Action_queue* object);
  void slot_action_question(Question_data data);
  void slot_focus_question();
  void action_started();
  void question_widget_destroyed(QObject* object);
  void action_state_widget_destroyed(QObject* object);

  void on_action_clipboard_copy_triggered();

  void on_action_cut_triggered();

  void on_action_paste_triggered();

  void on_action_abort_last_created_task_triggered();

  void action_added(Action* a);

  void on_action_move_to_trash_triggered();

  void on_action_move_from_trash_triggered();

  void on_action_create_folder_triggered();

  void on_action_edit_new_file_triggered();
  void process_error(QProcess::ProcessError error);

  void on_action_rename_triggered();

public slots:
  void open_current();
  void switch_active_pane();
  void show_message(QString message, Icon::Enum icon);
  void show_error(QString message);

  /*! Create new action based on passed data.

    Created task is placed in the current queue (new queue is created if necessary).
    The action might be launched before or after this function is finished,
    depending on current queue state.
    */
  void create_action(Action_data data);

signals:
  void active_pane_changed();
  void selection_changed();
  void columns_changed(Columns);

};

#endif // MAIN_WINDOW_H
