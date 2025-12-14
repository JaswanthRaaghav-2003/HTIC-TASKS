
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

class Alternator {
public:
    Alternator() : turn_even(true) {}

    
    void print_even() {
        for (int i = 0; i <= 10; i += 2) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]{ return turn_even; }); 
            std::cout << i << '\n';
            turn_even = false;       
            lk.unlock();
            cv.notify_one();
        }
    }

   
    void print_odd() {
        for (int i = 1; i <= 9; i += 2) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]{ return !turn_even; }); 
            std::cout << i << '\n';
            turn_even = true;      
            lk.unlock();
            cv.notify_one();
        }
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    bool turn_even; 
};

int main() {
    Alternator alt;

    
    std::thread t_even(&Alternator::print_even, &alt);
    std::thread t_odd(&Alternator::print_odd, &alt);

   
    t_even.join();
    t_odd.join();

    return 0;
}
