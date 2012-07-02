#include <QtCore/QCoreApplication>
#undef signals // Collides with GTK symbols
#include "gtk/gtk.h"
#include "gio/gio.h"
#define signals protected
