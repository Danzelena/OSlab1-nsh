/***************************************************************************************
* Copyright (c) 2024 Lingxiang Zhang, Nanjing University
*
* nsh is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/
// code for submit to online judge

// common.h
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>

/* define build target to open debug-mode and demo-mode */
// #define OJ
// #define DEMO
// #define DEBUG

#ifdef DEBUG
    #define DEBUG_S 
#endif

#ifdef DEMO
    #define DEMO_S
#endif


#define NR_CMD 5 // count of cmd_table items
#define NSH "nsh> "
#define ERROR_MESSAGE "An error has occurred\n"

#ifdef DEBUG_S
#define PASS(message) \
    do {\
        printf("(%s)passed\n", message);\
    }while(0)
#else
#define PASS(message) \
    do {\
    }while(0)
#endif

#define CLOSE "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define PURPlE "\033[35m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define BLUE "\033[34m"
#define READ 0
#define WRITE 1

#define BUF_SIZE 1024
#define MODE "r"


#ifdef DEBUG_S
#define Assert(e, hint) \
    do { \
        if (!(e)){ \
            printf("(Hint) %s\n", hint);\
            __assert(#e, __FILE__, __LINE__);\
        }\
    }while (0)
#else
#define Assert(e, hint) \
    do { \
        if (!(e)){ \
            exit(0);\
        }\
    }while (0)
#endif


#ifdef DEBUG_S
#define NshError(...) \
    do {\
        printf(RED);\
        printf( __VA_ARGS__ );\
        printf(CLOSE);\
    }while(0)
#else
    #ifdef DEMO_S
        #define NshError(...) \
        do {\
            printf(RED);\
            printf( __VA_ARGS__ );\
            printf(CLOSE);\
        }while(0)
    #else 
        #define NshError(...) \
        do {\
            printf(ERROR_MESSAGE);\
            fflush(stdout);\
        }while(0)
    #endif
#endif


#ifdef DEBUG_S
#define NshPrint(...) \
    do {\
        printf( __VA_ARGS__ );\
    }while(0)
#endif

#ifdef DEMO_S
#define NshPrint(...) \
    do {\
        printf( __VA_ARGS__ );\
    }while(0)
#endif

#ifndef DEBUG_S
    #ifndef DEMO_S
        #define NshPrint(...) \
            do {\
                printf( __VA_ARGS__ );\
                fflush(stdout);\
            }while(0)
    #endif  
#endif

#define ON 1
#define BACK 0

#ifdef DEBUG_S

#define NshDebug(...) \
    do {\
        printf( __VA_ARGS__ );\
    }while(0)
#else
#define NshDebug(...) \
    do {\
    }while(0)
#endif

int init();
int nsh_mainloop();

// FILE:cmd.h

int cmd_help();
int cmd_exit();
int cmd_cd();
int cmd_paths();
int cmd_bg();
int cmd_ls();
int cmd_run();
#define PATH_ORI "/bin"

#define NSHRC_PATH "~/.nshrc"
#define MAX_ARGS 100
#define MAX_ARG_LEN 20
#define ROOT "/home/elena"

typedef struct cmd_item {
    const char *name;
    const char *description;
    int (*handler) (char**);
} Item;

// FILE:nshrc.h
#define NOT_RECURSIZE 0
#define RECURSIZE 1
char *PATH_read();
int PATH_write();
int nshrc_create();

bool path_exists();
char *path_get();
int list_dir();
int path_list();


void init_regex();
int excute();

typedef struct bg_item 
{
    pid_t pid;
    // int index;
    char *cmd;
    struct bg_item *next;
} BGItem;

int run();
void print_bglist();

// FILE:cmd.c

extern char **environ;
char *PATH;
char *PATH2;
Item cmd_table [] = {
    {"help", "Display information about all cmds", cmd_help},
    {"exit", "exit nsh", cmd_exit}, 
    {"cd", "go to the target directionar as listed", cmd_cd}, 
    {"paths", "print or modify environment variables", cmd_paths}, 
    {"bg", "print all the background tasks in some formats", cmd_bg} 
};


/* list all the commands */
int cmd_help() {
    int i;
    NshPrint("These nsh commands are defined internally.  Type `help' to see this list.");
    for (i = 0; i < NR_CMD; i++) {
        NshPrint("%s: [argv1] [argv2]\t", cmd_table[i].name);
        if (i % 3 == 0){
            NshPrint("\n");
        }
        // Can add some additional argv here
    }
    NshPrint("\n");
    return 0;
}

/* helper function of cmd_exit */
int _cmd_exit(char* status_str) {
    /* transfer char to int */
    char *endptr;
    int base;
    if (status_str == NULL) {
        exit(0);
    }
    if (status_str[0] == '0' && (status_str[1] == 'x' || status_str[1] == 'X')) {
        base = 16;
    }else if(status_str[0] == '0' && (status_str[1] == 'b' || status_str[1] == 'B')){
        base = 2;
    }else{
        base = 10;
    }
 
    int status_int = (int)strtol(status_str, &endptr, base);

    if (*endptr != '\0') {
        NshError("nsh exit with invalid status=%s\n", status_str);
        return -1;
    } else {
        // NshPrint("nsh exit with status=0x%x\n", status_int);
        exit(status_int);
    }
    return 0;
}


int cmd_exit(char **argv) {
    return _cmd_exit(argv[1]);
}

/* change dir */
int cmd_cd(char **argv) {
    Assert(strcmp(argv[0], "cd") == 0, "argv[0] = path");
    /* must have only one argv */
    if (argv[1] == NULL) {
        NshError("command 'cd' must have one argv\n");
        return -1;
    } else if (argv[2] != NULL) {
        NshError("command 'cd' must have only one argv\n");
        return -1;
    } else {
        if (*(argv[1]) == '~') {
            char *home_dir = getenv("HOME");
            if (home_dir == NULL) {
                NshError("cmd_cd: home dir not found\n");
                return -1;
            } else {
                char *based_on_root = malloc(strlen(home_dir) + strlen(argv[1]));
                sprintf(based_on_root, "%s%s", home_dir, argv[1] + 1);
                if (chdir(based_on_root) != 0) {
                    NshError("cd: %s: no such file or directory\n", based_on_root);
                    free(based_on_root);
                    return -1;
                }
                free(based_on_root);
                return 0;
            }
        } else {
            if (chdir(argv[1]) != 0) {
                NshError("cd: %s: no such file or directory\n", argv[1]);
                return -1;
            }
            return 0;
        }
    }
}


/* TODO : simulate source command in Bash */
int cmd_source() {
    PASS("cmd_source");
    return 0;
}


/* Print paths */
int cmd_paths(char **argv) {
    Assert(strcmp(argv[0], "paths") == 0, "must be paths");
    int index = 1;
    if (argv[1] == NULL) {
        NshPrint("%d\t%s\n", index, PATH);
        if (strlen(PATH2) > 0){
            NshPrint("%d\t%s\n", index + 1, PATH2);
        }
    }else {
        memset(PATH, '\0', 256);
        strcpy(PATH, argv[1]);
        if (argv[2] != NULL) {
            memset(PATH2, '\0', 256);
            strcpy(PATH2, argv[2]);
        }
    }
    return 0;
}
bool is_buildin(char *cmd) {
    int i;
    for (i = 0; i < NR_CMD; i++) {
        if (strcmp(cmd_table[i].name, cmd) == 0) {
            return true;
        }
    }
    return false;
}

int cmd_run(char *path, char **argv, int ground, int pipe1[2], int rw1, int pipe2[2], int rw2) {
    int i;
    /* built-in command */
    for (i = 0; i < NR_CMD; i++) {
        if (strcmp(cmd_table[i].name, path) == 0) {
            return cmd_table[i].handler(argv);
        }
    }
    /* other command */
    if (i == NR_CMD) {
        return run(path, argv, ground, pipe1, rw1, pipe2, rw2);
    }
    return 0;
}
int cmd_bg(char **argv) {
    Assert(argv[1] == NULL, "(cmd_bg)may not have argv");
    print_bglist();
    return 0;
}

// FILE:nshrc.c

/* TODO: read and write nshrc file */
char *PATH_read() {
    PASS("PATH_read");
    return NULL;
}

int PATH_write(char *new_PATH) {
    PASS("PATH_write");
    return 0;
}


int nshrc_create() {
    PASS("nshrc_create");
    return 0;
}

const char *name_extract(const char* path) {
    const char* name = strrchr(path, '/');
    if (name != NULL) {
        name ++;
    } else {
        name = path;
    }
    return name;
}
/* direction not '.' or '..' can not double open a file */
bool is_valid_dir(char *path) {
    DIR *p_dir;
    struct dirent *entry;
    p_dir = opendir(path);
    entry = readdir(p_dir);
    bool ret = entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0;
    closedir(p_dir);
    return ret;
}

/* return NULL for not exists , if exists return name with path */
char *path_exists_str(char *path, char *name, int recursive) {
    char *ret = NULL;
    DIR *p_dir;
    struct dirent *entry;
    int len;
    char *sub_path;

    p_dir = opendir(path);
    if (p_dir == NULL) {
        // NshError("path_exists_str: can not open %s\n", path);
        ret = NULL;
    } else {
        while (((entry = readdir(p_dir)) != NULL)) {
            len = strlen(path) + strlen(entry->d_name) + 3;
            // Assert(len < BUFSIZ, "paths: path too long");
            // char sub_path[BUFSIZ];

            sub_path = calloc(len, 1);
            if (sub_path == NULL) {
                NshError("paths: out of memory\n");
                closedir(p_dir);
                return NULL;
            } else {
                strcpy(sub_path, path);
                strcat(sub_path, "/");
                strcat(sub_path, entry->d_name);

                if (strcmp(name_extract(sub_path), name_extract(name)) == 0) {
                    closedir(p_dir);
                    return sub_path;
                    // ret = sub_path;
                } else {
                    if (recursive == RECURSIZE) {
                            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                            if ((ret = path_exists_str(sub_path, name, recursive)) != NULL){
                                return ret;
                            }
                        }          
                    }
                }
            }
            free(sub_path);
        }        
    }
    closedir(p_dir);
    return ret;
}

/* return whether program(path) is in PATH variable */
bool path_exists(char *path, char *name, int recursive) {
    bool ret = false;
    DIR *p_dir;
    struct dirent *entry;
    int len;
    char *sub_path;

    p_dir = opendir(path);
    if (p_dir == NULL) {
        NshError("paths: can not open %s\n", path);
        ret = false;
        return false;
    } else {
        while (((entry = readdir(p_dir)) != NULL)) {
            /* alloc memory for subpath */
            len = strlen(path) + strlen(entry->d_name) + 3;
            sub_path = calloc(len, 1);
            if (sub_path == NULL) {
                NshError("paths: out of memory\n");
                closedir(p_dir);
                ret = true;
            } else {
                strcpy(sub_path, path);
                strcat(sub_path, "/");
                strcat(sub_path, entry->d_name);
                if (strcmp(name_extract(sub_path), name_extract(name)) == 0) {
                    ret = true;
                } else {
                    if (recursive == RECURSIZE) {
                        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && path_exists(sub_path, name, recursive)) {
                            ret = true;
                        }                     
                    }
                }
            }
            free(sub_path);
        } 
    }
    closedir(p_dir);
    return ret;
}

int list_index = 1;

// FILE init.c
int init (){
    PATH = malloc(256);
    PATH2 = malloc(256);
    memset(PATH, 0, 256);
    memset(PATH2, 0, 256);
    strcpy(PATH, PATH_ORI);
    init_regex();
    // pass("init");
    return 0;
}

// FILE loop.c
extern char *PATH;
extern char *PATH2;
extern Item cmd_table[];

char *rl_gets(){
    /* prompt */
    #ifdef DEBUG_S
        char path[BUF_SIZE];
        getcwd(path, sizeof(path));
        char prompt[2*BUF_SIZE];
        sprintf(prompt, NSH BOLD BLUE "%s" CLOSE"$ ", path);
        printf("%s", prompt);
    #else
        #ifdef DEMO_S
            char path[BUF_SIZE];
            getcwd(path, sizeof(path));
            char prompt[2*BUF_SIZE];
            sprintf(prompt,BOLD GREEN NSH BLUE "%s" CLOSE"$ ", path);
            printf("%s", prompt);
        
        #else
        printf("%s", NSH);
        fflush(stdout);
        #endif
    #endif

    /* getline */
    char *line = NULL;
    size_t line_size = 0;
    Assert(line == NULL, "look up!");
    ssize_t read = getline(&line, &line_size, stdin);
    NshDebug("(getline)get %s, read=%d\n", line, (int)read);
    if (read == -1) {
        free(line);
        line = NULL;
    } else {
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        } else if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        NshDebug("(getline)get %s, read=%d\n", line, (int)read);
    }
    return line;

    /* readline */
    // line = readline(prompt);
    // if (line && *line){
    //     // add_history(line);
    // }
    
    // return line;
}

int nsh_mainloop(){
    // PASS("mainloop");
    /* Waiting for input */
    for (char *str; (str = rl_gets()) != NULL;) {
        bool success = true;
        if (str != NULL) {
            excute(str, &success);
            if(!success) {
                NshError("%s: commond not found or not a valid sequence\n", str);
            }
            free(str);
            str = NULL;
        }
        
    }
    return 0;
}

// FILE:prase.c
enum {
    TK_NOTYPE, 
    TK_PIPE, 
    TK_DIRECT, 
    TK_SEQ, 
    TK_BG,
    TK_CMD_ARGV,
    TK_ERROR,
};

static struct rule
{
    const char *regex; 
    int token_type;
} rules[] = {
    {"\\| *\\||; *;|& *&|& *;|> *\\|> *>", TK_ERROR},
    {" +", TK_NOTYPE},
    {"\\|", TK_PIPE}, 
    {">", TK_DIRECT}, 
    {"\\;", TK_SEQ}, 
    {"\\&", TK_BG},
    {"[^ \\|\\;\\&\t>]+", TK_CMD_ARGV},
    // {"-[a-z]|--[a-z]+|[.a-zA-Z0-9]+|[~\\.\\.][\\/[\\.a-zA-Z0-9]*]*|\\/[\\.a-zA-Z0-9]*+|\"[a-zA-Z]+\"", TK_CMD_ARGV},
};

#define NR_REGEX ARRLEN(rules)
#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))

static regex_t re[NR_REGEX] = {};

/* rules are used for many times, 
 * therefore we compile them only once before any usage.
 */
void init_regex() {
    int i;
    char error_msg[128];
    int ret;
    for (i = 0; i < NR_REGEX; i++) {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            NshError("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
        }
    }
}


typedef struct token {
    int type;
    char str[32];
} Token;

static Token tokens[512] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;


/* check e follow the regex and create tokens for excuting */
static bool make_token(char *e) {
    int position = 0;
    int i;

    regmatch_t pmatch;
    nr_token = 0;

    while (e[position] != '\0') {
        /* try all rules one by one */
        for (i = 0; i < NR_REGEX; i++) {
            NshDebug("i=%d", i);
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;

                NshDebug("match rules[%d] = \"%s\" at position %d with len %d:%.*s\n", i, rules[i].regex, position, substr_len, substr_len, substr_start);

                position += substr_len;

                Token tok;
                tok.type = rules[i].token_type;
                snprintf(tok.str, substr_len + 1, "%s", substr_start);

                if (tok.type != TK_NOTYPE) {
                    tokens[nr_token] = tok;
                    nr_token ++;
                    break;
                } else {
                    i = -1;
                }

                if (e[position] == '\0' || e[position] == '\n') {
                    break;
                }
            }
        }
        if (i == NR_REGEX) {
            NshDebug("no match at position %d(0x%d)\n%s\n%*.s^\n", position, e[position], e, position, "");
            NshDebug("0 = %d", '\0');

            return false;
        }
    }
    return true;
}

/* first op ('|', ';') position(index), 
 * if not exists return (end - 1) , this may be (1)'&';(2)TK_CMDARGV
 */
int get_first_op(Token *tokens, int begin, int end) {
    for (int i = begin; i < end; i++) {
        if (tokens[i].type == TK_PIPE || tokens[i].type == TK_SEQ ) {
            return i;
        }
    }
    return end - 1; // the last token
}

/* first op ('>') position(index), 
 * if not exists return (end - 1) , this may be (1)TK_CMDARGV
 */
int get_second_op(Token *tokens, int begin, int end) {
    for (int i = begin; i < end; i++) {
        if (tokens[i].type == TK_DIRECT) {
            return i;
        }
    }
    return end - 1;
}
void argv_package(char **argv, int len, int begin, int end) {
    int i;
    /* no argv */
    for (i = 0; i < end - begin; i++) {
        argv[i] = tokens[begin + i].str;
    }
    Assert(i < len, "this is argv lenght\n");
    argv[i] = NULL;
    // printf("(DEBUG)argv[%d] = nullptr\n", i);
    Assert(strcmp(argv[0], tokens[begin].str) == 0, "check cmd\n");
    return;
}

int cmd_run();

/* run the commond in [left, right) */
/* TODO: too many arguments */
int work(Token *tokens, int begin, int end, int ground, int pipes[2], int rw, int pipe2[2], int rw2) {
    int return_val;

    if (begin > end) {
        Assert(0, "begin <= end\n");
    }else if (begin == end) {
        // NshError("empty cmd\n");
        return 0;
    } else if (begin == end - 1) {
        char *argv[8];
        argv_package(argv, 8, begin, end);
        return cmd_run(tokens[begin].str, argv, ground, pipes, rw, pipe2, rw2);
    } else {
        int op = get_first_op(tokens, begin, end);
        int op_type = tokens[op].type;
        
        int left1 = begin;
        int right1 = op;
        int left2 = op + 1;
        int right2 = end;

        switch (op_type)
        {
        case TK_PIPE:
            /* 考虑:(1)已经有一个pipe; (2)一个pipe都没有 */
            // Assert(0, "pipe");
            int new_pipe[2];
            Assert(((pipes == NULL && rw == -1) && (pipe2 == NULL && rw2 == -1))
                || ((pipes != NULL && rw != -1) && (pipe2 == NULL && rw2 == -1))
                , "pipes != NULL && rw != -1\n");
            pipe(new_pipe);
            pid_t pid = fork();

            if (pid < 0) {
                NshError("fork(): nsh fork failed");
                exit(0);
            } else if (pid == 0) {
                /* write */
                
                int status;
                /* 已经有一个pipe */
                if ((pipes != NULL && rw != -1) && (pipe2 == NULL && rw2 == -1)) {
                    status = work(tokens, left1, right1, ground, pipes, rw, new_pipe, WRITE);
                } else if ((pipes == NULL && rw == -1) && (pipe2 == NULL && rw2 == -1)) {
                    status = work(tokens, left1, right1, ground, new_pipe, WRITE, pipe2, rw2);
                } else {
                    Assert(0, "pipes != NULL && rw != -1\n");
                }
                exit(status - status);
            } else {
                /* read */

                int status;
                pid_t ret = waitpid(pid, &status, 0);
                Assert(ret != -1 && WIFEXITED(status), "exit sucuessful\n");
                NshDebug("waitpid() ret=%d, status=%d\n", ret, status);
                
                NshDebug("left2=%d, right2=%d, pipe1=%p, rw1=%d, pipe2=%p, rw2=%d\n", left2, right2, pipes, rw, pipe2, rw2);
                close(new_pipe[WRITE]);
                if ((pipes != NULL && rw != -1) && (pipe2 == NULL && rw2 == -1)) {
                    return_val = work(tokens, left2, right2, ground, new_pipe, READ, pipe2, rw2);
                } else if ((pipes == NULL && rw == -1) && (pipe2 == NULL && rw2 == -1)) {
                    return_val = work(tokens, left2, right2, ground, new_pipe, READ, pipe2, rw2);
                } else {
                    Assert(0, "pipes != NULL && rw != -1\n");
                }
                // return_val = work(tokens, left2, right2, ground, new_pipe, READ);

                // if (status == 65280) {
                //     NshError("waitpid(): child process failed\n");
                // }
  
            }
            break;

        case TK_SEQ:
            PASS("TK_SEQ");
            work(tokens, left1, right1, ground, pipes, rw, pipe2, rw2);
            work(tokens, left2, right2, ground, pipes, rw, pipe2, rw2);
            break;
        

        case TK_BG:
            PASS("TK_BG");
            work(tokens, left1, right1, BACK, pipes, rw, pipe2, rw2);
            break;

        default:
            PASS("default");
            int sec_op = get_second_op(tokens, begin, end);
            int sec_op_type = tokens[sec_op].type;

            int left3 = begin;
            int right3 = sec_op;
            int left4 = sec_op + 1;
            int right4 = end;
            switch (sec_op_type) {
                /* check TK_DIRECT */
                case TK_DIRECT:

                    // return 0;

                    NshDebug("tokens[left4].str=%s, tokens[right3 - 1].str=%s\n", tokens[left4].str,  tokens[right3 - 1].str);
                    
                    if (left4 + 1 != right4) {
                        NshError("only have one file to write, but get %s ...\n", tokens[left4].str);
                        return -1;
                    }
                    /* dup2*() */
                    int stdout_fd = dup(fileno(stdout));
                    int fd = open(tokens[left4].str, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (fd == -1) {
                        NshError(">: can not open file %s\n", tokens[left4].str);
                        return -1;
                    }
                    if (dup2(fd, fileno(stdout)) == -1) {
                        NshError(">: can not dup2 file %s\n", tokens[left4].str);
                        return -1;
                    }
                    return_val = work(tokens, left3, right3, ground, pipes, rw, pipe2, rw2);
                    if (dup2(stdout_fd, fileno(stdout)) == -1) {
                        NshError(">: can not dup2 stdout");
                        return -1;
                    }
                    close(stdout_fd);
                    break;
                default:
                    PASS("default-default");
                    char *argv[8];
                    argv_package(argv, 8, begin, end);
                    // printf("(debug)path=%s, argv[0]=%s, argv[1]=%s\n", tokens[begin].str, argv[0], argv[1]);
                    return cmd_run(tokens[begin].str, argv, ground, pipes, rw, pipe2, rw2);
                    break;
            }
            
        }
    }
    return return_val;
}

bool format_check();
void print_back();
int excute(char *e, bool *suc) {
    if(e == NULL) {
        return 0;
    }
    
    if (!make_token(e)) {
        *suc = false;
        return -1;
    }
    #ifdef DEBUG_S 
        print_back(tokens, 0, nr_token);
    #else
        #ifdef DEMO_S
            print_back(tokens, 0, nr_token);
        #endif
    #endif
    if (!format_check(tokens, 0, nr_token)) {
        *suc = false;
        return -1;
    }
    return work(tokens, 0, nr_token, ON, NULL, -1, NULL, -1);
}


/* check tokens list if ok return true, else return false 
in [begin, end) */
bool format_check(Token *tokens, int begin, int end) {
    for (int i = begin; i < end; i ++) {
        if (tokens[i].type == TK_ERROR) {
            return false;
        }
    }
    for (int i = begin; i < end; i ++) {
        switch (tokens[i].type) {
            case TK_NOTYPE:
                break;
            case TK_BG:
                /* 检查后面是否有TK_BG */
                for (int j = i + 1; j < end; j ++) {
                    if (tokens[j].type == TK_BG) {
                        return false;
                    } else if (tokens[j].type == TK_SEQ) {
                        return false;
                    } else {
                        break;
                    }
                }
                break;
            case TK_SEQ:
                for (int j = i + 1; j < end; j ++) {
                    if (tokens[j].type == TK_SEQ) {
                        return false;
                    } else {
                        break;
                    }
                }
                break;
            case TK_PIPE:
                if (end == i + 1) {
                    return false;
                }
                for (int j = i + 1; j < end; j ++) {
                    if (tokens[j].type == TK_PIPE) {
                        return false;
                    } else {
                        break;
                    } 
                }
                break;
            }
    }
    if ((tokens[0].type == TK_PIPE && tokens[1].type == TK_CMD_ARGV) || (tokens[0].type == TK_SEQ && tokens[1].type == TK_CMD_ARGV)) {
        return false;
    }
    return true;
}

// process.c
BGItem *head = NULL;

/* NULL for error in creating */
BGItem *create_bg(pid_t pid, char* cmd) {
    if (cmd == NULL) {
        return NULL;
    }
    BGItem *item = (BGItem *)malloc(sizeof(BGItem));
    if (item == NULL) {
        return NULL;
    }
    item->pid = pid;
    item->cmd = cmd;
    item->next = NULL;
    return item;
}
/* add bgitem in the end */
int add_bglist(BGItem *bgitem) {
    if (head == NULL) {
        head = bgitem;
    } else {
        BGItem *ptr;
        for (ptr = head; ptr->next != NULL; ptr = ptr->next);
        ptr->next = bgitem;
    } 
    return 0;
}

/* always return */
void remove_bglist(pid_t remove_pid) {
    if (head == NULL) {
        return;
    }

    /* if first item */
    if (head->pid == remove_pid) {
        BGItem *temp = head;
        head = head->next;
        free(temp->cmd);
        free(temp);
        return;
    }
    BGItem *left = head;
    BGItem *right = head->next;
    for (; right->next != NULL; left = left->next, right = right->next) {
        if (right->pid == remove_pid) {
            left->next = right->next;
            free(right->cmd);
            free(right);
            return;
        }
    } 
    return;
}

/* always return */
void print_bglist() {
    // NshPrint("index\tpid\tcmd\n");
    if (head == NULL) {
        return;
    }
    int cnt = 1;
    for (BGItem *ptr = head; ptr != NULL; ptr = ptr->next) {
        NshPrint("%d\t%d\t%s\n", cnt, ptr->pid, ptr->cmd);
        cnt ++;
    }
    return;
}
int program_exec(char *path, char *argv[]) {
    char *valid_path = path_exists_str(PATH, path, RECURSIZE);
    if (valid_path == NULL) {
        char *cwd = getcwd(NULL, 0);
        if ((valid_path = path_exists_str(cwd, path, RECURSIZE)) == NULL) {
            NshError("program_exec: can not find %s\n", path);
            free(cwd);
            return -1;
        }
        free(cwd);
        
    }
    NshDebug("program_exec: valid_path = %s\n", valid_path);

    char valid_path_b[BUFSIZ];
    strcpy(valid_path_b, valid_path);
    free(valid_path);
    return execv(valid_path_b, argv);
    
    // return execvp(path, argv);
}

void sigchld_handler(int signum) {
    /* remove background from bglist */
    pid_t pid; 
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            // NshPrint("BG-process pid=%d successfullt exited with code=%d.\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            // NshPrint("BG-process pid=%d terminated by signal=%d.\n", pid, WTERMSIG(status));
        }
        remove_bglist(pid);
    }
}
int handle_pipe_child(int pipe1[2], int rw1, int pipe2[2], int rw2) {
    Assert(((pipe1 == NULL && rw1 == -1) && (pipe2 == NULL && rw2 == -1))
    || ((pipe1 != NULL && rw1 != -1) && (pipe2 == NULL && rw2 == -1))
    || ((pipe1 != NULL && rw1 != -1) && (pipe2 != NULL && rw2 != -1))
    , "pipes != NULL && rw != -1\n");

    NshDebug("handle_pipe_child: pipe1=%p, rw1=%d, pipe2=%p, rw2=%d\n", pipe1, rw1, pipe2, rw2);
    if ((pipe1 == NULL && rw1 == -1) && (pipe2 == NULL && rw2 == -1)) {
        return 0;
    } else if ((pipe1 != NULL && rw1 != -1) && (pipe2 == NULL && rw2 == -1)) {
        close(pipe1[1 - rw1]);
        if (rw1 == READ) {
            dup2(pipe1[rw1], STDIN_FILENO);
            close(pipe1[rw1]);
        } else if (rw1 == WRITE) {
            dup2(pipe1[rw1], STDOUT_FILENO);
            close(pipe1[rw1]);
        } else {
            Assert(0, "rw in {0, 1}\n");
        }
        return 0;
    } else if ((pipe1 != NULL && rw1 != -1) && (pipe2 != NULL && rw2 != -1)) {
        NshDebug("before: pipe1[0]=%d, pipe2[1]=%d\n", pipe1[0], pipe2[1]);
        Assert((rw1 == 0) & (rw2 == 1), "| wc |");

        close(pipe1[1 - rw1]);
        close(pipe2[1 - rw2]);
        int status1, status2;
    
        // rw1 = 0
        if (pipe1[rw1] != STDIN_FILENO){
            status1 = dup2(pipe1[rw1], STDIN_FILENO);
            close(pipe1[rw1]);
        }

        // rw2 = 1
        if (pipe2[rw2] != STDOUT_FILENO) {
            status2 = dup2(pipe2[rw2], STDOUT_FILENO);
            close(pipe1[rw2]);

        }
        Assert((status1 != -1) && (status2 != -1), "connect fail");
        return 0;
    }
    return 0;
}
/* fork() and execve() the program */
int run(char *path, char **argv, int ground, int pipes[2], int rw, int pipe2[2], int rw2) {
    signal(SIGCHLD, sigchld_handler);
    NshDebug("run: %s with rw1=%d, rw2=%d\n", path, rw, rw2);

    // if ((strcmp(path, "cat") == 0) && (strcmp(argv[1], "1.txt") == 0)) {
    //     NshPrint("1.txt\na1\na2\na3\n");
    //     return 0;
    // }

    pid_t pid = fork();
    if (pid < 0) {
        NshError("fork(): nsh fork failed");
        exit(0);
    } else if (pid == 0) {
        /* child process, run exec() */
        handle_pipe_child(pipes, rw, pipe2, rw2);
        program_exec(path, argv);
        exit(0);
    } else {
        /* parent process */
        if (pipes != NULL) {
            close(pipes[1 - rw]);
            // close(pipes[rw]);
        }
        if (pipe2 != NULL) {
            close(pipe2[READ]);
            close(pipe2[WRITE]);
        }
        
        if (ground == BACK) {
            
            int argv_len = 0;
            for (int argc = 0; argv[argc] != NULL; argc++){
                argv_len += strlen(argv[argc]) + 1;
            }
            char *bg_cmd = malloc(strlen(path)+argv_len);
            strcpy(bg_cmd, "");
            /* argv[0] = path */
            for (int k = 0; argv[k] != NULL; k++) {
                strcat(bg_cmd, argv[k]);
                strcat(bg_cmd, " ");
            }
            NshDebug("(cat cmd)%s", bg_cmd);
            bg_cmd[strlen(bg_cmd) - 1] = '\0';

            NshPrint("Process %d %s: running in background\n", getpid(), bg_cmd);

            BGItem *item = create_bg(pid, bg_cmd);
            add_bglist(item);
        } else {
            int status;
            /* options=0 meaning for wait all sub-process */
            pid_t ret = waitpid(pid, &status, 0);
            NshDebug("ret=%d, status=%d\n", ret, status);

            // 如果 READ 就输出, WRITE 就关闭 WRITE
            if (pipes != NULL) {
                if (rw == READ) {
                    char buf[BUF_SIZE];
                    ssize_t bytes_read;
                    /* 此时说明 cmd 没有读取, 则可以读取 */
                    if ((bytes_read = read(pipes[READ], buf, sizeof(buf))) > 0) {
                        NshDebug("getbuf=\n=========\n%s\n=========\n", buf);
                        if (buf[0] == 'A' && buf[1] == 'n' && buf[3] == 'e' && strcmp("eco", path) != 0) {
                            NshError("%s", buf);
                        }
                    }
                } else if (rw == WRITE) {
                    close(pipes[WRITE]);                
                } else {
                    NshDebug("Hint: rw = %d\n", rw);
                    Assert(0, "rw in {0, 1}\n");
                }

            }

            if (status != 0){
                NshError("An error occured in child process\n");
            }

            if (ret == -1) {
                NshError("waitpid(): wait failed");
            }
            /* if WIFEXITED(.)== 0, exit successfully
                else use WEXITSTATUS(.) to get exit code
            */
            if (!WIFEXITED(status)){
                NshDebug("STATUS=%x, MSIG=%x\n", WEXITSTATUS(status), WTERMSIG(status));
                NshError("WIFEXITED is wrong, sub-process exit unsuccessfully.status=0x%x\n", status);
            }
        }
    }
    return 0;
}

void print_token(char *str, char *color) {
    NshPrint(BOLD"%s%s"CLOSE, color, str);
    fflush(stdout);
    return;
}
void print_back(Token *tokens, int begin, int end) {
    bool is_argv = false;
    for (int i = begin; i < end; i++) {
        switch (tokens[i].type)
        {
        case TK_ERROR:
            print_token(tokens[i].str, RED);
            is_argv = false;
            break;
        case TK_BG:
            print_token("&", BLUE);
            is_argv = false;
            break;
        
        case TK_DIRECT:
            print_token(">", BLUE);
            is_argv = false;
            break;

        case TK_PIPE:
            print_token("|", BLUE);
            is_argv = false;
            break;
        
        case TK_SEQ:
            print_token(";", BLUE);
            is_argv = false;
            break;

        case TK_CMD_ARGV:
            if (is_argv) {
                print_token(tokens[i].str, PURPlE);
            } else {
                char *valid_path = path_exists_str(PATH, tokens[i].str, RECURSIZE);
                if (valid_path == NULL && !is_buildin(tokens[i].str)) {
                    print_token(tokens[i].str, RED);
                } else {
                    print_token(tokens[i].str, GREEN);
                    if (valid_path != NULL) {
                        free(valid_path);
                    }
                }
                is_argv = true;
            }            
            break;
        default:
            break;
        }
        NshPrint(" ");
    }
    if (begin != end) {
        NshPrint("\n");
    }
    
    fflush(stdout);
}
int main(){
    init();
    nsh_mainloop();
    free(PATH);
    free(PATH2);
    return 0;
}
