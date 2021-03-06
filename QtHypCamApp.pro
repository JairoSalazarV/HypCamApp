#-------------------------------------------------
#
# Project created by QtCreator 2018-01-04T21:54:41
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HypCamApp
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        hypcamapp.cpp \
    __common.cpp \
    hypCamAPI.cpp \
    formsettings.cpp \
    rasphypcam.cpp \
    formtimertxt.cpp \
    formcreatenewfile.cpp \
    formimageedition.cpp \
    formndvisettings.cpp \
    formlstsettings.cpp \
    formlstcustoms.cpp \
    formlstcamactions.cpp \
    formlstalgorithms.cpp \
    formcameraid.cpp

HEADERS += \
        hypcamapp.h \
    __common.h \
    lstStructs.h \
    lstfilenames.h \
    lstStructs.h \
    lstcustoms.h \
    lstraspividstruct.h \
    hypCamAPI.h \
    formsettings.h \
    rasphypcam.h \
    formtimertxt.h \
    formcreatenewfile.h \
    lstpaths.h \
    formimageedition.h \
    formndvisettings.h \
    formlstsettings.h \
    formlstcustoms.h \
    formlstcamactions.h \
    formlstalgorithms.h \
    formcameraid.h

FORMS += \
        hypcamapp.ui \
    formsettings.ui \
    formtimertxt.ui \
    formcreatenewfile.ui \
    formimageedition.ui \
    formndvisettings.ui \
    formlstsettings.ui \
    formlstcustoms.ui \
    formlstcamactions.ui \
    formlstalgorithms.ui \
    formcameraid.ui

CONFIG += mobility
MOBILITY = 

RESOURCES += \
    images.qrc

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

