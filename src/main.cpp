#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/types.h>
#include <vector>
#include <unistd.h>
#include <sstream>
#include <sys/wait.h>

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::string input;
    std::vector<std::string> builtins = {"echo", "type", "exit"};

    while (true) {
	std::cout << "$ ";
	if (!std::getline(std::cin, input)) {
	    break;
	}
	std::vector<std::string> args;
	std::stringstream ss(input);
	std::string token;
	while (ss >> token) {
	    args.push_back(token);
	}
	if (args.empty()) continue;
	std::string cmd = args[0];

	if (cmd=="exit") break;
	else if (cmd=="echo") {
	    for (size_t i=1; i<args.size(); i++) {
		std::cout << args[i] << (i+1 < args.size() ? " " : "");
	    }
	    std::cout << std::endl;
	}
	else if (cmd=="type") {
	    if (args.size() < 2) {
		std::cout << "type: missing operand" << std::endl;
		continue;
	    }
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
			if (dir.empty()) continue;
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
		std::cout << cmd_to_check << ": not found" << std::endl;
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
		execvp(c_args[0], c_args.data());
		std::cout << cmd << ": command not found" << std::endl;
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
    }
    return 0;
}

