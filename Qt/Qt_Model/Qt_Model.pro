QT       += core gui
QT       += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ControlCenter.cpp \
    Environment.cpp \
    Input.cpp \
    LaunchSilo.cpp \
    Missile.cpp \
    ModelDispatcher.cpp \
    Output.cpp \
    RadarStation.cpp \
    add.cpp \
    auxiliary.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    ControlCenter.h \
    Environment.h \
    Input.h \
    LaunchSilo.h \
    Missile.h \
    ModelDispatcher.h \
    Output.h \
    RadarStation.h \
    add.h \
    auxiliary.h \
    mainwindow.h

FORMS += \
    add.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Resources/pause.png \
    Resources/radar.png \
    Resources/run.png \
    Resources/stop.png
