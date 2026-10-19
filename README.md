# Simple Shell

A Unix-style command-line shell implemented in C. The shell supports external command execution, foreground and background processes, built-in commands, signal handling, and command history. It uses operating system system calls for process management and provides basic shell functionality through an interactive command-line interface.

## Features

- **Command Execution:** Execute external programs in separate child processes.
- **Foreground and Background Processes:** Run commands synchronously or in the background using `&`.
- **Built-in Commands:** Support `cd`, `pwd`, `exit`, and `help` without creating additional processes.
- **Directory Navigation:** Change directories using absolute or relative paths, navigate to the home directory using `~`, and return to the previous directory using `-`.
- **Command History:** Store and display the 10 most recently entered commands, numbered sequentially.
- **History Execution:** Re-execute previous commands using `!n` or repeat the most recent command using `!!`.
- **Signal Handling:** Handle `SIGINT` (`Ctrl+C`) without terminating the shell.
- **Process Management:** Wait for foreground processes and clean up completed background processes.

## Built-in Commands

| Command | Description |
|---|---|
| `cd [directory]` | Change the current working directory. |
| `pwd` | Display the current working directory. |
| `help [command]` | Display help for built-in or external commands. |
| `history` | Display the 10 most recent commands. |
| `!n` | Execute a command from history by its command number. |
| `!!` | Execute the most recently entered command. |
| `exit` | Exit the shell. |

## Requirements

- C compiler (Clang or GCC)
- CMake
- A Unix-like operating system with POSIX system call support

## Build and Run

Build the project using CMake:

```bash
cmake -S . -B build
cmake --build build
```

Start the shell:

```bash
./build/shell
```

The shell displays the current working directory in its prompt and accepts commands interactively.

## Usage

Execute a command in the foreground:

```bash
ls -la
```

Execute a command in the background:

```bash
sleep 10 &
```

Navigate to the home directory:

```bash
cd
```

View recent commands:

```bash
history
```

Re-execute a previous command:

```bash
!5
```

Repeat the last command:

```bash
!!
```

Display available built-in commands:

```bash
help
```

## Technologies Used

- **C** – Core implementation and command processing.
- **POSIX System Calls** – Process creation, execution, and management using functions such as `fork()`, `exec()`, and `waitpid()`.
- **Signal Handling** – Handle interrupts using `SIGINT`.
- **CMake** – Build configuration and executable generation.
