#define _POSIX_C_SOURCE 200809L
#include "msgs.h"
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
// Handle exit command,
void if_exit(int count, char **history) {
  // if user provides any arguments, print error
  if (count > 1) {
    write(STDERR_FILENO, FORMAT_MSG("exit", TMA_MSG),
          strlen(FORMAT_MSG("exit", TMA_MSG)));
  } else {
    for (int i = 0; i < 10; i++) {
      free(history[i]);
    }
    exit(0);
  }
}

// Handle printing current working directory
void if_pwd(int count) {
  if (count > 1) {
    write(STDERR_FILENO, FORMAT_MSG("pwd", TMA_MSG),
          strlen(FORMAT_MSG("pwd", TMA_MSG)));
  } else {
    char buf[1000];
    if (getcwd(buf, 1000) == NULL) {
      write(STDERR_FILENO, FORMAT_MSG("pwd", GETCWD_ERROR_MSG),
            strlen(FORMAT_MSG("pwd", GETCWD_ERROR_MSG)));
    } else {
      write(STDOUT_FILENO, buf, strlen(buf));
      write(STDOUT_FILENO, "\n", 1);
    }
  }
}

char prev_directory[1000];

// handle changing current directory
void if_cd(int count, char **argument) {

  char buf[1000];
  if (getcwd(buf, sizeof(buf)) == NULL) {
    write(STDERR_FILENO, FORMAT_MSG("cd", GETCWD_ERROR_MSG),
          strlen(FORMAT_MSG("cd", GETCWD_ERROR_MSG)));
  }

  // if user enters too many command arguments, print error
  if (count > 2) {
    write(STDERR_FILENO, FORMAT_MSG("cd", TMA_MSG),
          strlen(FORMAT_MSG("cd", TMA_MSG)));
  }

  char *target;
  struct passwd *path = getpwuid(getuid());

  // user just enters cd
  if (count == 1) {
    target = path->pw_dir;
  } else if (strcmp(argument[1], "~") == 0) {
    target = path->pw_dir;
  } else if (argument[1][0] == '~') { // check only first char in the string
    char buf2[1000];
    snprintf(buf2, sizeof(buf2), "%s%s", path->pw_dir, argument[1] + 1);
    target = buf2;
  } else if (strcmp(argument[1], "-") == 0) {
    // if user is in /home and uses cd /, then cd - should go back to /home
    target = prev_directory;
  } else {
    target = argument[1];
  }

  if (chdir(target) == -1) {
    write(STDERR_FILENO, FORMAT_MSG("cd", CHDIR_ERROR_MSG),
          strlen(FORMAT_MSG("cd", CHDIR_ERROR_MSG)));
  } else {
    strcpy(prev_directory, buf);
  }
}

// Handle help command
void if_help(int count, char **argument) {
  // if user enters more than argument print error
  if (count > 2) {
    write(STDERR_FILENO, FORMAT_MSG("help", TMA_MSG),
          strlen(FORMAT_MSG("help", TMA_MSG)));
  }

  // if user enters no arguments, print help msg for all internal commands
  if (count == 1) {
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

  // if user enters internal/external command, display respective msg
  else {
    if (strcmp(argument[1], "exit") == 0) {
      write(STDOUT_FILENO, FORMAT_MSG("exit", EXIT_HELP_MSG),
            strlen(FORMAT_MSG("exit", EXIT_HELP_MSG)));

    } else if (strcmp(argument[1], "pwd") == 0) {
      write(STDOUT_FILENO, FORMAT_MSG("pwd", PWD_HELP_MSG),
            strlen(FORMAT_MSG("pwd", PWD_HELP_MSG)));

    } else if (strcmp(argument[1], "cd") == 0) {
      write(STDOUT_FILENO, FORMAT_MSG("cd", CD_HELP_MSG),
            strlen(FORMAT_MSG("cd", CD_HELP_MSG)));

    } else if (strcmp(argument[1], "help") == 0) {
      write(STDOUT_FILENO, FORMAT_MSG("help", HELP_HELP_MSG),
            strlen(FORMAT_MSG("help", HELP_HELP_MSG)));
    } else if (strcmp(argument[1], "help") == 0) {
      write(STDOUT_FILENO, FORMAT_MSG("help", HELP_HELP_MSG),
            strlen(FORMAT_MSG("help", HELP_HELP_MSG)));
    } else if (strcmp(argument[1], "history") == 0) {
      write(STDOUT_FILENO, FORMAT_MSG("history", HISTORY_HELP_MSG),
            strlen(FORMAT_MSG("history", HISTORY_HELP_MSG)));
    } else {
      char buf[1000];
      snprintf(buf, sizeof(buf), "%s: %s\n", argument[1], EXTERN_HELP_MSG);
      write(STDOUT_FILENO, buf, strlen(buf));
    }
  }
}

void if_history(int count, char **history) {
  int amount = (count < 10 ? count : 10);

  for (int i = 0; i < amount; i++) {
    int index = (count - 1 - i) % 10;
    int command_num = count - 1 - i;
    if (history[index] != NULL) {
      char hist[1000];
      snprintf(hist, sizeof(hist), "%d	%s", command_num, history[index]);
      write(STDOUT_FILENO, hist, strlen(hist));
    }
  }
}
