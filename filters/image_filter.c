#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "bitmap.h"
#include <fcntl.h>


#define ERROR_MESSAGE "Warning: one or more filter had an error, so the output image may not be correct.\n"
#define SUCCESS_MESSAGE "Image transformed successfully!\n"


/*
 * Check whether the given command is a valid image filter, and if so,
 * run the process.
 *
 * We've given you this function to illustrate the expected command-line
 * arguments for image_filter. No further error-checking is required for
 * the child processes.
 */
void run_command(const char *cmd) {
    if (strcmp(cmd, "copy") == 0 || strcmp(cmd, "./copy") == 0 ||
        strcmp(cmd, "greyscale") == 0 || strcmp(cmd, "./greyscale") == 0 ||
        strcmp(cmd, "gaussian_blur") == 0 || strcmp(cmd, "./gaussian_blur") == 0 ||
        strcmp(cmd, "edge_detection") == 0 || strcmp(cmd, "./edge_detection") == 0) {
        execl(cmd, cmd, NULL);
    } else if (strncmp(cmd, "scale", 5) == 0) {
        // Note: the numeric argument starts at cmd[6]
        execl("scale", "scale", cmd + 6, NULL);
    } else if (strncmp(cmd, "./scale", 7) == 0) {
        // Note: the numeric argument starts at cmd[8]
        execl("./scale", "./scale", cmd + 8, NULL);
    } else {
        fprintf(stderr, "Invalid command '%s'\n", cmd);
        exit(1);
    }
}


int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: image_filter input output [filters...]\n");
        exit(1);
    }

    int num_filters = argc - 3;
    if (num_filters == 0) { 
        argv[3] = "copy";
        num_filters = 1;
    }

    int pipefds[num_filters - 1][2];

    // Create pipes
    for (int i = 0; i < num_filters - 1; i++) {
        if (pipe(pipefds[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    for (int i = 0; i < num_filters; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            if (i > 0) {
                dup2(pipefds[i - 1][0], STDIN_FILENO);
                close(pipefds[i - 1][1]);
            } else {
                int input_fd = open(argv[1], O_RDONLY);
                if (input_fd < 0) {
                    perror("open input file");
                    exit(1);
                }
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }

            if (i < num_filters - 1) {
                dup2(pipefds[i][1], STDOUT_FILENO);
                close(pipefds[i][0]);
            } else {
                int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (output_fd < 0) {
                    perror("open output file");
                    exit(1);
                }
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }

            run_command(argv[i + 3]);
            perror("run_command");
            exit(1);
        } 
    }

    for (int i = 0; i < num_filters - 1; i++) {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }

    int count_error = 0;
    for (int i = 0; i < num_filters; i++) {
        int status;
        wait(&status);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            count_error++;
            fprintf(stderr, ERROR_MESSAGE);
        }
    }

    if (count_error == 0) {
        printf(SUCCESS_MESSAGE);
    }
    return count_error > 0 ? 1 : 0;
}