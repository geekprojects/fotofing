
SUBDIRS=widgets dialogs library edit

bin_PROGRAMS=fotofing

fotofing_SOURCES=main.cpp ui.cpp window.cpp mainwindow.cpp photodetails.cpp uiutils.cpp
fotofing_CPPFLAGS=`pkg-config gtkmm-3.0 --cflags`
fotofing_LDFLAGS=../libfotofing/libfotofing.la ../libfotofing-workflow/libfotofing-workflow.la widgets/libwidgets.la dialogs/libdialogs.la library/liblibrary.la edit/libedit.la `pkg-config gtkmm-3.0 --libs`


