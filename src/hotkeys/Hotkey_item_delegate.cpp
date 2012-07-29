#include "Hotkey_item_delegate.h"
#include "Hotkey_text_edit.h"
#include <QKeyEvent>
#include <QDebug>

QWidget *Hotkey_item_delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  return new Hotkey_text_edit(parent);
  Q_UNUSED(option);
  Q_UNUSED(index);
}

bool Hotkey_item_delegate::eventFilter(QObject *object, QEvent *event) {
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
