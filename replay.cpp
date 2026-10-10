#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>


#include "book.hpp"


int main() {
    Book book;
    std::ifstream data;
    std::string line;
    data.open("sample_tape.txt");



    while (getline(data, line)) {
        if(line.empty()) { continue; }
        if(line[0] == '#') { continue; }
        std::stringstream ss(line);

        int order_id, owner_id, order_price, order_quantity;
        std::string action_string, type_string, side_string;
        OrderType order_type;
        Side order_side;

        if(!line.compare(0, 3, "ADD")) {
            ss >> action_string >> order_id >> owner_id >> type_string >> side_string >> order_price >> order_quantity;
            if(type_string == "MARKET") { order_type = MARKET; }
            else if(type_string == "LIMIT") { order_type = LIMIT; }
            else if(type_string == "IOC") { order_type = IOC; }

            if(side_string == "BUY") { order_side = BUY; }
            else if(side_string == "SELL") { order_side = SELL; }
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            long int timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

            Order order{order_id, owner_id, order_type, order_side, order_price, order_quantity, timestamp};
            book.add(order);
        }
        else if(!line.compare(0, 6, "CANCEL")) {
            ss >> action_string >> order_id;
            book.cancel(order_id);
        }
        else { continue; }


    }

    book.print();
    return 0;
}