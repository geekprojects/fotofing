
SUBDIRS=widgets

bin_PROGRAMS=fotofing

fotofing_SOURCES=main.cpp mainwindow.cpp library.cpp photodetails.cpp photoview.cpp calendarpopup.cpp about.cpp sourcesdialog.cpp tagdialog.cpp uiutils.cpp
fotofing_CPPFLAGS=`pkg-config gtkmm-3.0 --cflags`
fotofing_LDFLAGS=../libfotofing/libfotofing.la widgets/libwidgets.la `pkg-config gtkmm-3.0 --libs`


