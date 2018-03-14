TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += \
    -lpthread

SOURCES += main.cpp \
    rtp_head.cpp \
    rtp_sender.cpp \
    rtp_receiver.cpp \
    z_udp.cpp \
    z_bits.c \
    h264_nal.cpp

HEADERS += \
    rtp_head.h \
    rtp_sender.h \
    rtp_receiver.h \
    z_udp.h \
    z_bits.h \
    h264_nal.h

