#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "hotkeys/Hotkeys.h"
#include "File_info.h"
#include "Task.h"
#include "App_info.h"
#include "Action.h"
#include <QPushButton>
#include "Core_ally.h"

namespace Ui {
  class Main_window;
}

class Core;
class Pane;

class Tasks_thread;
class Action_queue;
class Action;
class Question_widget;






class Main_window : public QMainWindow, Core_ally {
  Q_OBJECT
  
public:
  explicit Main_window(Core *c);
  ~Main_window();
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
  Ui::Main_window *ui;
  QTimer save_settings_timer;
  Pane* active_pane;
  Hotkeys hotkeys;
  File_info_list old_path_items;
  Action_queue* current_queue;
  QList<Question_widget*> question_widgets;
    Columns columns;

  Action_queue* create_queue();
  void resizeEvent(QResizeEvent *);
  void view_or_edit_selected(bool edit);
  void send_answer(int index);
  bool eventFilter(QObject *object, QEvent *event);

  void keyPressEvent(QKeyEvent* event);


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
  void refresh_path_toolbar();
  void go_to(QString uri);
  void slot_selection_changed();
  void slot_actions_recursive_fetch_triggered();
  void slot_queue_destroyed(QObject* object);
  void slot_action_question(Question_data data);
  void slot_focus_question();
  void action_started();
  void question_widget_destroyed(QObject* object);

public slots:
  void open_current();
  void switch_active_pane();
  void show_message(QString message, Icon::Enum icon);

signals:
  void active_pane_changed();
  //void signal_add_task(Task task);
  void selection_changed();
  void action_added(Action*);
  void columns_changed(Columns);

};

#endif // MAIN_WINDOW_H
