#include "Hotkey_item_delegate.h"
#include "Hotkey_text_edit.h"
#include <QKeyEvent>
#include <QDebug>

QWidget *Hotkey_item_delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  Hotkey_text_edit* e = new Hotkey_text_edit(parent);
  //e->setText(index.data(Qt::DisplayRole).toString());
  return e;
}

bool Hotkey_item_delegate::eventFilter(QObject *object, QEvent *event) {
  //reimplement eventFilter to do nothing on key press:
  //we don't want to close editor on Tab or Esc
  if (event->type() == QEvent::KeyPress) {
    return false;
  }

  return QItemDelegate::eventFilter(object, event);
}

void Hotkey_item_delegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                        const QModelIndex &index) const {
  model->setData(index, dynamic_cast<Hotkey_text_edit*>(editor)->toPlainText());
  qDebug() << "Hotkey_item_delegate::setModelData";
}
