TEMPLATE = app
TARGET = obx_explorer
QT += core \
    gui \
    webkit sql
HEADERS += tree_view.h \
    about_dialog.h \
    boox_action.h \
    obx_explorer.h \
    explorer_view.h
SOURCES += tree_view.cpp \
    about_dialog.cpp \
    boox_action.cpp \
    obx_explorer.cpp \
    main.cpp \
    explorer_view.cpp
FORMS += 
RESOURCES += obx_explorer.qrc
LIBS += -lonyx_sys \
    -lonyx_screen \
    -lonyx_ui \
    -lonyx_wireless
