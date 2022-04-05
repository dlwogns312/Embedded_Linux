CC=arm-none-linux-gnueabi-gcc

.SUFFIXES: .c .h .o

DEPS=main.h
CFLAGS= -static #-W

TARGET = test
OBJDIR = build/

_OBJS = input_process.o output_process.o main.o device.o
OBJS = $(addprefix $(OBJDIR), $(_OBJS))

$(OBJDIR)$(TARGET): $(OBJS)
	$(CC) -static -o $(OBJDIR)$(TARGET) $(OBJS)

$(OBJDIR)%.o: %.c $(OBJDIR)
	$(CC) -static -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

.PHONY: clean
clean:
	rm -rf $(OBJDIR)

.PHONY: push
push:
	make
	adb push $(OBJDIR)$(TARGET) /data/local/tmp