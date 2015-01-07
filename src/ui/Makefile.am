
SUBDIRS=widgets dialogs library

bin_PROGRAMS=fotofing

fotofing_SOURCES=main.cpp mainwindow.cpp photodetails.cpp uiutils.cpp
fotofing_CPPFLAGS=`pkg-config gtkmm-3.0 --cflags`
fotofing_LDFLAGS=../libfotofing/libfotofing.la widgets/libwidgets.la dialogs/libdialogs.la library/liblibrary.la `pkg-config gtkmm-3.0 --libs`


