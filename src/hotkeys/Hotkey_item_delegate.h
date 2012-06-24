#ifndef HOTKEY_ITEM_DELEGATE_H
#define HOTKEY_ITEM_DELEGATE_H

#include <QItemDelegate>

/*! Item delegate for editing hotkeys.
  */
class Hotkey_item_delegate: public QItemDelegate {
public:
  Hotkey_item_delegate(QObject* parent): QItemDelegate(parent) {}
  //createEditor is reimplemented to set Hotkey_text_edit as editor widget
  virtual QWidget* createEditor(QWidget* parent,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index ) const;

private:
  //eventFilter is reimplemented to do nothing on key press:
  //we don't want to close editor on Tab or Esc
  bool eventFilter(QObject *object, QEvent *event);

  //setModelData is reimplemented to set data based on QLineEdit::toPlainText
  //instead of QListEdit::toHtml
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const;

};

#endif // HOTKEY_ITEM_DELEGATE_H
