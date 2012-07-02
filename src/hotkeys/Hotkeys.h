#ifndef HOTKEYS_H
#define HOTKEYS_H

#include <QAbstractTableModel>
#include "Hotkey.h"
#include <QSettings>

/*!
  Main class for using hotkeys subsystem.
  All shortcuts available in the application must be added using
  Hotkeys::add on program startup. You must specify default value
  of any shortcut (or empty string if you want it to be disabled by default).
  All shortcuts will be bound to specified slots or actions.

  All added shortcuts appear in hotkey editor automatically. User can edit them.
  All modifications apply immediately after pressing OK button.
  User settings will be stored using QSettings.

  This class is also a model for editor dialog list view.
*/
class Hotkeys : public QAbstractTableModel {
  Q_OBJECT
public:
  explicit Hotkeys(QWidget *parent = 0);
  ~Hotkeys();


  /*! Bind shortcut to specified QAction.
    \param name           Untranslatable name used for storing value.
    \param action         Object to be binded. QAction::setShortcut is called when you call
                          this function. If user edits the shortcut, QAction::setShortcut
                          will be called again. The shortcut is displayed in the right part
                          of menu. action->text() is used as displayable name for shortcut.
                          Initial value of action->shortcut() is used as default value.
    */
  void add(QString name, QAction *action);

  /*! Helper method. Add QAction and connect it to specified slot.
    \param name           Untranslatable name used for storing value.
    \param text           The name displayed in hotkey editor.
    \param default_value  Default value of shortcut as returned by QKeySequence::toString.
    \param receiver       Object to be connected to shortcut. Also this object is used
                          as QAction's parent.
    \param slot           Slot to be connected to shortcut. This slot will be called each time
                          hotkey is pressed. If user edits the shortcut, changes take place
                          immediately after pressing OK button.
    */
  QShortcut* add(QString name, QString text, QString default_value, QObject* receiver, const char* slot);


  /*! Set group name used to store settings in QSettings. Default is "hotkeys".
    */
  void set_group_name(QString n);

  void set_default_value(QModelIndex index);
  void disable_shortcut(QModelIndex index);

  /*!
    Create and show editor dialog.
    */
  void open_editor();

public slots:
  /*! Save all hotkeys using Hotkey::editor_value as the source.
    */
  void save();

private:
  QWidget* parent_widget;
  QList<Hotkey*> hotkeys;
  QString group_name;

  static const int column_name = 0;
  static const int column_value = 1;

  //methods implemented for acting as a model
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

};

#endif // HOTKEYS_H
