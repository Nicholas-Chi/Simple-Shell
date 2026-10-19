#define _POSIX_C_SOURCE 200809L
#include "internal_commands.h"
#include "msgs.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

char *history[10] = {NULL};
char total = 0;
// Get current working directory and display it in shell
void current_directory() {
  char directory[1000];
  if (getcwd(directory, sizeof(directory)) == NULL) {
    // Failed to get cwd
    write(STDERR_FILENO, FORMAT_MSG("shell", GETCWD_ERROR_MSG),
          strlen(FORMAT_MSG("shell", GETCWD_ERROR_MSG)));
  } else {
    write(STDOUT_FILENO, directory, strlen(directory));
    write(STDOUT_FILENO, "$ ", 2);
  }
}

// Clean up zombies
void clean_zombies() {
  int wstatus = 0;
  // -1 for any child, and nohang in the case there are no children
  // successful calls will return pid of child, hence > 0
  while (waitpid(-1, &wstatus, WNOHANG) > 0) {
  }
}

// execute shell commands
void execute(char *command) {
  bool background = false;
  char *buff = command;
  char *saveptr = NULL;
  char *token = NULL;
  char *delim = " \t\n\r";

  // Store commands into array to be used in exec
  char *arguments[1000];
  int i = 0;

  while ((token = strtok_r(buff, delim, &saveptr)) != NULL) {
    arguments[i] = token;
    i++;
    buff = NULL;
  }

  arguments[i] = NULL;

  if (strcmp(arguments[0], "!!") == 0) {

    if (total != 0 && history[(total - 1) % 10] != NULL) {
      int index = total - 1;
      char *buff3 = strdup(history[index % 10]);
      write(STDOUT_FILENO, buff3, strlen(buff3));
      if (history[total % 10] != NULL) {
        free(history[total % 10]);
      }
      history[total % 10] = strdup(buff3);
      total++;
      char *saveptr3 = NULL;
      char *token3 = NULL;
      char *delim3 = " \t\n\r";
      i = 0;

      while ((token3 = strtok_r(buff3, delim3, &saveptr3)) != NULL) {
        arguments[i] = token3;
        i++;
        buff3 = NULL;
      }

      arguments[i] = NULL;
      free(buff3);

    } else {
      write(STDERR_FILENO, FORMAT_MSG("history", HISTORY_NO_LAST_MSG),
            strlen(FORMAT_MSG("history", HISTORY_NO_LAST_MSG)));
      return;
    }
  } else if (arguments[0] != NULL && arguments[0][0] == '!') {
    char *loc = arguments[0] + 1;
    // use strspn(), taken from
    // www.tutorialspot.com/c_standard_library/c_function_strspn.htm
    // find length of loc with only characters in "0123456789"
    if (strspn(loc, "0123456789") != strlen(loc)) {
      write(STDERR_FILENO, FORMAT_MSG("history", HISTORY_INVALID_MSG),
            strlen(FORMAT_MSG("history", HISTORY_INVALID_MSG)));
      return;
    }
    int comm_num = atoi(loc);

    if (history[comm_num % 10] != NULL) {
      char *buff2 = strdup(history[comm_num % 10]);
      write(STDOUT_FILENO, buff2, strlen(buff2));
      if (history[total % 10] != NULL) {
        free(history[total % 10]);
      }
      history[total % 10] = strdup(buff2);
      total++;
      char *saveptr2 = NULL;
      char *token2 = NULL;
      char *delim2 = " \t\n\r";
      i = 0;

      while ((token2 = strtok_r(buff2, delim2, &saveptr2)) != NULL) {
        arguments[i] = token2;
        i++;
        buff2 = NULL;
      }

      arguments[i] = NULL;
      free(buff2);
    } else {
      write(STDERR_FILENO, FORMAT_MSG("history", HISTORY_INVALID_MSG),
            strlen(FORMAT_MSG("history", HISTORY_INVALID_MSG)));
      return;
    }
  }

  // Check if there is an & in command
  if (i > 0 && strcmp(arguments[i - 1], "&") == 0) {
    background = true;
    arguments[i - 1] = NULL;
  }

  if (i == 0 || arguments[0] == NULL) {
    return;
  }

  if (strcmp(arguments[0], "exit") == 0) {
    if_exit(i, history);
    return;
  } else if (strcmp(arguments[0], "pwd") == 0) {
    if_pwd(i);
    return;
  } else if (strcmp(arguments[0], "cd") == 0) {
    if_cd(i, arguments);
    return;
  } else if (strcmp(arguments[0], "help") == 0) {
    if_help(i, arguments);
    return;
  } else if (strcmp(arguments[0], "history") == 0) {
    if_history(total, history);
    return;
  }

  pid_t pid = fork();

  // if fork() fails
  if (pid == -1) {
    write(STDERR_FILENO, FORMAT_MSG("shell", FORK_ERROR_MSG),
          strlen(FORMAT_MSG("shell", FORK_ERROR_MSG)));
  } else if (pid == 0) {
    if (execvp(arguments[0], arguments) == -1) {
      write(STDERR_FILENO, FORMAT_MSG("shell", EXEC_ERROR_MSG),
            strlen(FORMAT_MSG("shell", EXEC_ERROR_MSG)));
    }
  } else {
    // Handle & condtion
    if (background == true) {
      //
    } else {
      if (waitpid(pid, NULL, 0) == -1) {
        write(STDERR_FILENO, FORMAT_MSG("shell", WAIT_ERROR_MSG),
              strlen(FORMAT_MSG("shell", WAIT_ERROR_MSG)));
      }
    }
  }
}

// Signal handler function that will print help message for internal commands
void handle_sigint(int signum) {
  write(STDOUT_FILENO, "\n", 1);
  write(STDOUT_FILENO, FORMAT_MSG("exit", EXIT_HELP_MSG),
        strlen(FORMAT_MSG("exit", EXIT_HELP_MSG)));
  write(STDOUT_FILENO, FORMAT_MSG("pwd", PWD_HELP_MSG),
        strlen(FORMAT_MSG("pwd", PWD_HELP_MSG)));
  write(STDOUT_FILENO, FORMAT_MSG("cd", CD_HELP_MSG),
        strlen(FORMAT_MSG("cd", CD_HELP_MSG)));
  write(STDOUT_FILENO, FORMAT_MSG("help", HELP_HELP_MSG),
        strlen(FORMAT_MSG("help", HELP_HELP_MSG)));
  write(STDOUT_FILENO, FORMAT_MSG("history", HISTORY_HELP_MSG),
        strlen(FORMAT_MSG("history", HISTORY_HELP_MSG)));
}

// Pass user command into execute
int main() {
  char command[1000];
  ssize_t input;

  struct sigaction act;
  act.sa_handler = handle_sigint;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  int ret = sigaction(SIGINT, &act, NULL);
  if (ret == -1) {
    perror("Sigaction failed");
    exit(EXIT_FAILURE);
  }

  // ctrl-c seems to break the loop, first command entered after doesn't work
  // sets read errno to EINTR upon signal interupt
  while (true) {
    current_directory();
    // clean_zombies();
    input = read(STDIN_FILENO, command, sizeof(command));

    // check if read() returns an error
    if (input == -1) {
      if (errno == EINTR) {
        continue; // reset the if loop
      }
      write(STDERR_FILENO, FORMAT_MSG("shell", READ_ERROR_MSG),
            strlen(FORMAT_MSG("shell", READ_ERROR_MSG)));
      continue;
    }

    command[input] =
        '\0'; // Needed since read() doesn't add null terminating character
    // if (history[total % 10] != NULL) {
    //   free(history[total % 10]);
    // }
    //  Use strdup() from www.geeksforgeeks.org/strdup-strndup-functions-c/
    //  will copy string from pointer, but needs to be freed
    if (command[0] == '!') {
      // clean_zombies();
      execute(command);
    } else {
      if (history[total % 10] != NULL) {
        free(history[total % 10]);
      }

      history[total % 10] = strdup(command);
      total++;
      // clean_zombies();
      execute(command);
    }
    clean_zombies();
  }

  return 0;
}
