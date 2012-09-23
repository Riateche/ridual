#include "Path_button.h"
#include <QMenu>
#include "Directory.h"
#include "Special_uri.h"
#include <QContextMenuEvent>

Path_button::Path_button(Core *c, QString text, QString p_uri)
: QToolButton(0)
, Core_ally(c)
, uri(p_uri)
, parent_directory(0)
, go_parent_visible(false)
{
  setText(text);
  setToolTip(uri);
  setCheckable(true);
  connect(this, SIGNAL(clicked()), this, SLOT(slot_clicked()));
  //setContextMenuPolicy(Qt::CustomContextMenu);
}

void Path_button::contextMenuEvent(QContextMenuEvent *e) {
  if (Special_uri(uri).name() == Special_uri::places) {
    QMenu* menu = new QMenu(this);
    menu->addAction(Special_uri(Special_uri::places).caption())->setEnabled(false);
    menu->exec(e->globalPos());
    return;
  }
  if (parent_directory == 0) {
    Directory d(core, uri);
    parent_directory = new Directory(core, d.get_parent_uri());
    connect(parent_directory, SIGNAL(ready(File_info_list)),
            this, SLOT(directory_ready(File_info_list)));
  }
  menu_point = e->globalPos();
  menu_pending = true;
  parent_directory->refresh();
}

void Path_button::slot_clicked() {
  emit go_to(uri);
}

void Path_button::directory_ready(File_info_list files) {
  if (!menu_pending) return;
  QMenu* menu = new QMenu();
  if (go_parent_visible) {
    menu->addAction("Go to parent", this, SLOT(action_go_parent_triggered()));
    menu->addSeparator();
  }
  foreach(File_info i, files) {
    if (i.is_folder) {
      QAction* a = menu->addAction(i.name, this, SLOT(menu_action_triggered()));
      if (i.uri == uri) {
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
  Directory d(core, uri);
  emit go_to(d.get_parent_uri());
}
