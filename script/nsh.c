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
#include <readline/readline.h>
#include <readline/history.h>

#define NR_CMD 5 // count of cmd_table items
#define NSH "nsh> "
#define ERROR_MESSAGE "An error has occured\n"
// #define PASS(message) \
//     do {\
//         printf("(%s)passed\n", message);\
//     }while(0)
#define PASS(message) \
    do {\
    }while(0)
#define CLOSE "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define READ 0
#define WRITE 1

// #define NDEBUG
#ifdef NDEBUG
#define Assert(e) (void)0
#else
#define Assert(e, hint) \
    do { \
        if (!(e)){ \
            printf("(Hint) %s\n", hint);\
            __assert(#e, __FILE__, __LINE__);\
        }\
    }while (0)
#endif
       
#define NshPrint(...) \
    do {\
        printf( __VA_ARGS__ );\
    }while(0)
// #define NshError(...) \
//     do {\
//         printf(RED);\
//         printf( __VA_ARGS__ );\
//         printf(CLOSE);\
//     }while(0)

#define NshError(...) \
    do {\
        printf(ERROR_MESSAGE);\
    }while(0)

#define ON 1
#define BACK 0

#define PrintError(message) \
    do {\
        printf(RED"%s"CLOSE, message);\
    }while(0)
// char *prompt;
// char *error_message;
int init();
int nsh_mainloop();
int prase_args();

// cmd.h

int cmd_help();
int cmd_exit();
int cmd_cd();
int cmd_paths();
int cmd_bg();
int cmd_ls();
int cmd_run();
char *get_first_argv();
char *get_second_argv();
#define PATH_ORI "/bin"

#define NSHRC_PATH "~/.nshrc"
#define MAX_ARGS 100
#define MAX_ARG_LEN 20
#define ROOT "/home/elena"

typedef struct cmd_item {
    const char *name;
    const char *description;
    int (*handler) (char*);
} Item;

// nshrc.h
#define NOT_RECURSIZE 0
#define RECURSIZE 1
char *PATH_read();
int PATH_write();
int nshrc_create();

bool path_exists();
char *path_get();
int list_dir();
int path_list();

// prase.h
void init_regex();
// int prase_args();
int excute();


// process.h


typedef struct bg_item 
{
    pid_t pid;
    // int index;
    char *cmd;
    struct bg_item *next;
} BGItem;

int run();
void print_bglist();

// cmd.c


extern char **environ;
char *PATH = PATH_ORI;
Item cmd_table [] = {
    {"help", "Display information about all cmds", cmd_help},
    {"exit", "exit nsh", cmd_exit}, 
    {"cd", "go to the target directionar as listed", cmd_cd}, 
    {"paths", "print or modify environment variables", cmd_paths}, 
    {"bg", "print all the background tasks in some formats", cmd_bg} 
    // {"ls", "list all the file/directionary", cmd_ls}
};
// char **get_argvs(const char *argv) {
//     char args[MAX_ARGS][MAX_ARG_LEN];
// }

/* will return the first argv, if NULL will return NULL*/
char *get_first_argv(const char *argv) {
    if (argv == NULL) {
        return NULL;
    } else {
        char buf[512];
        strncpy(buf, argv, sizeof(buf));
        const char * delim = " ";
        char *token = strtok(buf, delim);
        printf("1:token=%s\n", token);
        return token;  
        
    }
    // return argv;
}

/* if not exists will return NULL */
char *get_second_argv(const char *argv) {
    // PASS("get_second_argv");
    if (argv == NULL) {
        return NULL;
    } else {
        char buf[512];
        strncpy(buf, argv, sizeof(buf));
        const char *delim = " ";
        char *token = strtok(buf, delim);
        if (token != NULL) {
            token = strtok(NULL, delim);
        }
        printf("2:token=%s\n", token);
        return token;
    }
    // return NULL;
}
/* list all the commands */
int cmd_help() {
    int i;
    NshPrint("These nsh commands are defined internally.  Type `help' to see this list.");
    for (i = 0; i < NR_CMD; i++) {
        NshPrint("%s: [argv1] [argv2]\t", cmd_table[i].name);
        if (i % 3 == 0){
            NshPrint("\n");
        }
        // TODO: add some additional argv here
    }
    NshPrint("\n");
    return 0;
}


int cmd_exit(char* status_str) {
    /* transfer char to int */
    char *endptr;
    int base;
    if (status_str == NULL) {
        exit(1);
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
        NshPrint("nsh exit with status=0x%x\n", status_int);
        exit(status_int);
    }
    return 0;
}
/* change dir */
int cmd_cd(char *str) {
    // printf("(debug)str=%s\n", str);
    if (str == NULL) {
        NshError("command 'cd' must have one argv\n");
        return -1;
    } else {
        /* then argv'end is '\0' */
        char buf[BUFSIZ];
        memset(buf, '\0', sizeof(buf));
        strcpy(buf, str);
        // char *end = buf + strlen(buf);
        if (strcmp(str, "~") == 0) {
            return chdir(ROOT);
        } else {
            char *path = strtok(buf, " ");
            const char *path2 = path + strlen(path) + 1;
            if (*path == '~') {
                char based_on_root[256];
                sprintf(based_on_root, "%s%s", ROOT, path + 1);
                // printf("(debug)new_path = %s", based_on_root);
                path = based_on_root;    
            }
            if (*path2 != '\0') {
                NshError("command 'cd' gets too many argv=%s", buf);
                return -1;
            } else {
                if (chdir(path) != 0) {
                    NshError("cd: %s: no such file or directory\n", str);
                    return -1;
                } else {
                    return 0;
                }
            }   
        }
    }
    return 0;
}


int cmd_source() {
    PASS("cmd_source");
    return 0;
}
/* print paths */
int cmd_paths(char *argv) {
    int recursive = RECURSIZE;
    if (argv == NULL) {
        return list_dir(PATH, recursive);
    } else if (get_second_argv(argv) != NULL) {
        NshError("commond paths allow one argv, but gets %s\n", argv);
        return -1;
    } else {
        PATH = get_first_argv(argv);
        return list_dir(PATH, recursive);
    }
    // PASS("cmd_paths");
    return 0;
}
int cmd_run(char *path, char **argv, int ground, int pipes[2], int rw) {
    int i;
    /* built-in command */
    for (i = 0; i < NR_CMD; i++) {
        if (strcmp(cmd_table[i].name, path) == 0) {
            return cmd_table[i].handler(argv[1]);
        }
    }
    /* other command */
    if (i == NR_CMD) {
        /* handle '~' in path */
        char *cwd = NULL;
        cwd = getcwd(NULL, 0);
        if (path_exists(PATH, path, RECURSIZE) || path_exists(cwd, path, RECURSIZE)) {
            free(cwd);
            return run(path, argv, ground, pipes, rw);
        } else {
            NshError("%s: commond not found\n", path);
            free(cwd);
            return -1;
        }
    }
    return 0;
}
int cmd_bg() {
    print_bglist();
    return 0;
}

// nshrc.c

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
    // return false;
}

/* generately get all the path in the parent path but not recursively */
char* path_get(char *parent_path) {
    static DIR *p_dir;
    static char buf[BUFSIZ];
    
    struct dirent *entry;
    if (p_dir == NULL) {
        p_dir = opendir(parent_path);
        if (p_dir == NULL) {
            NshError("path_gets: can not open %s\n", parent_path);
            perror(" ");
            exit(-1);
            return NULL;
        }
    }
    // entry = (struct dirent*)readdir(p_dir);
    if ((entry = readdir(p_dir)) != NULL) {
        snprintf(buf, sizeof(buf), "%s/%s", parent_path, entry->d_name);
        return buf;
    } else {
        closedir(p_dir);
        p_dir = NULL;
        return NULL;
    }

}

int path_list(char *path, int recursive) {
    return list_dir(path, recursive);
}
int list_index = 1;
int list_dir(char *path, int recursive) {
    // TODO: update this function to a generator-mode
    DIR *p_dir;
    struct dirent *entry;
    int len;
    char *sub_path;

    p_dir = opendir(path);
    if (p_dir == NULL) {
        NshError("paths: can not open %s\n", path);
        return -1;
    }
    while (((entry = readdir(p_dir)) != NULL)) {
        /* alloc memory for subpath */
        len = strlen(path) + strlen(entry->d_name) + 3;
        sub_path = calloc(len, 1);
        if (sub_path == NULL) {
            NshError("paths: out of memory\n");
            closedir(p_dir);
            return -1;
        }

        strcpy(sub_path, path);
        strcat(sub_path, "/");
        strcat(sub_path, entry->d_name);

        /* implement paths commond */
        NshPrint("%d\t%s\n", list_index, name_extract(sub_path));
        list_index ++;

        if (entry->d_type == DT_DIR && recursive != NOT_RECURSIZE && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            list_dir(sub_path, recursive);
        }
        free(sub_path);
    }
    closedir(p_dir);
    list_index = 1;
    return -1;
}

// init.c
int init (){
    init_regex();
    // pass("init");
    return 0;
}

// loop.c



#define BUF_SIZE 1024
#define MODE "r"

extern char *PATH;
extern Item cmd_table[];
char *rl_gets(){
    char *line = NULL;
    Assert(line == NULL, "look up!");


    /* prompt */
    char path[BUF_SIZE];
    getcwd(path, sizeof(path));

    char prompt[2*BUF_SIZE];
    // sprintf(prompt, NSH BOLD BLUE "%s" CLOSE"$ ", path);
    sprintf(prompt, NSH);

    line = readline(prompt);
    if (line && *line){
        // add_history(line);
    }
    return line;
}

/* match the commond and run the command */
// int cmd_match(char *cmd, char* arg){
//     int i;
//     for (i = 0; i < NR_CMD; i++) {
//         if (strcmp(cmd_table[i].name, cmd) == 0) {
//             return cmd_table[i].handler(arg);
//         }
//         // PASS("match command");
//     }
//     if (i == NR_CMD) {
//         // return cmd_run(cmd, arg);
//         char *cwd = NULL;
//         cwd = getcwd(NULL, 0);
//         printf("debug: cwd=%s\n", cwd);
//         if (path_exists(PATH, cmd, RECURSIZE) || path_exists(cwd, cmd, RECURSIZE)) {
//             // TODO: add a function a parse all the args
//             char *argv[MAX_ARGS];
//             memset(argv, '\0', sizeof(argv));
//             argv[0] = cmd;
//             argv[1] = get_first_argv(arg);
//             argv[2] = get_second_argv(arg);
//             argv[3] = NULL;
//             free(cwd);
//             return cmd_run(cmd, argv);
//         } else {
//             NshError("%s: commond not found\n", cmd);
//             free(cwd);
//             return -1;
//         }
//     }
//     return 0;
// }
int call_bash(char *str){
    if (str == NULL){
        return 0;
    }
    FILE *fp;
    char buf[BUF_SIZE];
    fp = popen(str, MODE);
    if (fp == NULL) {
        return -1;
    }
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        NshPrint("%s", buf);
    }
    
    return 0;
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
        }



        /* handle input(=str) */
        // char *str_end = str + strlen(str);
        // char *str_cmd = strtok(str, " ");
        // if (str_cmd == NULL){
        //     continue;   
        //     // Assert(0, "first char can't be empty");
        // }
        // char *str_args = str_cmd + strlen(str_cmd) + 1;

        // /* Check args exists */

        // if (str_args > str_end){
        //     str_args = NULL;
        // }

        // cmd_match(str_cmd, str_args);
        // int i;
        // for (i = 0; i < NR_CMD; i++){
            
        // }
        // if (i == NR_CMD){
        //     Print(RED"This command is out of my business, I will call "BOLD"Bash"CLOSE RED" to do\n"CLOSE);
        //     if (call_bash(str) != 0){
        //         Print("Some error happened in Bash");
        //     }
        // }
    }

    // Assert(0);
    // pass("nsh_mainloop");
    return 0;
}

// prase.c
enum {
    TK_NOTYPE, 
    TK_PIPE, 
    TK_DIRECT, 
    TK_SEQ, 
    TK_BG,
    TK_CMD_ARGV,
    // TK_ARGV,
};

static struct rule
{
    const char *regex; 
    int token_type;
} rules[] = {
    {" +", TK_NOTYPE},
    {"\\|", TK_PIPE}, 
    {">", TK_DIRECT}, 
    {"\\;", TK_SEQ}, 
    {"\\&", TK_BG},
    {"-[a-z]|--[a-z-]+|[a-zA-Z0-9]+|./[.a-zA-Z]+|\"[a-zA-Z]+\"|..|../|[.a-zA-Z]+", TK_CMD_ARGV},
    // {"[a-zA-Z]+", TK_CMD},
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
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;

                // printf("(debug)match rules[%d] = \"%s\" at position %d with len %d:%.*s\n", i, rules[i].regex, position, substr_len, substr_len, substr_start);

                position += substr_len;

                Token tok;
                tok.type = rules[i].token_type;
                snprintf(tok.str, substr_len + 1, "%s", substr_start);

                if (tok.type != TK_NOTYPE) {
                    tokens[nr_token] = tok;
                    nr_token ++;
                    break;
                }
            }
        }
        if (i == NR_REGEX) {
            // printf("(debug)no match ar position %d\n%s\n%*.s^\n", position, e, position, "");
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
        if (tokens[i].type == TK_PIPE || tokens[i].type == TK_SEQ) {
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
    for (i = 0; i < end - begin; i++) {
        argv[i] = tokens[begin + i].str;
    }
    Assert(i < len, "this is argv lenght\n");
    argv[i] = NULL;
    Assert(strcmp(argv[0], tokens[begin].str) == 0, "check cmd\n");
    return;
}
// TODO: It is not a nice way to directly cite the funciton. 
int cmd_run();

/* run the commond in [left, right) */
int work(Token *tokens, int begin, int end, int ground, int pipes[2], int rw) {
    if (begin > end) {
        Assert(0, "begin <= end\n");
    }else if (begin == end) {
        return 0;
    } else if (begin == end - 1) {
        char *argv[8];
        argv_package(argv, 8, begin, end);
        cmd_run(tokens[begin].str, argv, ground, pipes, rw);
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
            PASS("TK_PIPE");

            int pipe1[2];
            pipe(pipe1);
            pid_t pid;
            pid = fork();

            if (pid < 0) {
                NshError("fork(): nsh fork failed");
                exit(1);
            } else if (pid == 0) {
                /* write */
                
                work(tokens, left1, right1, ground, pipe1, WRITE);
                exit(1);
                // close(pipe1[1]);
                
                // printf("left2=%d, right2=%d\n", left2, right2);
                // sleep(1);
                // work(tokens, left2, right2, ground, pipe1, 0);
            } else {
                /* read */
                // close(pipe1[0]);
                int status;
                pid_t ret = waitpid(pid, &status, 0);
                Assert(ret != -1 && WIFEXITED(status), "exit sucuessful\n");
                // printf("(debug)%s\n",tokens[left2].str);
                work(tokens, left2, right2, ground, pipe1, READ);
                
                // dup2(pipe1[1], STDOUT_FILENO);
                // work(tokens, left1, right1, ground, pipe1, 1);
            }
            /* fork() */

            break;
        case TK_SEQ:
            PASS("TK_SEQ");
            work(tokens, left1, right1, ground, pipes, rw);
            work(tokens, left2, right2, ground, pipes, rw);
            break;
        case TK_BG:
            PASS("TK_BG");
            work(tokens, left1, right1, BACK, pipes, rw);
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
                    

                    FILE *fp;
                    if (left4 + 1 != right4) {
                        NshDebug("only have one file to write\n, but get %s ...\n", tokens[left4].str);
                        return -1;
                    }
                    // Assert(left4 + 1 == right4, "only have one file to write\n");

                    int stdout_fd = dup(fileno(stdout));

                    fp = freopen(tokens[left4].str, "w", stdout);
                    if (fp == NULL) {
                        NshError(">: can not open file %s\n", tokens[left4].str);
                        return -1;
                    }
                    work(tokens, left3, right3, ground, pipes, rw);
                    fclose(fp);
                    freopen("/dev/tty", "w", stdout);
                    close(stdout_fd);
                    break;
                default:
                    PASS("default-default");
                    char *argv[8];
                    argv_package(argv, 8, begin, end);
                    // printf("(debug)path=%s, argv[0]=%s, argv[1]=%s\n", tokens[begin].str, argv[0], argv[1]);
                    cmd_run(tokens[begin].str, argv, ground, pipes, rw);
                    break;

            }
            

        }
    }
    return 0;
}

int excute(char *e, bool *suc) {
    if(e == NULL) {
        return 0;
    }
    // printf("(debug)e=%s\n", e);
    if (!make_token(e)) {
        *suc = false;
        return 0;
    }
    return work(tokens, 0, nr_token, ON, NULL, -1);
}
int prase_args(char *e, bool *suc) {
    if (!make_token(e)) {
        *suc = false;
        return 0;
    }
    for (int j = 0; j < nr_token; j++) {
        // printf("(debug)token[%d].type=%d, .str=%s\n", j, tokens[j].type, tokens[j].str);
    }
    return 0;
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
    // printf("(debug)create item\n");
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
    // printf("(debug) add_bglist\n");
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
        free(temp);
        return;
    }
    BGItem *left = head;
    BGItem *right = head->next;
    for (; right->next != NULL; left = left->next, right = right->next) {
        if (right->pid == remove_pid) {
            left->next = right->next;
            free(right);
            return;
        }
    } 
    return;
}

/* always return */
void print_bglist() {
    NshPrint("index\tpid\tcmd\n");
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
    // NshPrint("nsh: begin to run %s\n", path);
    return execvp(path, argv);
}

void sigchld_handler(int signum) {
    /* TODO: remove background from bglist */
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
/* fork() and execve() the program */
int run(char *path, char **argv, int ground, int pipes[2], int rw) {
    signal(SIGCHLD, sigchld_handler);
    pid_t pid = fork();
    if (pid < 0) {
        NshError("fork(): nsh fork failed");
        exit(1);
    } else if (pid == 0) {
        /* child process, run exec() */
        if (pipes != NULL) {
            close(pipes[1 - rw]);
            if (rw == READ) {
                sleep(2);
                dup2(pipes[rw], STDIN_FILENO);
            } else if (rw == WRITE) {
                dup2(pipes[rw], STDOUT_FILENO);
            } else {
                Assert(0, "rw in {0, 1}\n");
            }
            // printf("(debug)pipes[0]=%d, pipes[1]=%d\n", pipes[0], pipes[1]);
        }

        if (ground == BACK) {
            // sleep(10);
            return program_exec(path, argv);
        }else {
            return program_exec(path, argv);
        }
    } else {
        /* parent process */
        if (pipes != NULL) {
            close(pipes[READ]);
            close(pipes[WRITE]);
        }

        if (ground == BACK) {
            BGItem *item = create_bg(pid, path);
            add_bglist(item);
        } else {
            int status;
            /* options=0 meaning for wait all sub-process */
            pid_t ret = waitpid(pid, &status, 0);
            if (ret == -1) {
                NshError("waitpid(): wait failed");
                // perror("wait failed\n");
            }
            /* if WIFEXITED(.)== 0, exit successfully
                else use WEXITSTATUS(.) to get exit code
            */
            if (WIFEXITED(status)) {
                // NshPrint("ON-process pid=%d successfullt exited with code=%d, path=%s.\n", pid, WEXITSTATUS(status), path);
            }else {
                NshError("WIFEXITED is wrong, sub-process exit unsuccessfully.\n");
            }
        }
    }
    return 0;
}

int main(){
    // nsh_print("nsh print test: %s, %d, %f7\n", "string", 1, 1.1);
    init();
    nsh_mainloop();
    // prase_args();
    return 0;
}