#include "Hotkey_editor.h"
#include "ui_Hotkey_editor.h"
#include <QAbstractTableModel>
#include <QItemDelegate>
#include "Hotkey_text_edit.h"
#include <QDebug>
#include "Hotkey_item_delegate.h"
#include "Hotkeys.h"

#include <QPushButton> //debug
Hotkey_editor::Hotkey_editor(Hotkeys *p_parent) :
  QDialog(),
  ui(new Ui::Hotkey_editor),
  hotkeys(p_parent)
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  ui->setupUi(this);
  ui->table->setModel(hotkeys);
  //set item delegate to add hotkey editor to second column
  ui->table->setItemDelegateForColumn(1, new Hotkey_item_delegate(this));
  connect(ui->button_box, SIGNAL(accepted()), hotkeys, SLOT(save()));
}

Hotkey_editor::~Hotkey_editor() {
  delete ui;
}
