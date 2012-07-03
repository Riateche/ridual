#include "Settings_dialog.h"
#include "ui_Settings_dialog.h"
#include <QSettings>

Settings_dialog::Settings_dialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::Settings_dialog)
{
  ui->setupUi(this);
  QSettings s;
  ui->edit_command->setText(s.value("edit_command", "gedit %U").toString());
  ui->view_command->setText(s.value("view_command", "gedit %U").toString());
}

Settings_dialog::~Settings_dialog() {
  delete ui;
}

void Settings_dialog::on_buttonBox_accepted() {
  QSettings s;
  s.setValue("edit_command", ui->edit_command->text());
  s.setValue("view_command", ui->view_command->text());
  close();
}
