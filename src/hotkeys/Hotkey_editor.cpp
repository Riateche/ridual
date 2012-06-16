#include "Hotkey_editor.h"
#include "ui_Hotkey_editor.h"
#include <QAbstractTableModel>
#include <QItemDelegate>
#include "Hotkey_text_edit.h"
#include <QDebug>
#include "Hotkey_item_delegate.h"

#include <QPushButton> //debug
Hotkey_editor::Hotkey_editor(QList<Hotkey> hotkeys) :
  QDialog(),
  ui(new Ui::Hotkey_editor),
  model(hotkeys, settings_group())
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  ui->setupUi(this);
  ui->table->setModel(&model);
  //QItemDelegate* item_delegate = new QItemDelegate();
  //item_delegate->setItemEditorFactory(this);
  ui->table->setItemDelegateForColumn(1, new Hotkey_item_delegate(this));
}

Hotkey_editor::~Hotkey_editor() {
  delete ui;
}

QKeySequence Hotkey_editor::get_hotkey(QString name) {
  QSettings s;
  s.beginGroup(settings_group());
  QString v = s.value(name).toString();
  s.endGroup();
  return QKeySequence(v);
}




void Hotkey_editor::on_buttonBox_accepted() {
  model.save();
}
