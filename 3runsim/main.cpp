#include <iostream>
#include <thread>
#include <vector>

int main(int argc, char *argv[]) {
    int totalThreads;
    if (argc < 2) {
        perror("please specify threads count");
        return 1;
    } else {
        totalThreads = std::stoi(argv[1]);
        if (totalThreads < 1) {
            std::cout << "too small threads count";
            return 2;
        }
    }

    std::atomic<int> currentProcesses = 0;

    std::string command;
    while (std::getline(std::cin, command)) {
        std::cout << command << std::endl;

        if (currentProcesses == totalThreads) {
            std::cout << "all threads are unavailable" << std::endl;
            continue;
        }

        ++currentProcesses;
        try {
            std::thread([&currentProcesses, &command]() {
                system(command.c_str());
                --currentProcesses;
            }).detach();
        } catch (std::system_error &e) {
            --currentProcesses;
        }
    }
    return 0;
}
