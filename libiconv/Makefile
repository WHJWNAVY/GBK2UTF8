src_dir=$(pwd)
TARGET:=gen_gbk2uni_tab
OBJECTS:=gen_gbk2uni_tab.o
CFLAGS:=-Wall -Os
LDFLAGS:=
CC:=gcc

$(TARGET):$(OBJECTS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

all:$(TARGET)

clean:
	rm $(TARGET) $(OBJECTS)

lint:
	find ${src_dir} -iname "*.[ch]" | xargs clang-format -i

.PHONY:all clean
