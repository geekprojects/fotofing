
TGT=schema
SRCS=schema.cpp
OBJS=$(SRCS:.cpp=.o)

all: $(OBJS)
	gcc $(OBJS) -o $(TGT) -lstdc++ -Wl,-rpath=$(PWD)/../../../libfotofing -lfotofing -L../../../libfotofing

clean:
	rm -f $(OBJS) $(TGT)

.cpp.o:
	gcc -c -O3 -ggdb -Wall -Werror -I../../../include $<

