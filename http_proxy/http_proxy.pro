TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    connection.c \
    address_utils.c \
    cache.c \
    http_utils.c \
    logger.c \
    hash_table.c

HEADERS += \
    common.h \
    connection.h \
    address_utils.h \
    cache.h \
    http_utils.h \
    logger.h \
    hash_table.h

LIBS += -pthread
