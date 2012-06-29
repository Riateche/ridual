#include "Special_uri.h"
#include <QObject>

Special_uri::Special_uri(Special_uri::Name name): _name(name) {
}

QString Special_uri::uri() {
  switch(_name) {
    case places: return "places";
    case mounts: return "places/mounts";
    case bookmarks: return "places/bookmarks";
    case userdirs: return "places/userdirs";
    default: qWarning("Special_uri::uri failed"); return QString();
  }
}

QString Special_uri::caption() {
  switch(_name) {
    case places:      return QObject::tr("Places");
    case mounts:      return QObject::tr("Mounted filesystems");
    case bookmarks:   return QObject::tr("Bookmarks");
    case userdirs:    return QObject::tr("User dirs");
    default: qWarning("Special_uri::caption failed"); return QString();
  }

}

Special_uri::Special_uri(QString uri) {
  _name = _invalid;
  if (uri == "places")            _name = places;
  if (uri == "places/mounts")     _name = mounts;
  if (uri == "places/bookmarks")  _name = bookmarks;
  if (uri == "places/userdirs")   _name = userdirs;
}

Special_uri::operator bool() const {
  return _name != _invalid;
}
