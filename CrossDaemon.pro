QT -= gui
QT += network

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += qtExample/main.cpp \
	src/daemon.cpp \
    src/winservice.cpp \
	src/unixdaemon.cpp \
    qtExample/qtdaemon.cpp \
    qtExample/webserver.cpp \
    qtExample/webservercontroller.cpp

HEADERS += \
    src/daemon.h \
    src/winservice.h \
	src/unixdaemon.h \
    src/daemongeneric.h \
    qtExample/qtdaemon.h \
    qtExample/webserver.h \
    qtExample/webservercontroller.h
