#ifndef PANE_H
#define PANE_H

#include <QWidget>
#include <QDir>
#include "File_list_model.h"
#include <QSettings>
#include <QFileSystemWatcher>
#include <QItemSelection>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

namespace Ui {
  class Pane;
}

class Main_window;
class Directory;
class File_list_proxy_model;

class Pane : public QWidget {
  Q_OBJECT
  
public:
  explicit Pane(QWidget* parent);
  ~Pane();
  void set_main_window(Main_window* main_window);
  void set_uri(QString dir);


  void load_state(QSettings* s);
  void save_state(QSettings* s);

  /*! Check if this pane is active. Only one of two panes can be active at a time.
    */
  bool is_active() const;

  /*! Get the uri of location which content is displayed in file list. This can be empty.
    */
  QString get_uri();

  /*! Set columns used by file list table.
    */
  inline void set_columns(Columns columns) { file_list_model.set_columns(columns); }

  /*! Get list of selected files. If  fallback_to_current is true (default) and
    none is selected but there is a current row, this row is treated as selected.
    */
  File_info_list get_selected_files(bool fallback_to_current = true);

  /*! Get file pointed by current row. If there is no current row,
    invalid File_info is returned (invalid File_info has empty 'uri' field).
    */
  File_info get_current_file();

  //! Set focus to this pane's file list.
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
  void completion_directory_ready(File_info_list files);
  void on_list_customContextMenuRequested(const QPoint &pos);
  void action_launch_triggered();

  void on_address_textEdited(const QString &);

private:
  Ui::Pane *ui;
  bool ready;
  File_list_proxy_model* proxy_model;
  File_list_model file_list_model;
  Main_window* main_window;
  Directory* directory, *pending_directory;
  QString last_completion_uri;

  void update_model_current_index();
  bool eventFilter(QObject* object, QEvent* event);
  QStandardItemModel uri_completion_model;
  Directory* completion_directory;

};

#endif // PANE_H
