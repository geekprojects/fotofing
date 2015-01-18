
bin_PROGRAMS=features

features_SOURCES=features_tagger.cpp features_config.cpp
#features_SOURCES=test.cpp
features_CPPFLAGS=`xml2-config --cflags`
features_LDFLAGS=../../libfotofing/libfotofing.la -lgeek-core -lopencv_core -lopencv_objdetect -lopencv_highgui -lopencv_imgproc `xml2-config --libs`

