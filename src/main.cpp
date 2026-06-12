#include <iostream>
#include <string>
#include <vector>

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    std::string input;
    bool flag;
    std::cout << "$ ";
    std::vector<std::string> builtins = {"echo", "type", "exit"};
    while (std::getline(std::cin, input)) {
	flag=0;
	if (input=="exit") break;
	else if (input.substr(0,5)=="echo ") std::cout << input.substr(5) << std::endl;
	else if (input.substr(0,5)=="type ") {
	    for (auto str : builtins) {
		if (str == input.substr(5)) {std::cout << input.substr(5) << " is a shell builtin" << std::endl; flag=1;}
	    }
	    if (!flag) std::cout << input.substr(5) << ": not found" << std::endl;
	}
	else std::cout << input << ": command not found" << std::endl;
	std::cout << "$ ";
    }
}

