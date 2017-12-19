#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include "binary_tree.h"

#define MAX_READ 255
#define NB_CMDS 6 //Change to something not static
#define ERR_OK 0
#define EXIT 1
#define ERR_ARGS 2
#define NOT_IMPLEMENTED 5
#define NB_ARGS 1

//Static declarations
static int do_exit(char **c, int);

static int do_help(char **c, int);

static int do_pwd(char **c, int);

static int do_cd(char **c, int);

static int do_alias(char **c, int);

static int tokenize_input(char *input, char ***parsed, int *size_parsed);

static void print_introduction();

static environment_var *extract_environment_var(char *input);

typedef int (*shell_fct)(char **fct, int); ///fct contains function name and args

struct shell_map {
    const char *name;    /// command name
    shell_fct fct;       /// function that is used
    const char *help;    /// command's description
    size_t argc;         /// number of arguments possible
    const char *args;    /// arguments description
};

node *env_var_root; ///Binary tree which will contain all the environment variables

struct shell_map shell_cmds[] = {
        {"help",  do_help,  "display this help.",                      0, NULL},
        {"exit",  do_exit,  "exit shell.",                             0, NULL},
        {"quit",  do_exit,  "exit shell.",                             0, NULL},
        {"pwd",   do_pwd,   "print name of current/working directory", 0, NULL},
        {"cd",    do_cd,    "change directory.",                       1, "<pathname>"},
        {"alias", do_alias, "define or display aliases",               1, "[alias-name[=string]]"},
};

int main() {

    printf("Init\n");
    fflush(stdout);
    char input[MAX_READ + 1] = "";
    char **parsed = NULL;
    int err = 0;
    int size_parsed = 0;
    int k = 0;
    shell_fct function = NULL;
    env_var_root = NULL;

    printf("before open profile file\n");
    fflush(stdout);

    //reading the profile file
    FILE *fp;
    fp = fopen("profile", "r");
    if (fp == NULL) {
        fprintf(stderr, "no profile file found!\n");
        exit(EXIT_FAILURE);
    }


    printf("before init profile file\n");
    fflush(stdout);
    char *buf = NULL;
    size_t size_buf = 0;
    ssize_t nread;
    printf("before reading profile file\n");
    fflush(stdout);
    while ((nread = getline(&buf, &size_buf, fp)) != -1) {
        printf("Retrieved line of length %zu\n", nread);
        fflush(stdout);
        environment_var *tuple = extract_environment_var(buf);
        printf("extract_environment_var done\n");
        fflush(stdout);
        if (tuple != NULL) {
            env_var_root = insert_node(env_var_root, tuple);
            printf("insert_node done\n");
            fflush(stdout);
        }
        free(tuple);
    }

    free(buf);
    fclose(fp);


    printf("before checking profile file\n");
    fflush(stdout);
    if (search(env_var_root, "PATH") == NULL || search(env_var_root, "HOME") == NULL) {
        fprintf(stderr, "Profile file does not contain either PATH or HOME variable !\n");
        exit(EXIT_FAILURE);
    }

    print_introduction();

    //Reading the standard input
    while ((!feof(stdin) && !ferror(stdin)) && err != EXIT) {
        size_parsed = 0;
        k = 0;

        parsed = calloc(NB_ARGS, sizeof(char *));
        if (parsed != NULL) {
            fgets(input, MAX_READ, stdin);
            size_t position_last_char = strlen(input) - 1;

            //Cleaning the last \n if it exists
            if (input[position_last_char] == '\n') {
                input[position_last_char] = '\0';
            }
            input[MAX_READ] = '\0';

            // take care if end of file or error in file
            if (feof(stdin) || ferror(stdin)) {
                err = EXIT;
            } else {
                //tokenize the line
                err = tokenize_input(input, &parsed, &size_parsed);
                if (err == ERR_OK) {

                    //finding the function asked
                    do {
                        err = strcmp(parsed[0], shell_cmds[k].name);
                        ++k;
                    } while (k < NB_CMDS && err != 0);
                    --k;
                    if (err == 0) {
                        function = shell_cmds[k].fct;
                        if ((shell_cmds[k]).argc >= (size_t) size_parsed - 1) { //Does it have the right arguments ?
                            err = function(parsed, size_parsed - 1);
                            if (err < 0) { //The function returned an error
                                perror("Function returned an error\n");
                            }
                        } else {
                            printf("ERROR SHELL: wrong number of arguements\n");
                            err = ERR_ARGS;
                        }
                    } else { //Command does not exist or not implemented
                        printf("ERROR SHELL: Invalid command\n");
                        err = ERR_ARGS;
                    }
                }
            }
            free(parsed);
        } else {
            perror("Calloc Error\n");
            exit(EXIT_FAILURE);
        }
        fflush(stderr);
        fflush(stdout);
        print_introduction();
    }
    return EXIT_SUCCESS;
}

/**
 * @brief tokenize the input given
 * @param input the input to tonkenize (IN)
 * @param parsed the input tokenized (OUT)
 * @param size_parsed the length of parsed
 * @return 0 on success; >0 on error
 */
static int tokenize_input(char *input, char ***parsed, int *size_parsed) {
    char *ptr = NULL;
    int size = NB_ARGS;

    int k = 1;
    int i = 0;
    size_t l = strlen(input);

    *(size_parsed) = 0;

    if (input == NULL || parsed == NULL) {
        return ERR_ARGS;
    }

    if (l <= 0) {
        return ERR_ARGS;
    }

    ptr = input;

    do {
        if (k == 0) { //in the middle of a word
            if (input[i] == ' ') {// new word
                k = 1;
                if (i > 0) {
                    input[i] = '\0';//finish the word
                    if (*size_parsed > size - 1) {
                        ++size;
                        *parsed = realloc(*parsed, sizeof(char *) * size);
                        if (*parsed == NULL) return EXIT;
                    }
                    (*parsed)[*(size_parsed)] = ptr;
                    *(size_parsed) = *(size_parsed) + 1;
                }
            }
        } else { //beginning of a line or after a space char
            if (input[i] == ' ') { //get rid of multiple spaces
                k = 1;
            } else { //beginning of a new word
                ptr = input + i;
                k = 0;
            }
        }
        ++i;
    } while (i < l);

    //Acknowledge the last word if not finished before
    if (k == 0) {
        if (*size_parsed > size - 1) {
            ++size;
            *parsed = realloc(*parsed, sizeof(char) * size * MAX_READ);
            if (*parsed == NULL) {
                perror("Realloc Error\n");
                exit(EXIT_FAILURE);
            }
        }
        (*parsed)[*(size_parsed)] = ptr;
        *(size_parsed) = *(size_parsed) + 1;
    }

    return 0;
}

static int do_exit(char **c, int nb_args) {
    return EXIT;
}

static int do_help(char **c, int nb_args) {
    for (int i = 0; i < NB_CMDS; ++i) {
        printf("- %s", shell_cmds[i].name);
        if (shell_cmds[i].argc > 0) {
            printf(" %s", shell_cmds[i].args);
        }
        printf(": %s\n", shell_cmds[i].help);
    }
    return ERR_OK;
}

static int do_pwd(char **c, int nb_args) {
    unsigned int size = 128;
    char *buf = NULL;
    do {
        size *= 2;
        buf = realloc(buf, size * sizeof(char));
        if (buf == NULL) {
            fprintf(stderr, "Realloc of buffer failed\n");
            exit(EXIT_FAILURE);
        }
        buf = getcwd(buf, size); //NULL terminated string
        //Fetch the pathname even though the initial allocated memory isn't enough
    } while (buf == NULL && errno == ERANGE);

    //If it is null here, then another problem other than size of buffer
    if (buf == NULL) {
        fprintf(stderr, "PWD failed to get the pathname\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, buf);
    fprintf(stdout, "\n");
    return ERR_OK;
}

static int do_cd(char **c, int nb_args) {
    char *pathname;
    if (nb_args == 0) {
        pathname = search(env_var_root,"HOME")->data->content; //Should always work since
    } else {
        pathname = c[1];
    }
    if (chdir(pathname) != 0) {
        switch (errno) {
            case EACCES:
                fprintf(stderr, "Search permission is denied for any component of the pathname\n");
                break;
            case ELOOP:
                fprintf(stderr, "A loop exists in symbolic links encountered during resolution of the path argument\n");
                break;
            case ENAMETOOLONG:
                fprintf(stderr, "The length of a component of a pathname is longer than {NAME_MAX}\n");
                break;
            case ENOENT:
                fprintf(stderr, "A component of path does not name an existing directory or path is an empty string\n");
                break;
            case ENOTDIR:
                fprintf(stderr,
                        "A component of the pathname names an existing file that is neither a directory nor a symbolic link to a directory\n");
                break;
            default:
                fprintf(stderr, "Something went wrong with cd\n");
                break;
        }
        return -1;
    }
    return ERR_OK;
}

static int do_alias(char **c, int nb_args) {
    //Alias called without argument
    if (nb_args == 0) {
        display_tree(env_var_root);
        return ERR_OK;
    }

    environment_var *alias = extract_environment_var(c[1]);
    if (alias == NULL) { //alias called without '=' in arguments
        node *nd = search(env_var_root, c[1]);
        if (nd != NULL) {
            display(nd);
        }
        free(alias);
        return ERR_OK;
    }
    //Create or update the new environment variable
    insert_node(env_var_root, alias);
    free(alias);
    return ERR_OK;
}

static void print_introduction() {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);

    timeinfo = localtime(&rawtime);
    char *buf= asctime(timeinfo);
    char *bufBis = calloc(strlen(buf), sizeof(char));
    strncpy(bufBis, buf, strlen(buf)-1);
    fprintf(stdout, "%s | Interprete ! >", bufBis);
    free(bufBis);
}

static environment_var *extract_environment_var(char *input) {
    printf("extract_environment init\n");
    fflush(stdout);
    char *middle = strchr(input, '=');
    if (middle == NULL) {
        return NULL;
    }
    printf("extract_environment found middle\n");
    fflush(stdout);

    unsigned long var_length = middle - input;
    unsigned long content_length = strlen(input) - var_length + 1;
    environment_var *tuple = malloc(sizeof(environment_var));
    assert(tuple);
    tuple->var = calloc(var_length + 1, sizeof(char));//+1 for null terminating string
    tuple->content = calloc(content_length + 1, sizeof(char));

    if (tuple->var == NULL || tuple->content == NULL) {
        fprintf(stderr, "Calloc failed in environment_var function\n");
        exit(EXIT_FAILURE);
    }
    printf("extract_environment sizes found : %zu, %zu\n", var_length, content_length);
    fflush(stdout);

    tuple->var = strncpy(tuple->var, input, var_length);
    assert(tuple->var);
    printf("extract_environment first strncpy\n");
    fflush(stdout);
    tuple->content = strncpy(tuple->content, middle + 1, content_length - 1); //+1 to get rid of the =
    assert(tuple->content);
    printf("extract_environment second strncpy\n");
    fflush(stdout);

    return tuple;
}