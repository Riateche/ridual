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
  All shortcuts will be bound to specified actions. If an action is added to
  some menu, current value of its shortcut is displayed in it
  (it's the default QAction behaviour).

  All added shortcuts appear in hotkey editor automatically. The user can edit them.
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
    Use this function if you already have QAction object (for example, added to menu).

    \param name           Unique untranslatable latin string used for storing value.
    \param action         Action to be binded. QAction::setShortcut is called when you call
                          this function. If user edits the shortcut, QAction::setShortcut
                          will be called again. The shortcut is displayed in the right part
                          of menu. action->text() is used as displayable name for shortcut.
                          Initial value of action->shortcut() is used as default value.
    */
  void add(QString name, QAction *action);

  /*! Bind shortcut to specified QAction.
    This function is like add(QString, QAction), but allows to specify the text
    displayed in the shortcut editor. You should use this function if you want
    the text in the action differ from the text for editor.
    */
  void add(QString name, QString text, QAction *action);


  /*! Helper method. Add QAction and connect it to specified slot.
    Use this function if you don't have QAction object yet.
    \param name           Untranslatable name used for storing value.
    \param text           The name displayed in hotkey editor.
    \param default_value  Default value of shortcut as returned by QKeySequence::toString.
    \param receiver       Object to be connected to shortcut. Also this object is used
                          as QAction's parent.
    \param slot           Slot to be connected to shortcut. This slot will be called each time
                          hotkey is pressed. If user edits the shortcut, changes take place
                          immediately after pressing OK button.

    \return               Created QAction object. You must not delete this object.
                          You can change action text if you want, but it doesn't affect
                          the text used in hotkey editor.


    */
  QAction* add(QString name, QString text, QString default_value, QWidget* receiver, const char* slot);

  /*!
    Create and show editor dialog.
    */
  void open_editor();


  /*! Set group name used to store settings in QSettings. Default is "hotkeys".
    You should use this function only if you doesn't like default group name.
    This function should be called before any calls of Hotkeys::add.
    */
  void set_group_name(QString n);

  /*! This function is called from Hotkey_editor to set default value of
    specified hotkey.
    */
  void set_default_value(QModelIndex index);

  /*! This function is called from Hotkey_editor to disable
    specified hotkey.
    */
  void disable_shortcut(QModelIndex index);


public slots:
  /*! Save all hotkeys using Hotkey::editor_value as the source.
    This function is used by Hotkey_editor.
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
