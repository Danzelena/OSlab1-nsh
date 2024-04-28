#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int pipe1[2], pipe2[2];
    pid_t pid1, pid2;

    // 创建两个管道
    if (pipe(pipe1) < 0 || pipe(pipe2) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // 创建第一个子进程
    if ((pid1 = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        // 子进程1中执行ls命令
        close(pipe1[0]); // 关闭管道1的读端
        dup2(pipe1[1], STDOUT_FILENO); // 将标准输出重定向到管道1的写端
        close(pipe1[1]); // 关闭不需要的文件描述符

        execlp("ls", "ls", NULL); // 执行ls命令
        perror("execlp ls");
        exit(EXIT_FAILURE);
    }

    // 创建第二个子进程
    if ((pid2 = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
        // 子进程2中执行wc命令
        close(pipe1[1]); // 关闭管道1的写端
        dup2(pipe1[0], STDIN_FILENO); // 将标准输入重定向到管道1的读端
        close(pipe1[0]); // 关闭不需要的文件描述符

        close(pipe2[0]); // 关闭管道2的读端
        dup2(pipe2[1], STDOUT_FILENO); // 将标准输出重定向到管道2的写端
        close(pipe2[1]); // 关闭不需要的文件描述符

        execlp("wc", "wc", NULL); // 执行wc命令
        perror("execlp wc");
        exit(EXIT_FAILURE);
    }

    // 主进程中执行sort命令
    close(pipe1[0]); // 关闭管道1的读端
    close(pipe1[1]); // 关闭管道1的写端

    close(pipe2[1]); // 关闭管道2的写端
    dup2(pipe2[0], STDIN_FILENO); // 将标准输入重定向到管道2的读端
    close(pipe2[0]); // 关闭不需要的文件描述符

    execlp("sort", "sort", NULL); // 执行sort命令
    perror("execlp sort");
    exit(EXIT_FAILURE);

    return 0;
}
