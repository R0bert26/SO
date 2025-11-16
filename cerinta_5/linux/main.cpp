#include <iostream>
#include <unistd.h>
#include <vector>
#include <cstring>

bool isPrime(int n) {
    if (n < 2) return false;

    for (int i = 2; i * i <= n; i++)
        if (n % i == 0) return false;

    return true;
}

int main() {
    const int NUM_PROCESSES = 10;
    const int INTERVAL = 1000;

    int pc[NUM_PROCESSES][2];
    int cp[NUM_PROCESSES][2];

    for (int i = 0; i < NUM_PROCESSES; i++) {
        pipe(pc[i]);
        pipe(cp[i]);

        pid_t pid = fork();

        if (pid == 0) {
            close(pc[i][1]);
            close(cp[i][0]);

            int range[2];
            read(pc[i][0], range, sizeof(range));

            int start = range[0];
            int end = range[1];

            std::vector<int> primes;
            for (int n = start; n <= end; n++)
                if (isPrime(n)) primes.push_back(n);

            write(cp[i][1], primes.data(), primes.size() * sizeof(int));

            close(pc[i][0]);
            close(cp[i][1]);
            return 0;
        }
        else {
            close(pc[i][0]);
            close(cp[i][1]);

            int start = i * INTERVAL + 1;
            int end = start + INTERVAL - 1;

            int range[2] = { start, end };
            write(pc[i][1], range, sizeof(range));

        }
    }

    for (int i = 0; i < NUM_PROCESSES; i++) {
        int buffer[1000];
        int bytes = read(cp[i][0], buffer, sizeof(buffer));

        int count = bytes / sizeof(int);

        for (int k = 0; k < count; k++)
            std::cout << buffer[k] << " ";

        close(pc[i][1]);
        close(cp[i][0]);
    }

    std::cout << "\n";
    return 0;
}
