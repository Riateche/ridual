#include "Settings_dialog.h"
#include "ui_Settings_dialog.h"
#include <QSettings>
#include <QStandardItemModel>
#include "Columns.h"
#include "Core.h"
#include "Main_window.h"

Settings_dialog::Settings_dialog(Core* core) :
  Core_ally(core),
  ui(new Ui::Settings_dialog)
{
  ui->setupUi(this);

  QSettings s;
  ui->edit_command->setText(s.value("edit_command", "gedit %U").toString());
  ui->view_command->setText(s.value("view_command", "gedit %U").toString());
  ui->show_folders_before_files->setChecked(s.value("sort_folders_before_files", true).toBool());
  fill_columns(false);
}

Settings_dialog::~Settings_dialog() {
  delete ui;
}

void Settings_dialog::on_buttonBox_accepted() {
  QSettings s;
  s.setValue("edit_command", ui->edit_command->text());
  s.setValue("view_command", ui->view_command->text());
  s.setValue("sort_folders_before_files", ui->show_folders_before_files->isChecked());

  Columns c;
  foreach(QListWidgetItem* item,
          ui->visible_columns->findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard)) {
    c << static_cast<Column::Enum>(item->data(Qt::UserRole).toInt());
  }
  core->get_main_window()->set_columns(c);
  core->emit_settings_changed();
  close();
}

void Settings_dialog::on_reset_columns_clicked() {
  fill_columns(true);
}

void Settings_dialog::fill_columns(bool reset) {
  ui->visible_columns->clear();
  ui->hidden_columns->clear();
  Columns columns = reset? Columns::get_default(): core->get_main_window()->get_columns();
  foreach(Column::Enum c, Columns::get_all_regular()) {
    QListWidgetItem* item = new QListWidgetItem(Columns::name(c));
    item->setData(Qt::UserRole, static_cast<int>(c));
    (columns.contains(c) ? ui->visible_columns: ui->hidden_columns)->addItem(item);
  }
}
