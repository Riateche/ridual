#ifndef HOTKEY_EDITOR_MODEL_H
#define HOTKEY_EDITOR_MODEL_H

#include <QAbstractTableModel>
#include <QSettings>

class Hotkey {
public:
  Hotkey(QString p_name, QString p_value) : name(p_name), value(p_value) {}
  QString name, value;
};

class Hotkey_editor_model : public QAbstractTableModel {
  Q_OBJECT
public:
  explicit Hotkey_editor_model(QList<Hotkey> commands, QString settings_group);
  ~Hotkey_editor_model();
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  
signals:
  
public slots:


private:
  QList<Hotkey> hotkeys;
  QSettings settings;
  static const int column_name = 0;
  static const int column_value = 1;
};

#endif // HOTKEY_EDITOR_MODEL_H
