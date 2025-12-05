#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <random>

// Definirea culorilor
enum Color { NONE, WHITE, BLACK };

class ResourceController {
private:
    std::mutex mtx;
    std::condition_variable cv_white;
    std::condition_variable cv_black;

    Color current_color = NONE; // Cine deține resursa acum
    int active_count = 0;       // Câte fire sunt înăuntru
    int waiting_white = 0;      // Câți albi așteaptă
    int waiting_black = 0;      // Câți negri așteaptă

public:
    // ---------------- LOGICA PENTRU FIRELE ALBE ----------------
    void enter_white(int id) {
        std::unique_lock<std::mutex> lock(mtx);
        waiting_white++;

        // Așteaptă dacă:
        // 1. Culoarea curentă este NEAGRĂ (ocupat de inamic)
        // 2. SAU culoarea este ALBĂ, dar sunt NEGRI care așteaptă (PREVENIRE STARVATION)
        while (current_color == BLACK || (current_color == WHITE && waiting_black > 0)) {
            cv_white.wait(lock);
        }

        waiting_white--;
        current_color = WHITE;
        active_count++;

        // Log pentru vizualizare
        std::cout << "[WHITE " << id << "] a intrat. Activi: " << active_count << "\n";
    }

    void exit_white(int id) {
        std::unique_lock<std::mutex> lock(mtx);
        active_count--;

        std::cout << "[WHITE " << id << "] a iesit.  Activi: " << active_count << "\n";

        if (active_count == 0) {
            current_color = NONE;
            // Prioritizăm cealaltă culoare pentru a preveni înfometarea
            if (waiting_black > 0) {
                cv_black.notify_all(); // Trezim toți negrii
            } else {
                cv_white.notify_all(); // Dacă nu sunt negri, pot intra alți albi
            }
        }
    }

    // ---------------- LOGICA PENTRU FIRELE NEGRE ----------------
    void enter_black(int id) {
        std::unique_lock<std::mutex> lock(mtx);
        waiting_black++;

        // Așteaptă dacă:
        // 1. Culoarea curentă este ALBĂ
        // 2. SAU culoarea este NEAGRĂ, dar sunt ALBI care așteaptă (PREVENIRE STARVATION)
        while (current_color == WHITE || (current_color == BLACK && waiting_white > 0)) {
            cv_black.wait(lock);
        }

        waiting_black--;
        current_color = BLACK;
        active_count++;

        std::cout << "[BLACK " << id << "] a intrat. Activi: " << active_count << "\n";
    }

    void exit_black(int id) {
        std::unique_lock<std::mutex> lock(mtx);
        active_count--;

        std::cout << "[BLACK " << id << "] a iesit.  Activi: " << active_count << "\n";

        if (active_count == 0) {
            current_color = NONE;
            // Prioritizăm cealaltă culoare
            if (waiting_white > 0) {
                cv_white.notify_all(); // Trezim toți albii
            } else {
                cv_black.notify_all(); // Dacă nu sunt albi, pot intra alți negri
            }
        }
    }
};

// Resursa partajată și controllerul ei
ResourceController controller;

// Simulare muncă
void thread_work(int id, Color my_color) {
    // Simulare timp aleatoriu înainte de a cere acces
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 100));

    if (my_color == WHITE) {
        controller.enter_white(id);
        // Secțiunea critică (folosirea resursei)
        std::this_thread::sleep_for(std::chrono::milliseconds(50 + rand() % 100));
        controller.exit_white(id);
    } else {
        controller.enter_black(id);
        // Secțiunea critică (folosirea resursei)
        std::this_thread::sleep_for(std::chrono::milliseconds(50 + rand() % 100));
        controller.exit_black(id);
    }
}

int main() {
    std::vector<std::thread> threads;
    int num_threads = 20;

    // Lansăm fire amestecate
    for (int i = 0; i < num_threads; ++i) {
        Color c = (rand() % 2 == 0) ? WHITE : BLACK;
        threads.push_back(std::thread(thread_work, i, c));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Toate firele au terminat executia.\n";
    return 0;
}
