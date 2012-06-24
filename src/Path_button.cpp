#include "Path_button.h"
#include "Main_window.h"
#include <QMenu>
#include "Directory.h"

#include <QContextMenuEvent>

Path_button::Path_button(Main_window* mw, QString text, QString p_path) :
  QToolButton(mw),
  main_window(mw),
  path(p_path),
  parent_directory(0),
  go_parent_visible(false)
{
  setText(text);
  setToolTip(path);
  setCheckable(true);
  connect(this, SIGNAL(clicked()), this, SLOT(slot_clicked()));
  //setContextMenuPolicy(Qt::CustomContextMenu);
}

void Path_button::contextMenuEvent(QContextMenuEvent *e) {
  if (parent_directory == 0) {
    Directory d(main_window, path);
    parent_directory = new Directory(main_window, d.get_parent_uri());
    connect(parent_directory, SIGNAL(ready(QList<File_info>)),
            this, SLOT(directory_ready(QList<File_info>)));
  }
  menu_point = e->globalPos();
  menu_pending = true;
  parent_directory->refresh();
}

void Path_button::slot_clicked() {
  emit go_to(path);
}

void Path_button::directory_ready(QList<File_info> files) {
  if (!menu_pending) return;
  QMenu* menu = new QMenu(main_window);
  if (go_parent_visible) {
    menu->addAction("Go to parent", this, SLOT(action_go_parent_triggered()));
    menu->addSeparator();
  }
  foreach(File_info i, files) {
    if (i.is_folder()) {
      QAction* a = menu->addAction(i.caption, this, SLOT(menu_action_triggered()));
      if (i.uri == path) {
        a->setEnabled(false);
      }
      a->setData(i.uri);
      menu->addAction(a);
    }
  }

  menu->popup(menu_point);
  menu_pending = false;
}

void Path_button::menu_action_triggered() {
  QAction* a = dynamic_cast<QAction*>(sender());
  if (!a) {
    qWarning("Path_button::menu_action_triggered: bad sender");
    return;
  }
  emit go_to(a->data().toString());
}

void Path_button::action_go_parent_triggered() {
  Directory d(main_window, path);
  emit go_to(d.get_parent_uri());
}
