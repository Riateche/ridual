#ifndef PATH_BUTTON_H
#define PATH_BUTTON_H

#include <QToolButton>

class Main_window;

class Path_button : public QToolButton {
  Q_OBJECT
public:
  explicit Path_button(Main_window* mw, QString text, QString path);

  void contextMenuEvent(QContextMenuEvent *e);
signals:
  
public slots:

private:
  Main_window* main_window;
  QString path;

private slots:

};

#endif // PATH_BUTTON_H
