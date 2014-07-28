
TGT=fotofing
SRCS=main.cpp mainwindow.cpp photodetails.cpp calendarpopup.cpp about.cpp
OBJS=$(SRCS:.cpp=.o)

all: $(OBJS)
	gcc $(OBJS) -o $(TGT) -lstdc++ `pkg-config gtkmm-3.0 --libs` -Wl,-rpath=$(PWD)/../libfotofing -lfotofing -L../libfotofing

clean:
	rm -f $(OBJS) $(TGT)

.cpp.o:
	gcc -c -O3 -ggdb -Wall -Werror -I../include `pkg-config gtkmm-3.0 --cflags` $<

