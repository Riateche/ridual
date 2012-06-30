#ifndef PATH_BUTTON_H
#define PATH_BUTTON_H

#include <QToolButton>
#include "File_info.h"
#include <QPushButton>

class Main_window;
class Directory;

class Path_button : public QToolButton {
  Q_OBJECT
public:
  explicit Path_button(Main_window* mw, QString text, QString uri);
  inline void set_go_parent_visible(bool v) { go_parent_visible = v; }

  void contextMenuEvent(QContextMenuEvent *e);
signals:
  void go_to(QString uri);
  
public slots:
  void slot_clicked();

private:
  Main_window* main_window;
  QString uri;
  Directory* parent_directory;

  bool menu_pending;
  QPoint menu_point;
  bool go_parent_visible;


private slots:
  void directory_ready(File_info_list files);
  void menu_action_triggered();
  void action_go_parent_triggered();
};

#endif // PATH_BUTTON_H
