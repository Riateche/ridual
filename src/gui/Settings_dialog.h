#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include "Core_ally.h"

namespace Ui {
  class Settings_dialog;
}

class Settings_dialog : public QDialog, Core_ally {
  Q_OBJECT
  
public:
  explicit Settings_dialog(Core* core);
  ~Settings_dialog();
  
private slots:
  void on_buttonBox_accepted();

  void on_reset_columns_clicked();

private:
  Ui::Settings_dialog *ui;
  void fill_columns(bool reset);
};


#endif // SETTINGS_DIALOG_H
