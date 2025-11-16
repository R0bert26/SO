#include <windows.h>
#include <iostream>
#include <string>
using namespace std;

int main() {
    const int NUM_PROCESSES = 10;
    const int INTERVAL = 1000;

    for (int i = 0; i < NUM_PROCESSES; i++) {
        HANDLE inRead, inWrite;
        HANDLE outRead, outWrite;

        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

        CreatePipe(&inRead, &inWrite, &sa, 0);
        CreatePipe(&outRead, &outWrite, &sa, 0);

        STARTUPINFO si = { 0 };
        si.cb = sizeof(si);
        si.hStdInput = inRead;
        si.hStdOutput = outWrite;
        si.dwFlags = STARTF_USESTDHANDLES;

        PROCESS_INFORMATION pi;

        CreateProcess(
            "child.exe",
            NULL, NULL, NULL, TRUE, 0,
            NULL, NULL, &si, &pi
        );

        CloseHandle(inRead);
        CloseHandle(outWrite);

        int start = i * INTERVAL + 1;
        int end = start + INTERVAL - 1;

        string msg = to_string(start) + " " + to_string(end) + "\n";
        DWORD written;
        WriteFile(inWrite, msg.c_str(), msg.size(), &written, NULL);
        CloseHandle(inWrite);

        char buffer[4096];
        DWORD read;
        ReadFile(outRead, buffer, sizeof(buffer), &read, NULL);
        buffer[read] = '\0';

        cout << buffer;

        CloseHandle(outRead);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return 0;
}
