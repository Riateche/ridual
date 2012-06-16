#ifndef HOTKEY_EDITOR_H
#define HOTKEY_EDITOR_H

#include <QDialog>
#include "Hotkey_editor_model.h"
#include <QItemEditorFactory>

namespace Ui {
  class Hotkey_editor;
}

class Hotkey_editor : public QDialog, public QItemEditorFactory {
  Q_OBJECT
  
public:
  explicit Hotkey_editor(QList<Hotkey> hotkeys, QString settings_group);
  ~Hotkey_editor();
  
private:
  Ui::Hotkey_editor *ui;
  Hotkey_editor_model model;

  QWidget* createEditor(QVariant::Type type, QWidget* parent) const;

};

#endif // HOTKEY_EDITOR_H
