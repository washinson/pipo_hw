#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <thread>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Print input file" << std::endl;
        return 1;
    }
    std::string filepath = argv[1];
    std::ifstream filestream(filepath);
    if (!filestream.is_open()) {
        std::cout << "cant open file" << std::endl;
        return 2;
    }
    std::map<int, std::vector<std::string>> delays;
    while (filestream) {
        int seconds;
        std::string program;
        filestream >> seconds >> program;
        delays[seconds].push_back(program);
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::vector<std::thread> threads;
    for (auto const &[seconds, programs]: delays) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::seconds{seconds} - (end - begin));
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        for (auto const &program : programs) {
            threads.emplace_back([now, begin, program](){
                std::cout << "run command: " << program << " in " << std::chrono::duration_cast<std::chrono::seconds>(now - begin).count() << "seconds" << std::endl;
                std::cout.flush();
                execl(program.c_str(), program.c_str(), (char *) nullptr);
            }).detach();
        }
    }
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    return 0;
}
