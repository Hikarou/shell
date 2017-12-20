#define _GNU_SOURCE

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#ifdef __GNUC__
#  define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_ ## x
#else
#  define UNUSED_FUNCTION(x) UNUSED_ ## x
#endif


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
static int do_exit(char **, int);

static int do_help(char **, int);

static int do_pwd(char **, int);

static int do_cd(char **c, int);

static int do_alias(char **c, int);

static int tokenize_input(char *input, char ***parsed, int *size_parsed);

static void print_introduction();

static environment_var *extract_environment_var(char *input);

static char *change_alias(char **first);

typedef int (*shell_fct)(char **fct, int); ///fct contains function name and args

struct shell_map {
    const char *name;    /// command name
    shell_fct fct;       /// function that is used
    const char *help;    /// command's description
    size_t argc;         /// number of arguments possible
    const char *args;    /// arguments description
};

node *alias_root; ///Binary tree which will contain all the environment variables

struct shell_map shell_cmds[NB_CMDS] = {
        {"help",  do_help,  "display this help.",                      0, NULL},
        {"exit",  do_exit,  "exit shell.",                             0, NULL},
        {"quit",  do_exit,  "exit shell.",                             0, NULL},
        {"pwd",   do_pwd,   "print name of current/working directory", 0, NULL},
        {"cd",    do_cd,    "change directory.",                       1, "[<pathname>]"},
        {"alias", do_alias, "define or display aliases",               1, "[alias-name[=string]]"},
};

int main() {
    char input[MAX_READ + 1] = "";
    char **parsed = NULL;
    int err = 0;
    int size_parsed = 0;
    int k = 0;
    shell_fct function = NULL;
    alias_root = NULL;

    //reading the profile file
    FILE *fp;
    fp = fopen("profile", "r");
    if (fp == NULL) {
        fprintf(stderr, "no profile file found!\n");
        exit(EXIT_FAILURE);
    }

    char *buf = NULL;
    size_t size_buf = 0;
    ssize_t nread;
    //Get all the entries
    while ((nread = getline(&buf, &size_buf, fp)) != -1) {
        //Clean the line from \n if needed
        size_t position_last_char = strlen(buf) - 1;
        if (buf[position_last_char] == '\n') {
            buf[position_last_char] = '\0';
        }
        environment_var *tuple = extract_environment_var(buf);
        if (tuple != NULL) {
            setenv(tuple->var, tuple->content, 1);
            printf("insert_node done\n");
            fflush(stdout);
        }
        free_env_var(tuple);
    }

    fclose(fp);
    free(buf);

    //Least environment variables needed
    if (getenv("PATH") == NULL || getenv("HOME") == NULL) {
        fprintf(stderr, "Profile file does not contain either PATH or HOME variable !\n");
        exit(EXIT_FAILURE);
    }


    //Reading the standard input
    while ((!feof(stdin) && !ferror(stdin)) && err != EXIT) {
        print_introduction();
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
                    // New environment variable
                    if (size_parsed == 1 && strchr(input, '=') != NULL) {
                        environment_var *new = extract_environment_var(parsed[0]);
                        setenv(new->var, new->content, 1); //1 for overwriting
                        free_env_var(new);
                    } else if (size_parsed == 1 && parsed[0][0] == '$') { //Printing the envrionment asked
                        size_t length_var = strlen(parsed[0]) - 1;
                        char *var = calloc(length_var, sizeof(char));
                        assert(var);
                        strncpy(var, parsed[0] + 1, length_var);
                        char *content = getenv(var);
                        if (content != NULL) {
                            fprintf(stdout, "$%s=%s\n", var, content);
                        } else {
                            fprintf(stdout, "\n");
                        }
                        free(var);
                    } else {
                        // if alias created, need to change TODO
                        char *replace = change_alias(&parsed[0]);


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
            }
            free(parsed);
        } else {
            perror("Calloc Error\n");
            exit(EXIT_FAILURE);
        }
        fflush(stderr);
        fflush(stdout);
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
    *(size_parsed) = 0;

    if (input == NULL || parsed == NULL) {
        return ERR_ARGS;
    }

    size_t l = strlen(input);
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
        } else { // (k == 1) beginning of a line or after a space char
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

/**
 * Exits the shell
 * @param UNUSED_c
 * @param UNUSED_nb_args
 * @return EXIT : The value to exit the shell
 */
static int do_exit(char **UNUSED(c), int UNUSED(nb_args)) {
    return EXIT;
}

/**
 * Prints the help for all the commands possible and their arguments
 * @param UNUSED_c
 * @param UNUSED_nb_args
 * @return ERR_OK : The value to continue the shell
 */
static int do_help(char **UNUSED(c), int UNUSED(nb_args)) {
    for (int i = 0; i < NB_CMDS; ++i) {
        fprintf(stdout, "- %s", shell_cmds[i].name);
        if (shell_cmds[i].argc > 0) {
            fprintf(stdout, " %s", shell_cmds[i].args);
        }
        fprintf(stdout, ": %s\n", shell_cmds[i].help);
    }
    return ERR_OK;
}

/**
 * Prints the current working directory
 * @param UNUSED_c
 * @param UNUSED_nb_args
 * @return ERR_OK : The value to continue the shell
 */
static int do_pwd(char **UNUSED(c), int UNUSED(nb_args)) {
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
    free(buf);
    return ERR_OK;
}

/**
 * Change the working directory
 * @param c The line tokenize (cd [<pathname>])
 * @param nb_args Nb of args given
 * @return ERR_OK if it went ok, ERR_ARGS otherwise
 */
static int do_cd(char **c, int nb_args) {
    char *pathname;
    // Take car of the cd without argument (Goes to the HOME path)
    if (nb_args == 0) {
        //pathname = search(env_var_root, "HOME")->data->content; //Should always work since TODO Ask Hoerdt
        pathname = getenv("HOME");
    } else if (nb_args > 1) {
        return ERR_ARGS;
    } else {
        pathname = c[1];
    }

    // Verbose on error with chdir
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
        return ERR_ARGS;
    }
    return ERR_OK;
}

/**
 * define or display aliases
 * @param c The line tokenize (alias [<pathname>])
 * @param nb_args number of arguments given
 * @return ERR_OK
 */
static int do_alias(char **c, int nb_args) {
    //Alias called without argument
    if (nb_args == 0) {
        display_tree(alias_root);
        return ERR_OK;
    }

    //If multiple aliases, take care of all of them
    for (int i = 0; i < nb_args; ++i) {
        environment_var *alias = extract_environment_var(c[1]);
        if (alias == NULL) { //alias called without '=' in arguments
            node *nd = search(alias_root, c[1]);
            if (nd != NULL) {
                display(nd);
            }
            free(alias);
        } else {
            //Create or update the new environment variable
            insert_node(alias_root, alias);
            free(alias);
        }
    }
    return ERR_OK;
}

/**
 * prints the introduction of the shell
 */
static void print_introduction() {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);

    timeinfo = localtime(&rawtime);
    char *buf = asctime(timeinfo);
    char *bufBis = calloc(strlen(buf), sizeof(char));
    strncpy(bufBis, buf, strlen(buf) - 1);
    fprintf(stdout, "%s | Interpreter! >", bufBis);
    free(bufBis);
}

/**
 * Extracts from the input the variable and the content from a line
 * @param input The line to extract from
 * @return the splitted line
 */
static environment_var *extract_environment_var(char *input) {
    //Find the '=' char
    char *middle = strchr(input, '=');
    if (middle == NULL) {
        return NULL;
    }

    unsigned long var_length = middle - input;
    unsigned long content_length = strlen(input) - var_length + 1;
    environment_var *tuple = malloc(sizeof(environment_var));
    assert(tuple);
    tuple->var = calloc(var_length + 1, sizeof(char));//+1 for null terminating string
    if (tuple->var == NULL) {
        free(tuple);
        fprintf(stderr, "Calloc failed in environment_var function\n");
        exit(EXIT_FAILURE);
    }

    tuple->content = calloc(content_length + 1, sizeof(char));
    if (tuple->content == NULL) {
        free(tuple->var);
        free(tuple);
        fprintf(stderr, "Calloc failed in environment_var function\n");
        exit(EXIT_FAILURE);
    }

    tuple->var = strncpy(tuple->var, input, var_length);
    assert(tuple->var);
    tuple->content = strncpy(tuple->content, middle + 1, content_length - 1); //+1 to get rid of the =
    assert(tuple->content);

    return tuple;
}

/**
 * Change the alias if needed
 * @param first the first occurence to change
 * @return the alias changed
 */
static char *change_alias(char **first) { //TODO
    char *current = *first;
    do {
        node *nd = search(alias_root, current);
        if (nd != NULL) {
            current = nd->data->content;
        } else {
            break;
        }
    } while (1);
    return current;
}