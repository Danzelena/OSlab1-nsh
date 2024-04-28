# nsh
> NJUAI OS lab1
[TOC]

## how to build and run it

### with Makefile
use
```shell
cd ./src/submit
```
to the directionary has `nsh.c`
then use

```shell
make
```

to build `nsh-demo`, `nsh-debug`, `nsh-oj`, which:

- `nsh-demo` is the final version of nsh
- `nsh-oj` is the version of nsh for online judge
- `nsh-debug` is the version of nsh for debugging
  Then you can choose on to run it by

```shell
./nsh-*
```

where `*` stand for `demo`, `oj` or `debug`.

> You can also use
>
> ```shell
> make demo
> ```
>
> ```shell
> make oj
> ```
>
> ```shell
> make debug
> ```
>
> to build the corresponding version of nsh, and run it.

### without Makefile(recommended)

这个是 `Makefile` , 在同一个目录下新建一个 `Makefile` 输入以下内容即可

```Makefile
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

```

### without Makefile (not recommended)

也可以直接修改 `nsh.c` 里面的

```c
/* define build target to open debug-mode and demo-mode */
// #define OJ
// #define DEMO
// #define DEBUG
```

按需求开启后使用

```shell
gcc -fsanitize=address -g -o nsh nsh.c
```

编译并运行

```shell
./nsh
```

即可.
## Acknowledge

通过本次实验, 我对系统调用和 shell 有了更深入的认识, 并且~~毫无意外的掉入了 **并发** 的大坑(bushi)~~, 

本次实验全部代码都是本人编写的(~~因此有点屎山~~), 下列链接中的内容在实验过程中提供了很多参考:

- [OS lab1]([实现一个简单的 shell | Operating System (nju.edu.cn)](https://gist.nju.edu.cn/course-os/docs/labs/lab1.html)) 提供了实验的要求以及基本设计思路

- [CSAPP 上的 shell lab]([实验 7：Shell Lab | 深入理解计算机系统（CSAPP） (gitbook.io)](https://hansimov.gitbook.io/csapp/labs/shell-lab)) 提供了一些设计 shell 的基本思路

- [jyy老师的精彩讲解]([Yanyan's Wiki (jyywiki.cn)](https://jyywiki.cn/OS/2024/lect16.md)) 提供了对输入进行语法树分析的一些启发

- [ICS pa 的一些启发 ]([表达式求值 · GitBook (nju-projectn.github.io)](https://nju-projectn.github.io/ics-pa-gitbook/ics2023/1.5.html)) 一些代码编写的 trick (比如使用 宏, 写 Makefile), 以及表达式求值部分的一些启发
