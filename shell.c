#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_READ 255
#define NB_CMDS 13 //Change to something not static
#define ERR_OK 0
#define EXIT 1
#define ERR_ARGS 2
#define NOT_IMPLEMENTED 5
#define NB_ARGS 1

//Static declarations
static int do_exit();
static int do_help();
static int tokenize_input(char *input, char ***parsed, int *size_parsed);
static void print_introduction();

typedef int (*shell_fct)(char **fct);

struct shell_map {
    const char *name;    /// nom de la commande
    shell_fct fct;       /// fonction réalisant la commande
    const char *help;    /// description de la commande
    size_t argc;         /// nombre d'arguments de la commande
    const char *args;    /// description des arguments de la commande
};

struct shell_map shell_cmds[] = {
        {"help",  do_help,  "display this help.",                                                            0, NULL},
        {"exit",  do_exit,  "exit shell.",                                                                   0, NULL},
        {"quit",  do_exit,  "exit shell.",                                                                   0, NULL},
        //{"ls",    do_lsall, "list all directories and files contained in the currently mounted filesystem.", 0, ""},
};

int main() {
    char input[MAX_READ + 1] = "";
    char **parsed = NULL;
    int err = 0;
    int size_parsed = 0;
    int k = 0;
    shell_fct function = NULL;

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
                        if ((shell_cmds[k]).argc == (size_t) size_parsed - 1) { //Does it have the right arguments ?
                            err = function(parsed);
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
                if (err != EXIT) {
                    print_introduction();
                }
            }
            free(parsed);
        } else {
            perror("Calloc Error\n");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
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

    //Acknoledge the last word if not finished before
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

static int do_exit() {
    return EXIT;
}

static int do_help() {
    for (int i = 0; i < NB_CMDS; ++i) {
        printf("- %s", shell_cmds[i].name);
        if (shell_cmds[i].argc > 0) {
            printf(" %s", shell_cmds[i].args);
        }
        printf(": %s\n", shell_cmds[i].help);
    }
    return ERR_OK;
}

static void print_introduction() {
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);

    timeinfo = localtime (&rawtime);
    fprintf(stdout,"%s Interprete ! >",asctime(timeinfo));
}