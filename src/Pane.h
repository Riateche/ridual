#ifndef PANE_H
#define PANE_H

#include <QWidget>
#include <QDir>
#include "File_list_model.h"
#include <QSettings>

namespace Ui {
  class Pane;
}

class Main_window;

class Pane : public QWidget {
  Q_OBJECT
  
public:
  explicit Pane(QWidget* parent);
  ~Pane();
  void set_main_window(Main_window* main_window);
  void set_directory(QString dir);

  bool eventFilter(QObject* object, QEvent* event);

  void load_state(QSettings* s);
  void save_state(QSettings* s);

  bool is_active() const;

public slots:
  void go_parent();
  void open_current();

private slots:
  void on_go_clicked();
  void directory_readed(QFileInfoList files);
  void active_pane_changed();

private:
  Ui::Pane *ui;
  QString directory;
  bool ready;
  File_list_model file_list_model;

  void focusInEvent(QFocusEvent *);
  Main_window* main_window;

};

#endif // PANE_H
