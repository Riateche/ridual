#ifndef HOTKEYS_H
#define HOTKEYS_H

#include <QAbstractTableModel>
#include "Hotkey.h"
#include <QSettings>

class Hotkeys : public QAbstractTableModel {
  Q_OBJECT
public:
  explicit Hotkeys(QWidget *parent = 0);
  ~Hotkeys();
  QShortcut* add(QString name, QString default_value, QObject* receiver, const char* slot);
  void add(QString name, QString default_value, QAction* action);
  void set_group_name(QString n);
  void save();
  void open_editor();

private:
  QWidget* parent_widget;
  QList<Hotkey*> hotkeys;
  QString group_name;

  static const int column_name = 0;
  static const int column_value = 1;

  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

};

#endif // HOTKEYS_H
