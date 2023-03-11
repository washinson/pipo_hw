#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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

//    std::mutex mergeMutex;
    auto shm_launch_mutex_id = shmget(IPC_PRIVATE, sizeof(pthread_mutex_t), 0600);
    if (shm_launch_mutex_id < 0) {
        perror("shmget");
        return 1;
    }
    auto *mutex = (pthread_mutex_t *) shmat(shm_launch_mutex_id, NULL, 0);
    if (mutex == (void *) -1) {
        perror("shmat");
        return 1;
    }
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    auto t = pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    if (t != 0) {
        perror("pthread_mutexattr_setpshared");
        return 1;
    }
    pthread_mutex_init(mutex, &mattr);

    auto resultMem = shmget(IPC_PRIVATE, sizeof(int64_t), 0600);
    auto *result = (int64_t *) shmat(shm_launch_mutex_id, NULL, 0);
    if (result == (void *) -1) {
        perror("result");
        return 1;
    }
    *result = 1;
    std::vector<pid_t> processes;

    for (int i = 0; i < std::min(totalThreads, n); ++i) {

        processes.push_back(fork());

        if (processes.back() < 0) {
            perror("fork error");
            return 1;
        } else if (processes.back() == 0) {
            int64_t res = 1;
            for (int j = i; j < n; j += totalThreads) {
                res *= j + 1;
            }

            pthread_mutex_lock(mutex);
            *result *= res;
            pthread_mutex_unlock(mutex);
            return 0;
        }
    }

    for (auto process : processes) {
        while(waitpid(process, NULL, WNOHANG) != process);
    }

    std::cout << *result << std::endl;

    pthread_mutex_destroy(mutex);
    shmdt(mutex);
    shmdt(result);
    if (shmctl(shm_launch_mutex_id, IPC_RMID, NULL) != 0) {
        perror("shmctl");
        return 1;
    }
    if (shmctl(resultMem, IPC_RMID, NULL) != 0) {
        perror("shmctl");
        return 1;
    }
    return 0;
}
