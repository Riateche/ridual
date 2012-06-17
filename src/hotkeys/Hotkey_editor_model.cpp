#include "Hotkey_editor_model.h"
#include <QCoreApplication>
/*
Hotkey_editor_model::Hotkey_editor_model(QList<Hotkey> commands, QString settings_group) :
  QAbstractTableModel()
{
  settings.beginGroup(settings_group);
  foreach (Hotkey h, commands) {
    hotkeys << Hotkey(h.name, settings.value(h.name, h.value).toString());
  }
}

Hotkey_editor_model::~Hotkey_editor_model() {
  settings.endGroup();
}

void Hotkey_editor_model::save() {
  foreach (Hotkey h, hotkeys) {
    settings.setValue(h.name, h.value);
  }
}

*/
