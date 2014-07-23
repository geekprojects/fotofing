
TGT=fotofing
SRCS=main.cpp mainwindow.cpp index.cpp photo.cpp file.cpp calendarpopup.cpp database.cpp utils.cpp sha1.cpp
OBJS=$(SRCS:.cpp=.o)

all: $(OBJS)
	gcc $(OBJS) -o $(TGT) -lstdc++ -lbeyond -lm -lsqlite3 `pkg-config gtkmm-3.0 --libs` -lexiv2

clean:
	rm -f $(OBJS) $(TGT)

.cpp.o:
	gcc -c -O3 -ggdb -Wall -Werror `pkg-config gtkmm-3.0 --cflags` $<

