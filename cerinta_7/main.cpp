#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <random>

enum Color { NONE, WHITE, BLACK };

class ResourceController {
private:
    std::mutex mtx;
    std::condition_variable cv_white;
    std::condition_variable cv_black;

    Color current_color = NONE; 
    int active_count = 0;      
    int waiting_white = 0;      
    int waiting_black = 0;      

public:
    void enter_white(int id) {
        std::unique_lock<std::mutex> lock(mtx);
        waiting_white++;

        while (current_color == BLACK || (current_color == WHITE && waiting_black > 0)) {
            cv_white.wait(lock);
        }

        waiting_white--;
        current_color = WHITE;
        active_count++;

        std::cout << "[WHITE " << id << "] a intrat. Activi: " << active_count << "\n";
    }

    void exit_white(int id) {
        std::unique_lock<std::mutex> lock(mtx);
        active_count--;

        std::cout << "[WHITE " << id << "] a iesit.  Activi: " << active_count << "\n";

        if (active_count == 0) {
            current_color = NONE;
            
            if (waiting_black > 0) {
                cv_black.notify_all(); 
            } else {
                cv_white.notify_all();
            }
        }
    }

    void enter_black(int id) {
        std::unique_lock<std::mutex> lock(mtx);
        waiting_black++;

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
            
            if (waiting_white > 0) {
                cv_white.notify_all(); 
            } else {
                cv_black.notify_all(); 
            }
        }
    }
};

ResourceController controller;

void thread_work(int id, Color my_color) {
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 100));

    if (my_color == WHITE) {
        controller.enter_white(id);
        std::this_thread::sleep_for(std::chrono::milliseconds(50 + rand() % 100));
        controller.exit_white(id);
    } else {
        controller.enter_black(id);
        std::this_thread::sleep_for(std::chrono::milliseconds(50 + rand() % 100));
        controller.exit_black(id);
    }
}

int main() {
    std::vector<std::thread> threads;
    int num_threads = 20;

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
