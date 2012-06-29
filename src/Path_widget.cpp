#include "Path_widget.h"

Path_widget::Path_widget(QWidget *parent) :
  QWidget(parent),
  v_layout(new QVBoxLayout(this))
{
  v_layout->setContentsMargins(0, 0, 0, 0);
  setLayout(v_layout);
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
}

Path_widget::~Path_widget() {
  foreach(Path_button* b, buttons) delete b;
}

void Path_widget::set_buttons(const QList<Path_button *>& new_buttons) {
  foreach(Path_button* b, buttons) delete b;
  buttons = new_buttons;
  refresh();
}

void Path_widget::add(int row, int column, QWidget *widget) {
  while (row >= h_layouts.count()) {
    h_layouts << new QHBoxLayout();
    v_layout->addLayout(h_layouts.last());
  }
  if (!widget) {
    h_layouts[row]->addStretch();
  } else {
    h_layouts[row]->insertWidget(column, widget);
  }
}

void Path_widget::refresh() {
/*  int c = layout->count();
  for(int i = 0; i < c; i++) layout->removeItem(layout->itemAt(i));
  foreach(QHBoxLayout* l, h_layouts) {
    int c = l->count();
    for(int i = 0; i < c; i++) l->removeItem(layout->itemAt(i));
  } */

  int left_margin, right_margin;
  v_layout->getContentsMargins(&left_margin, 0, &right_margin, 0);
  int current_width = width() - left_margin - right_margin;
  if (current_width < 10) return;
  int x = 0;
  int row = 0, column = 0;
  foreach(Path_button* button, buttons) {
    if (column > 0) x += v_layout->spacing();
    x += button->sizeHint().width();
    if (x > current_width) {
      add(row, column, 0);
      row++;
      column = 0;
      x = button->sizeHint().width();
    }
    add(row, column, button);
    column++;
  }
  add(row, column, 0);
}
