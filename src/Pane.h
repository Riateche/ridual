#ifndef PANE_H
#define PANE_H

#include <QWidget>
#include <QDir>
#include "File_list_model.h"
#include <QSettings>
#include <QFileSystemWatcher>
#include <QItemSelection>

namespace Ui {
  class Pane;
}

class Main_window;
class Directory;

class Pane : public QWidget {
  Q_OBJECT
  
public:
  explicit Pane(QWidget* parent);
  ~Pane();
  void set_main_window(Main_window* main_window);
  void set_uri(QString dir);

  bool eventFilter(QObject* object, QEvent* event);

  void load_state(QSettings* s);
  void save_state(QSettings* s);

  bool is_active() const;

  QString get_uri();

  inline void set_columns(Columns columns) { file_list_model.set_columns(columns); }

  File_info_list get_selected_files();
  File_info get_current_file();

  void setFocus();

signals:
  void uri_changed();
  void selection_changed();

public slots:
  void go_parent();
  void focus_address_line();
  void refresh();

private slots:
  void on_go_clicked();
  void active_pane_changed();
  void show_loading_indicator();
  void directory_ready(File_info_list files);
  void directory_error(QString message);
  void current_index_changed(QModelIndex current, QModelIndex previous);


  void on_list_customContextMenuRequested(const QPoint &pos);
  void action_launch_triggered();







private:
  Ui::Pane *ui;
  bool ready;
  File_list_model file_list_model;
  Main_window* main_window;
  Directory* directory, *pending_directory;

  void update_model_current_index();

};

#endif // PANE_H
