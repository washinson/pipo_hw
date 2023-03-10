#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int totalThreads;
    if (argc < 2) {
        totalThreads = (int) sysconf(_SC_NPROCESSORS_ONLN) + 1;
    } else {
        totalThreads = std::stoi(argv[1]);
        if (totalThreads < 1) {
            std::cout << "too small threads count";
            return 2;
        }
    }
    int n;
    std::cout << "input n" << std::endl;
    std::cin >> n;

    if (n < 0) {
        std::cout << "n must be more than zero";
        return 1;
    }

    std::mutex mergeMutex;
    int64_t result = 1;
    std::vector<std::thread> threads;

    for (int i = 0; i < std::min(totalThreads, n); ++i) {
        threads.emplace_back([&result, &mergeMutex, i, n, totalThreads]() {
            int64_t res = 1;
            for (int j = i; j < n; j += totalThreads) {
                res *= j + 1;
            }

            std::lock_guard<std::mutex> lock(mergeMutex);
            result *= res;
        });
    }

    for (std::thread &thread: threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    std::cout << result << std::endl;
    return 0;
}
