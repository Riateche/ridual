#include "Special_uri.h"
#include <QObject>

Special_uri::Special_uri(Special_uri::Name name): _name(name) {
}

QString Special_uri::uri() {
  switch(_name) {
    case places: return "places";
    case mounts:
      qWarning("Special_uri::uri is not available for mounts");
      return "places";
    default:
      qWarning("Special_uri::uri failed");
      return QString();
  }
}

QString Special_uri::caption() {
  switch(_name) {
    case places:      return QObject::tr("Places");
    case mounts:
      qWarning("Special_uri::caption is not available for mounts");
      return QString();
    default:
      qWarning("Special_uri::caption failed");
      return QString();
  }

}

Special_uri::Special_uri(QString uri) {
  _name = _invalid;
  if (uri == "places")            _name = places;
  if (uri.startsWith("places/mounts/")) {
    _name = mounts;
  }
}

Special_uri::operator bool() const {
  return _name != _invalid;
}
