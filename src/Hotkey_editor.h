#ifndef HOTKEY_EDITOR_H
#define HOTKEY_EDITOR_H

#include <QDialog>
#include "Hotkey_editor_model.h"
#include <QItemEditorFactory>
#include <QItemDelegate>

namespace Ui {
  class Hotkey_editor;
}

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



class Hotkey_editor : public QDialog, public QItemEditorFactory {
  Q_OBJECT
  
public:
  explicit Hotkey_editor(QList<Hotkey> hotkeys, QString settings_group);
  ~Hotkey_editor();

private slots:
  void on_buttonBox_accepted();

private:
  Ui::Hotkey_editor *ui;
  Hotkey_editor_model model;


};

#endif // HOTKEY_EDITOR_H
