Ridual
======


![screenshot](https://github.com/Riateche/ridual/raw/master/doc/screenshot.png)

### Description

Ridual is dual-pane file manager for Gnome environment. 
**It's in development stage now, so using Ridual is not recommended.**

Sheduled releases:

- Half-functional release: 2012/11/01
- Fully-functional release: 2013/02/01

Main advantages of Ridual:

- High integration with Gnome environment: Ridual supports GVFS mounts, bookmarks and user dirs, 
       removable drives list, file icons, applications accotiations.
- Keyboard-oriented control: Ridual has adjustable shortcuts for everything, 
       single-window dialogs and messages.
- Usability. It's not just "yet another [file manager name] clone". Some of GUI features are innovations.
- Advanced tasks and queues management.

This application is the winner of Ubuntu AppShowdown community vote 
(see [this page](http://developer.ubuntu.com/2012/09/announcing-the-ubuntu-app-showdown-community-winners/)
for more details). We must thank all voters for their support.

Ridual is written on C++/Qt and is totally free. It's distributed under MIT license. 

### Implemented features

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
- allows grouping tasks in separate queues.
- is open source and totally free.

### Planned features

Ridual is still in development stage, so there are many important features
that aren't implemented yet. The following features will be implemented soon:
- Convenient commands for selection changing.
- Creating and unpacking archives.
- Mounting archives to browse and edit them.
- Featured file search.
- Operating drives and resources (unmount, eject etc.)
- Displaying progress in Unity taskbar.
- Setting maximal copy speed.
- Undoing operations.
- Using navigation history.
- Files preview.
- Embedded terminal.

### Installing Ridual

#### Ubuntu (all supported versions)

Ridual can be installed from [our PPA](https://launchpad.net/%7Estrahovp/+archive/ridual):

    sudo add-apt-repository ppa:strahovp/ridual
    sudo apt-get update
    sudo apt-get install ridual

#### Arch Linux

Package `ridual` is available in AUR.

#### Building from sourses

This option should be used for other Linux distributions. 
You can download 'stable' version from [Tags](https://github.com/Riateche/ridual/tags) page.
You can also clone repository to get the latest source but it can be broken or unstable.

Install build dependencies (package names are given as in Ubuntu): 

- `libglib2.0-dev`
- `libgtk2.0-dev` 
- `libqt4-dev` 
 
Then run the following commands in the folder containing the source code:

    qmake
    make
    sudo checkinstall
    
### Collaboration

Feel free to help in the application development. If you want to improve existing code, 
fix a bug, add a test, suggest a feature, or build packages for your Linux distribution, 
it's always welcome. If you want to implement new features or bring large changes to existing code, 
please contact me first.     
    

    
### Running tests

Tests are incomplete and broken sometimes, but one cat try:

    cd project_dir
    tests/run.sh

Command line options are passed to gtest, e.g.

    tests/run.sh --gtest_filter=File_list_model.\*


### See also

- [PPA](https://launchpad.net/%7Estrahovp/+archive/ridual)
- [Software Center page](https://apps.ubuntu.com/cat/applications/ridual/)
- [Article about Ridual](http://habrahabr.ru/post/149647/) (Russian)


