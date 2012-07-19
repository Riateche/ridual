ridual
======

Ridual is dual-pane file manager for Ubuntu.
There are quite few dual-pane file managers for Linux. This one is
developed especially for Ubuntu. We're trying to achieve high
integration with Ubuntu-specific features.

![screenshot](https://github.com/Riateche/ridual/raw/master/doc/screenshot.png)

Ridual will become the best of its kind because it:
- uses system color theme. You will see no crazy colors.
- allows to use and change hotkeys for every action.
- displays file icons from system theme based on mime type.
- executes all filesystem operations in separate thread.
- supports GIO/GVFS mount points. It means you can mount and browse
any external drive or network filesystem (such as FTP, SMB, SFTP etc.) just as
it can be done in Nautilus. (Note that you can use resources which are already mounted or
can be mounted without asking a password. Password dialogs for GIO aren't implemented yet.)
- has useful navigation widget with buttons for each part of current path. You can
navigate to each of them by clicking on buttons. You can also navigate to another folder
by using buttons' context menu.
- can open any file in the program accosiated with them accordingly to gnome settings. You
can also choose another program in file's context menu.
- supports gnome bookmarks and user directories (Documents, Desktop, Music etc.)
- works in UTF-8 encoding.
- supports translations (but there is no translations yet).
- is open source and totally free.

Ridual is still in development stage, so there are many important features
that aren't implemented yet. The following features will be implemented soon:
- Copying, moving files, creating links etc.
- Grouping tasks in separate queues.
- Convenient commands for selection changing.
- Creating and unpacking archives.
- Mounting archives to browse and edit them.
- Featured file search.
- Operating drives and resources (unmount, eject etc.)
- Displaying progress in Unity taskbar.
- Settings maximal copy speed.
- Undoing operations.
- Using vavigation history.

### Installing Ridual

If you're using Ubuntu Precise, you can install Ridual from [our PPA](https://launchpad.net/%7Estrahovp/+archive/ridual).

You also can download sources and run the following commands:

    qmake
    make
    sudo checkinstall

Ridual is written on C++/Qt and is distributed under MIT license (see LICENSE file). 

### Running tests

    cd test-build-directory
    qmake path-to-project-dir CONFIG+=testing && make && ./ridual-test


