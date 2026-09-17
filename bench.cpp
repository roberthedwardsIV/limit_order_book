#include <iostream>
#include <chrono>

#include "book.hpp"

int main() {
    Book book;

    auto start = std::chrono::steady_clock::now();

    for(int i = 0; i<100000; i++) {
        Order order{i, 1, LIMIT, BUY, 100, 5, 123456789};
        book.add(order);
        book.cancel(order.getOrderId());
    }

    auto end = std::chrono::steady_clock::now();
    auto elapsed = end - start;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed);
    float ops_per_sec = 200000.0 / (ms.count() / 1000.0);


    std::cout << "Time Elapsed (total): " << ms.count() << "ms / " << ns.count() << "ns\n";
    std::cout << "Operations/Second: " << ops_per_sec << "\n";
    
    return 0;
}
