src_dir=$(pwd)
TARGET:=gbk2utf8
OBJECTS:=main.o gbk2uni.o
CFLAGS:=-Os
# CFLAGS+=-Wall
# LDFLAGS:=
CC:=gcc

$(TARGET):$(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

all:$(TARGET)

clean:
	rm $(TARGET) $(OBJECTS)

lint:
	find ${src_dir} -iname "*.[ch]" | xargs clang-format -i

.PHONY:all clean
