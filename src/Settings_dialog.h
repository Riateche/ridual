#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>

namespace Ui {
  class Settings_dialog;
}

class Settings_dialog : public QDialog {
  Q_OBJECT
  
public:
  explicit Settings_dialog(QWidget *parent = 0);
  ~Settings_dialog();
  
private slots:
  void on_buttonBox_accepted();

private:
  Ui::Settings_dialog *ui;
};

#endif // SETTINGS_DIALOG_H
