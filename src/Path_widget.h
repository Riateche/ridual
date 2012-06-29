#ifndef PATH_WIDGET_H
#define PATH_WIDGET_H

#include <QWidget>
#include "Path_button.h"
#include <QGridLayout>

class Path_widget : public QWidget {
  Q_OBJECT
public:
  explicit Path_widget(QWidget *parent = 0);
  ~Path_widget();
  void set_buttons(const QList<Path_button *>& new_buttons);

private:
  QList<Path_button*> buttons;
  QVBoxLayout* v_layout;
  QList<QHBoxLayout*> h_layouts;
  void add(int row, int column, QWidget* widget);
  
signals:
  
public slots:
  void refresh();
  
};

#endif // PATH_WIDGET_H
