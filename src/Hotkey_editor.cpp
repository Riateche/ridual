#include "Hotkey_editor.h"
#include "ui_Hotkey_editor.h"
#include <QAbstractTableModel>
#include <QItemDelegate>
#include "Hotkey_text_edit.h"

#include <QPushButton> //debug
Hotkey_editor::Hotkey_editor(QList<Hotkey> hotkeys, QString settings_group) :
  QDialog(),
  ui(new Ui::Hotkey_editor),
  model(hotkeys, settings_group)
{
  ui->setupUi(this);
  ui->table->setModel(&model);
  QItemDelegate* item_delegate = new QItemDelegate();
  item_delegate->setItemEditorFactory(this);
  ui->table->setItemDelegateForColumn(1, item_delegate);
}

Hotkey_editor::~Hotkey_editor() {
  delete ui;
}

QWidget *Hotkey_editor::createEditor(QVariant::Type type, QWidget *parent) const {
  return new Hotkey_text_edit(parent);
}
