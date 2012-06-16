#ifndef HOTKEY_ITEM_DELEGATE_H
#define HOTKEY_ITEM_DELEGATE_H

#include <QItemDelegate>

class Hotkey_item_delegate: public QItemDelegate {
public:
  Hotkey_item_delegate(QObject* parent): QItemDelegate(parent) {}
  //~Hotkey_item_delegate();
  virtual QWidget* createEditor(QWidget* parent,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index ) const;

private:
  bool eventFilter(QObject *object, QEvent *event);
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const;

};

#endif // HOTKEY_ITEM_DELEGATE_H
