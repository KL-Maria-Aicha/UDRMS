QT += core gui sql widgets svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG  += c++17
TARGET   = UDRMS
TEMPLATE = app

SOURCES += \
    main.cpp \
    databasemanager.cpp \
    loginwindow.cpp \
    mainwindow.cpp \
    studentdialog.cpp \
    roomdialog.cpp \
    menuitemdialog.cpp \
    dormdialog.cpp

HEADERS += \
    person.h \
    student.h \
    room.h \
    menuitem.h \
    restaurant.h \
    dormitory.h \
    user.h \
    databasemanager.h \
    loginwindow.h \
    mainwindow.h \
    studentdialog.h \
    roomdialog.h \
    menuitemdialog.h \
    dormdialog.h

RESOURCES += resources.qrc
