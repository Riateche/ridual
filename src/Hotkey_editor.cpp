#include "Hotkey_editor.h"
#include "ui_Hotkey_editor.h"
#include <QAbstractTableModel>
#include <QItemDelegate>
#include "Hotkey_text_edit.h"
#include <QDebug>

#include <QPushButton> //debug
Hotkey_editor::Hotkey_editor(QList<Hotkey> hotkeys, QString settings_group) :
  QDialog(),
  ui(new Ui::Hotkey_editor),
  model(hotkeys, settings_group)
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

//Hotkey_item_delegate::~Hotkey_item_delegate() {
//  qDebug() << "~Hotkey_item_delegate";
//}

QWidget *Hotkey_item_delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  Hotkey_text_edit* e = new Hotkey_text_edit(parent);
  //ui->table->installEventFilter(e);
  //ui->table->itemDelegate()->installEventFilter(e);
  return e;
}

bool Hotkey_item_delegate::eventFilter(QObject *object, QEvent *event) {
  //reimplement eventFilter to do nothing on key press:
  //we don't want to close editor on Tab or Esc
  if (event->type() == QEvent::KeyPress) {
    return false;
  }

  QItemDelegate::eventFilter(object, event);
}

void Hotkey_item_delegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                        const QModelIndex &index) const {
  model->setData(index, dynamic_cast<Hotkey_text_edit*>(editor)->toPlainText());
  qDebug() << "Hotkey_item_delegate::setModelData";
}

void Hotkey_editor::on_buttonBox_accepted() {
  model.save();
}
