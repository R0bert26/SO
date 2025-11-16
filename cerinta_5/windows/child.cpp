#include <iostream>
#include <vector>
using namespace std;

bool isPrime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; i++)
        if (n % i == 0) return false;
    return true;
}

int main() {
    int start, end;
    cin >> start >> end;

    for (int n = start; n <= end; n++)
        if (isPrime(n))
            cout << n << " ";

    return 0;
}
