Minimal C++ POSIX Shell

A lightweight, custom UNIX shell implementation written in C++. This project demonstrates the core mechanics of a command-line interpreter, including a Read-Eval-Print Loop (REPL), argument tokenization, built-in command handling, and process execution using the POSIX system calls fork, execvp, and waitpid.
🚀 Features

    Interactive REPL: Displays a persistent $  prompt and handles user input gracefully.

    Unbuffered I/O: Configured with std::unitbuf to ensure immediate flushing of stdout and stderr.

    Tokenization: Parses input strings into distinct command arguments, splitting by whitespace.

    Built-in Commands:

        exit: Safely terminates the shell session.

        echo: Prints arguments back to the terminal, matching classic shell behavior.

        type: Identifies whether a command is an internal shell builtin or an external executable located in the system's PATH.

    External Command Execution: Leverages fork() and execvp() to execute standard system binaries (e.g., ls, cat, grep, mkdir) in a dedicated child process while the parent shell waits for completion.

🛠️ How It Works
1. The Core Loop (REPL)

The shell continuously reads input from std::cin inside a while(true) loop. If it encounters an End-Of-File (EOF / Ctrl+D), it breaks out cleanly.
2. Builtins vs. Externals

When a command is entered:

    It is split into tokens using std::stringstream.

    The shell checks if the primary command matches any string in the builtins vector.

    If it's a type request, it explicitly checks the builtins list first, then manually parses the PATH environment variable using std::getenv("PATH") and verifies execution permissions via access(..., X_OK).

    If it's an external command, the shell shifts to system process management.

3. Process Management

To prevent external commands from replacing the shell process itself, the shell forks:

    Child Process (pid == 0): Converts C++ std::string vectors into a null-terminated array of char* to remain compatible with the POSIX C API, then invokes execvp.

    Parent Process (pid > 0): Catches the child PID and executes waitpid(pid, &status, 0), blocking the shell prompt until the child process finishes execution.

📦 Getting Started
Prerequisites

You need a C++ compiler supporting at least C++11 (like g++ or clang++) and a POSIX-compliant operating system (Linux, macOS, WSL).
Compilation

Compile the source file using your preferred compiler:
Bash

g++ -std=c++17 main.cpp -o myshell

Running the Shell

Launch the executable directly from your terminal:
Bash

./myshell

💻 Usage Examples
Bash

$ echo Hello World
Hello World

$ type echo
echo is a shell builtin

$ type ls
ls is /usr/bin/ls

$ ls -la
total 32
drwxr-xr-x  2 user user 4096 Jun 12 12:00 .
drwxr-xr-x 20 user user 4096 Jun 12 11:50 ..
-rwxr-xr-x  1 user user 17320 Jun 12 12:00 myshell

$ type invalid_command
invalid_command: not found

$ exit

🗺️ Future Roadmap

While functional as a baseline shell, this architecture can be expanded to support production-level shell features:

    [ ] Advanced Tokenization: Add support for single/double quotes and backslash escaping to handle arguments containing spaces.

    [ ] I/O Redirection: Implement >, >>, and < operations using dup2 and open.

    [ ] Piping: Connect multiple processes via pipelines (cmd1 | cmd2) using pipe().

    [ ] Signal Handling: Catch Ctrl+C (SIGINT) so it interrupts the running child process instead of killing the entire shell.

    [ ] Path Auto-completion: Implement tab-completion for system binaries and directories.

📄 License

This project is open-source and available under the MIT License.
