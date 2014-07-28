
TGT=libfotofing.so
SRCS=index.cpp photo.cpp source.cpp file.cpp database.cpp utils.cpp sha1.cpp
OBJS=$(SRCS:.cpp=.o)

all: $(OBJS)
	gcc -shared $(OBJS) -o $(TGT) -lbeyond -lm -lsqlite3 -lexiv2

clean:
	rm -f $(OBJS) $(TGT)

.cpp.o:
	gcc -c -fPIC -O3 -ggdb -Wall -Werror -I../include $<

