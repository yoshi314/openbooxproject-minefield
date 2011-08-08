TEMPLATE = app
static {
    TARGET = obx_explorer.oar
} else {
    TARGET = obx_explorer
}
QT += core \
    gui \
    webkit \
    sql \
    dbus
HEADERS += file_clipboard.h \
    database_utils.h \
    file_system_utils.h \
    tree_view.h \
    about_dialog.h \
    boox_action.h \
    obx_explorer.h \
    explorer_view.h
SOURCES += file_clipboard.cpp \
    database_utils.cpp \
    file_system_utils.cpp \
    tree_view.cpp \
    about_dialog.cpp \
    boox_action.cpp \
    obx_explorer.cpp \
    main.cpp \
    explorer_view.cpp
FORMS += 
RESOURCES += obx_explorer.qrc
static {
    LIBS += /opt/onyx/arm/lib/static/libonyx_sys.a \
        /opt/onyx/arm/lib/static/libonyx_wireless.a \
        /opt/onyx/arm/lib/static/libonyx_wpa.a \
        /opt/onyx/arm/lib/static/libonyx_ui.a \
        /opt/onyx/arm/lib/static/libonyx_data.a \
        /opt/onyx/arm/lib/static/libonyx_cms.a \
        /opt/onyx/arm/lib/static/libonyx_screen.a
} else {
    LIBS += -lonyx_sys \
        -lonyx_wireless \
        -lonyx_wpa \
        -lonyx_ui \
        -lonyx_data \
        -lonyx_cms \
        -lonyx_screen
}
