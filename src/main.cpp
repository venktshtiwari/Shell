#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/types.h>
#include <vector>
#include <unistd.h>
#include <sstream>
#include <sys/wait.h>
#include <fcntl.h>

namespace fs = std::filesystem;

int main() {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::string input;
    std::vector<std::string> builtins = {"echo", "type", "exit", "pwd"};

    while (true) {
        std::cout << "$ ";
        if (!std::getline(std::cin, input)) break;
        std::vector<std::string> args;
        std::string token = "";
        bool in_single_quote = false;
        bool in_double_quote = false;
        bool has_arg = false;
        int redirect = 0;
        std::string filename;

        // --- TOKENIZATION ---
        for (size_t i = 0; i < input.length(); i++) {
            char ch = input[i];

            if (in_single_quote) {
                if (ch == '\'') {
                    in_single_quote = false;
                    has_arg = true;
                } else {
                    token += ch;
                    has_arg = true;
                }
            } else if (in_double_quote){
                if (ch == '\"') {
                    in_double_quote = false;
                    has_arg = true;
                } else {
                    if (ch == '\\' && (i+1)<input.length()) {
                        if (input[i+1]=='\"' || input[i+1]=='\\') {
                            token += input[i+1];
                            has_arg = true;
                            i++;
                            continue;
                        }
                    }
                    token += ch;
                    has_arg = true;
                }
            } else {
                if (ch == '\'') {
                    in_single_quote = true;
                    has_arg = true;
                } else if (ch == '\"') {
                    in_double_quote = true;
                    has_arg = true;
                } else if (ch == ' ' || ch == '\t') {
                    if (has_arg) {
                    args.push_back(token);
                    token = "";
                    has_arg = false;
                    }
                } else if (ch=='\\' && i<input.length()-1) {
                    token += input[i+1];
                    has_arg = true;
                    i++;
                } else {
                    token += ch;
                    has_arg = true;
                }
            }
        }
        if (has_arg) args.push_back(token);
        if (args.empty()) continue;

        // ---  REDIRECTION  ---
        int file_desc = -1;
        int saved_stdout = -1;
        
        for (size_t i=0; i<args.size(); i++) {
            if (args[i]==">" || args[i] == "1>" || args[i] == "2>") {
                if ((i+1)<args.size()) {
                    redirect = (args[i] == "2>") ? 2 : 1;
                    filename = args[i+1];
                }
                else redirect = -1;
                args.erase(args.begin()+i, args.end());
                break;
            }
        }
        
        if (redirect == -1) {
            std::cerr << "No file to write to." << std::endl;
            continue;
        }
        else if (redirect > 0) {
            file_desc = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (file_desc < 0) {
                std::perror("Error opening file");
                continue;
            }
			saved_stdout = dup(redirect);
			dup2(file_desc, redirect);
        }

        // --- EXECUTION BLOCKS ---
        std::string cmd = args[0];
        
        if (cmd=="exit") {
            if (redirect > 0) {
				dup2(saved_stdout, redirect);
                close(file_desc);
                close(saved_stdout);
            }
            break;
        }
        else if (cmd=="echo") {
            for (size_t i=1; i<args.size(); i++) {
                std::cout << args[i] << (i+1 < args.size() ? " " : "");
            }
            std::cout << std::endl;
        }
        else if (cmd=="pwd") {
            fs::path cwd = fs::current_path();
            std::cout << cwd.string() << std::endl;
        }
        else if (cmd=="cd") {
            if (args.size() == 1 || args[1]=="~") {
                const char* home = getenv("HOME");
                if (home) fs::current_path(home);
            } else {
                fs::path new_dir = args[1];
                if (!fs::exists(new_dir)) {
                    std::cerr << "cd: " << args[1] << ": No such file or directory" << std::endl;
                } else {
                    fs::current_path(new_dir);
                }
            }
        }
        else if (cmd=="type") {
            if (args.size() < 2) {
                std::cerr << "type: missing operand" << std::endl;
            } else {
                std::string cmd_to_check = args[1];
                bool found = false;

                for (const auto& str: builtins) {
                    if (cmd_to_check==str) {
                        found = true;
                        std::cout << cmd_to_check << " is a shell builtin" << std::endl;
                        break;
                    }
                }
                if (!found) {
                    const char* path_env = std::getenv("PATH");
                    if (path_env) {
                        std::stringstream path_ss(path_env);
                        std::string dir;
                        while (std::getline(path_ss, dir, ':')) {
                            if (dir.empty()) continue; // Safe, it's an inner string-parsing loop
                            std::string full_path = dir + "/" + cmd_to_check;
                            if (access(full_path.c_str(), X_OK) == 0) {
                                std::cout << cmd_to_check << " is " << full_path << std::endl;
                                found = true;
                                break;
                            }
                        }
                    }
                }
                if (!found) {
                    std::cerr << cmd_to_check << ": not found" << std::endl;
                }
            }
        }
        else {
            std::vector<char*> c_args;
            for (auto& arg : args) {
                c_args.push_back(&arg[0]);
            }
            c_args.push_back(nullptr);

            pid_t pid = fork();

            if (pid == 0) {
                if (redirect>0) {
                    close(file_desc);
                    close(saved_stdout);
                }
                execvp(c_args[0], c_args.data());
                std::cerr << cmd << ": command not found" << std::endl;
                std::exit(1);
            }
            else if (pid>0) {
                int status;
                waitpid(pid, &status, 0);
            }
            else {
                std::cerr << "Failed to fork process" << std::endl;
            }
        }
        
        if (redirect > 0) {
			if (redirect == 1) std::cout << std::flush;
            else std::cerr << std::flush;            
			dup2(saved_stdout, redirect);
            close(file_desc);
            close(saved_stdout);
        }
    }
    return 0;
}