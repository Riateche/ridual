#include "Path_button.h"
#include "Main_window.h"
#include <QMenu>

#include <QContextMenuEvent>

Path_button::Path_button(Main_window* mw, QString text, QString p_path) :
  QToolButton(mw),
  main_window(mw),
  path(p_path)
{
  setText(text);
  setToolTip(path);
  //setContextMenuPolicy(Qt::CustomContextMenu);
}

void Path_button::contextMenuEvent(QContextMenuEvent *e) {
  QMenu* menu = new QMenu(this);
  menu->addAction("test1");
  menu->addAction("test2");
  menu->exec(e->globalPos());
}
