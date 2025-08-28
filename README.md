# `minibash`: A Minimalist UNIX Shell

`minibash` is a shell written in C that processes and executes user commands. It supports a variety of operations, including command execution, I/O redirection, piping, and sequential and conditional command flow. The shell is built entirely with system calls like `fork()`, `execvp()`, and `waitpid()`, without using the `system()` library function.

---

## Key Features

### Special Character Operations

* **`#` (Word Count)**: Counts words in a specified `.txt` file.
    * Usage: `minibash$ # sample.txt`
* **`~` (File Concatenation)**: Concatenates up to four `.txt` files to standard output.
    * Usage: `minibash$ file1.txt ~ file2.txt`
* **`+` (Background Execution)**: Runs a command in the background. The `fore` command can bring the last background process to the foreground.
    * Usage: `minibash$ command arg1 +`
    * Foreground: `minibash$ fore`

---

### Command Redirection and Piping

* **`|` (Piping)**: Pipes the standard output of one command to the standard input of the next. Supports up to four pipes.
    * Usage: `minibash$ ls -l | grep ".c"`
* **`<`, `>`, `>>` (I/O Redirection)**:
    * **`<`**: Redirects standard input.
    * **`>`**: Redirects standard output, overwriting the file.
    * **`>>`**: Redirects standard output, appending to the file.

---

### Execution Flow

* **`;` (Sequential Execution)**: Executes up to four commands in sequence.
    * Usage: `minibash$ cmd1; cmd2; cmd3`
* **`&&` and `||` (Conditional Execution)**:
    * **`&&`**: Executes the next command only if the previous one succeeds.
    * **`||`**: Executes the next command only if the previous one fails.
    * These can be combined for complex conditional logic.

---

## Technical Specifications

* **Compilation**: `gcc minibash.c -o minibash`
* **Execution**: `./minibash`
* **Argument Count (`argc`)**: All commands must have an argument count between 1 and 4.