
CC = gcc
CFLAGS = -fsanitize=address -g
MACRO_DEMO = -DDEMO
MACRO_DEBUG = -DDEBUG
MACRO_OJ = -DOJ

DEMO_OBJS = _demo
OJ_OBJS = _oj
DEBUG_OBJS = _debug

OBJS = $(DEMO_OBJS) $(OJ_OBJS) $(DEBUG_OBJS)
all : $(OBJS)

$(DEMO_OBJS): nsh.c
	$(CC) $(CFLAGS) $(MACRO_DEMO) -o nsh-demo nsh.c

$(DEBUG_OBJS): nsh.c
	$(CC) $(CFLAGS) $(MACRO_DEBUG) -o nsh-debug nsh.c

$(OJ_OBJS): nsh.c
	$(CC) $(CFLAGS) $(MACRO_OJ) -o nsh-oj nsh.c

demo: $(DEMO_OBJS)
	./nsh-demo

oj: $(OJ_OBJS)
	./nsh-oj

debug: $(DEBUG_OBJS)
	./nsh-debug

clean:
	rm -f $(OBJS)
