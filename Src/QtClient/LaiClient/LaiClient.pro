QT      += core gui
QT      += multimedia
CONFIG  += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LaiClient
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += /usr/local/ffmpeg/include
LIBS+= -L/usr/local/ffmpeg/lib \
       -lavcodec \
       -lavdevice \
       -lavfilter \
       -lavformat \
       -lavutil \
       -lswresample \
       -lswscale

SOURCES += \
        /home/lai/opencv3/EasyPR/src/core/chars_identify.cpp \
        /home/lai/opencv3/EasyPR/src/core/chars_recognise.cpp \
        /home/lai/opencv3/EasyPR/src/core/chars_segment.cpp \
        /home/lai/opencv3/EasyPR/src/core/core_func.cpp \
        /home/lai/opencv3/EasyPR/src/core/feature.cpp \
        /home/lai/opencv3/EasyPR/src/core/params.cpp \
        /home/lai/opencv3/EasyPR/src/core/plate_detect.cpp \
        /home/lai/opencv3/EasyPR/src/core/plate_judge.cpp \
        /home/lai/opencv3/EasyPR/src/core/plate_locate.cpp \
        /home/lai/opencv3/EasyPR/src/core/plate_recognize.cpp \
        /home/lai/opencv3/EasyPR/src/train/ann_train.cpp \
        /home/lai/opencv3/EasyPR/src/train/annCh_train.cpp \
        /home/lai/opencv3/EasyPR/src/train/create_data.cpp \
        /home/lai/opencv3/EasyPR/src/train/svm_train.cpp \
        /home/lai/opencv3/EasyPR/src/train/train.cpp \
        /home/lai/opencv3/EasyPR/src/util/kv.cpp \
        /home/lai/opencv3/EasyPR/src/util/program_options.cpp \
        /home/lai/opencv3/EasyPR/src/util/util.cpp \
        /home/lai/opencv3/EasyPR/thirdparty/LBP/helper.cpp \
        /home/lai/opencv3/EasyPR/thirdparty/LBP/lbp.cpp \
        /home/lai/opencv3/EasyPR/thirdparty/mser/mser2.cpp \
        /home/lai/opencv3/EasyPR/thirdparty/svm/corrected_svm.cpp \
        /home/lai/opencv3/EasyPR/thirdparty/textDetect/erfilter.cpp \
        /home/lai/opencv3/EasyPR/thirdparty/xmlParser/xmlParser.cpp \
        main.cpp \
        Widget/laiwidget.cpp \
        Tool/myitem.cpp \
        Tool/singledb.cpp \
        Thread/thread.cpp \
        Widget/set.cpp \
        Widget/login.cpp \
        Widget/register.cpp \
        Widget/main_interface.cpp \
        Widget/main_in.cpp \
        Widget/main_out.cpp \
        Widget/main_inmanage.cpp \
        Widget/main_equery.cpp \
        Widget/main_playback.cpp \
        Thread/thread_in.cpp \
        Sqlite/singledb_sqlite.cpp \
        Sqlite/car_model.cpp \
        Thread/thread_out.cpp \
        Thread/thread_inmanage.cpp \
        Widget/main_replay.cpp \
        Ffmpeg/ffmpeg_decode.cpp

HEADERS += \
        /home/lai/opencv3/EasyPR/include/easypr/core/character.hpp \
        /home/lai/opencv3/EasyPR/include/easypr/core/chars_identify.h \
        /home/lai/opencv3/EasyPR/include/easypr/core/chars_recognise.h \
        /home/lai/opencv3/EasyPR/include/easypr/core/chars_segment.h \
        /home/lai/opencv3/EasyPR/include/easypr/core/core_func.h \
        /home/lai/opencv3/EasyPR/include/easypr/core/feature.h \
        /home/lai/opencv3/EasyPR/include/easypr/core/params.h \
        /home/lai/opencv3/EasyPR/include/easypr/core/plate.hpp \
        /home/lai/opencv3/EasyPR/include/easypr/core/plate_detect.h \
        /home/lai/opencv3/EasyPR/include/easypr/core/plate_judge.h \
        /home/lai/opencv3/EasyPR/include/easypr/core/plate_locate.h \
        /home/lai/opencv3/EasyPR/include/easypr/core/plate_recognize.h \
        /home/lai/opencv3/EasyPR/include/easypr/train/ann_train.h \
        /home/lai/opencv3/EasyPR/include/easypr/train/annCh_train.h \
        /home/lai/opencv3/EasyPR/include/easypr/train/create_data.h \
        /home/lai/opencv3/EasyPR/include/easypr/train/svm_train.h \
        /home/lai/opencv3/EasyPR/include/easypr/train/train.h \
        /home/lai/opencv3/EasyPR/include/easypr/util/kv.h \
        /home/lai/opencv3/EasyPR/include/easypr/util/program_options.h \
        /home/lai/opencv3/EasyPR/include/easypr/util/switch.hpp \
        /home/lai/opencv3/EasyPR/include/easypr/util/util.h \
        /home/lai/opencv3/EasyPR/include/easypr/api.hpp \
        /home/lai/opencv3/EasyPR/include/easypr/config.h \
        /home/lai/opencv3/EasyPR/include/easypr/version.h \
        /home/lai/opencv3/EasyPR/include/easypr.h \
        /home/lai/opencv3/EasyPR/test/accuracy.hpp \
        /home/lai/opencv3/EasyPR/test/chars.hpp \
        /home/lai/opencv3/EasyPR/test/config.hpp \
        /home/lai/opencv3/EasyPR/test/plate.hpp \
        /home/lai/opencv3/EasyPR/test/result.hpp \
        /home/lai/opencv3/EasyPR/thirdparty/LBP/helper.hpp \
        /home/lai/opencv3/EasyPR/thirdparty/LBP/lbp.hpp \
        /home/lai/opencv3/EasyPR/thirdparty/mser/mser2.hpp \
        /home/lai/opencv3/EasyPR/thirdparty/svm/precomp.hpp \
        /home/lai/opencv3/EasyPR/thirdparty/textDetect/erfilter.hpp \
        /home/lai/opencv3/EasyPR/thirdparty/xmlParser/xmlParser.h \
        Widget/laiwidget.h \
        Tool/myitem.h \
        Tool/singledb.h \
        Thread/thread.h \
        Widget/set.h \
        Widget/login.h \
        Widget/register.h \
        Widget/main_interface.h \
        Widget/main_in.h \
        Widget/main_out.h \
        Widget/main_inmanage.h \
        Widget/main_equery.h \
        Widget/main_playback.h \
        Thread/thread_in.h \
        Sqlite/singledb_sqlite.h \
        Sqlite/car_model.h \
        Thread/thread_out.h \
        Thread/thread_inmanage.h \
        Widget/main_replay.h \
        Ffmpeg/ffmpeg_decode.h

RESOURCES += \
        png.qrc

FORMS += \
        Widget/set.ui \
        Widget/login.ui \
        Widget/register.ui \
        Widget/main_interface.ui \
        Widget/main_in.ui \
        Widget/main_out.ui \
        Widget/main_inmanage.ui \
        Widget/main_equery.ui \
        Widget/main_playback.ui \
        Widget/main_replay.ui

INCLUDEPATH += /home/lai/opncv3/EasyPR/
INCLUDEPATH += /usr/local/include/
INCLUDEPATH += /home/lai/opencv3/EasyPR/include/
LIBS += /usr/local/lib/libopencv_world.so
LIBS += /home/lai/opencv3/EasyPR/build/libeasypr.a
LIBS += -lsqlite3

DISTFILES += \
        model/ann.xml \
        model/ann_chinese.xml \
        model/annCh.xml \
        model/svm_hist.xml \
        model/province_mapping
