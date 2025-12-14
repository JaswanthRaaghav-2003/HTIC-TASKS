

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

class Alternator {
public:
    Alternator() : turn_even(true) {}

    // Even numbers: 0,2,...,10
    void print_even() {
        for (int i = 0; i <= 10; i += 2) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]{ return turn_even; }); // wait until it's even's turn
            std::cout << i << '\n';
            turn_even = false;       // next: odd
            lk.unlock();
            cv.notify_one();
        }
    }

    // Odd numbers: 1,3,...,9
    void print_odd() {
        for (int i = 1; i <= 9; i += 2) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]{ return !turn_even; }); // wait until it's odd's turn
            std::cout << i << '\n';
            turn_even = true;        // next: even
            lk.unlock();
            cv.notify_one();
        }
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    bool turn_even; // true -> even thread's turn
};

int main() {
    Alternator alt;

    // start threads
    std::thread t_even(&Alternator::print_even, &alt);
    std::thread t_odd(&Alternator::print_odd, &alt);

    // wait for both to finish
    t_even.join();
    t_odd.join();

    return 0;
}
