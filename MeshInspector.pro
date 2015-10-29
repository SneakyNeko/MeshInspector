TEMPLATE = app
TARGET = meshinspect
QT += widgets
INCLUDEPATH += .
QMAKE_CFLAGS += -std=c99
LIBS += -lproj

# Input
HEADERS += AppWindow.h    \
           Histogram.h    \
           NodeList.h     \
           nodeUtil.h     \
           StatsTable.h   \
           MeshView.h

SOURCES += main.cpp       \
           AppWindow.cpp  \
           Histogram.cpp  \
           NodeList.cpp   \
           nodeUtil.c     \
           nodeStats.c    \
           StatsTable.cpp \
           MeshView.cpp

