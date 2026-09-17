#include <map>
#include <unordered_map>
#include <list>
#include <optional>
#include <vector>
#include <iostream>

enum Side {
    BUY,
    SELL
};

enum OrderType {
    LIMIT,
    MARKET,
    IOC
};


class Order {
    int order_id;
    int owner_id;
    OrderType type;
    Side side;
    int price;
    int quantity;
    long int timestamp;
    public:
        Order(int mId, int oId, OrderType mType, Side mSide, int mPrice, int mQuantity, long int mTimestamp) : order_id(mId), owner_id(oId), type(mType), side(mSide), price(mPrice), quantity(mQuantity), timestamp(mTimestamp) {}

        int getOrderId() { return order_id; }
        int getOwnerId() { return owner_id; }
        OrderType getOrderType() { return type; }
        Side getOrderSide() { return side; }
        int getOrderPrice() { return price; }
        int getOrderQuantity() { return quantity; }
        long int getOrderTime() { return timestamp; } 

        void setOrderQuantity(int mQuantity) { quantity = mQuantity; return;}
};

class Trade {
    int aggressor_id; 
    int resting_id; 
    int price;
    int quantity;
    public:
        Trade(int mAggressor_id, int mResting_id, int mPrice, int mQuantity) : aggressor_id(mAggressor_id), resting_id(mResting_id), price(mPrice), quantity(mQuantity) {}
        int getAggressorId() { return aggressor_id; }
        int getRestingId() { return resting_id; }
        int getPrice() { return price; }
        int getQuantity() { return quantity; }
};

class PriceLevel {
    std::list<Order> level;
    int total_quantity = 0;
    public:
        std::list<Order>::iterator addOrder(Order new_order) {
            total_quantity += new_order.getOrderQuantity();
            std::list<Order>::iterator it = level.insert(level.end(), new_order);
            return it;
        }

        void cancelOrder(std::list<Order>::iterator it) {
            total_quantity -= (*it).getOrderQuantity();
            level.erase(it);
        }

        bool isLevelEmpty() {
            return level.empty();
        }
        std::list<Order>::iterator firstOrderOut() {
            return level.begin();
        } 

        int getLevelTotalQuantity() { return total_quantity; }
        void setLevelTotalQuantity(int mQuantity) { total_quantity = mQuantity; return; }

        std::list<Order>::iterator getLevelEndIt() {
            return level.end();
        }
};

struct BuyComparator {
    bool operator()(const int& left_price, const int& right_price) const {
        return left_price > right_price;
    }
};



class Book {
    std::map<int, PriceLevel, BuyComparator> buy_side;
    std::map<int, PriceLevel> sell_side;

    struct OrderLoc {
        std::list<Order>::iterator it;
        Side side;
        int price;
    };
    
    std::unordered_map<int, OrderLoc> order_id_hash_map;

    public:
        

        std::vector<Trade> add(Order new_order) {
            std::vector<Trade> Trades;

            if(new_order.getOrderQuantity() <= 0) { return Trades; }

            std::unordered_map<int, OrderLoc>::iterator matched_it = order_id_hash_map.find(new_order.getOrderId());
            if(matched_it != order_id_hash_map.end()) { return Trades; }

            if(doesOrderCross(new_order)) { 
                std::vector<Trade> crossed_trades = match(new_order);
                Trades.insert(Trades.end(), crossed_trades.begin(), crossed_trades.end());
                return Trades; }
            if(new_order.getOrderSide() == BUY) {
                std::list<Order>::iterator it = buy_side[new_order.getOrderPrice()].addOrder(new_order);
                OrderLoc location{it, BUY, new_order.getOrderPrice()};
                order_id_hash_map.insert({new_order.getOrderId(), location});
                return Trades;
            }
            else if(new_order.getOrderSide() == SELL) {
                std::list<Order>::iterator it = sell_side[new_order.getOrderPrice()].addOrder(new_order);
                OrderLoc location{it, SELL, new_order.getOrderPrice()};
                order_id_hash_map.insert({new_order.getOrderId(), location});
                return Trades;
            }
            return Trades;
        }

        void cancel(int order_id) {
            std::unordered_map<int, OrderLoc>::iterator it = order_id_hash_map.find(order_id);
            if(it == order_id_hash_map.end()) { return; }
            OrderLoc location = it->second;

            if(location.side == BUY) {
                std::map<int, PriceLevel, BuyComparator>::iterator level_it = buy_side.find(location.price);
                if(level_it == buy_side.end()) { 
                    order_id_hash_map.erase(it);
                    return; }
                level_it->second.cancelOrder(location.it);
                if(level_it->second.isLevelEmpty()) { buy_side.erase(level_it); }
            }
            else if(location.side == SELL) {
                std::map<int, PriceLevel>::iterator level_it = sell_side.find(location.price);
                if(level_it == sell_side.end()) { 
                    order_id_hash_map.erase(it);
                    return; }
                level_it->second.cancelOrder(location.it);
                if(level_it->second.isLevelEmpty()) { sell_side.erase(level_it); }
            }
            else { return; }

            order_id_hash_map.erase(it);
            return;
        }

        std::optional<int> best_bid() {
            if(buy_side.empty()) { return std::nullopt; }
            else {
            std::map<int, PriceLevel, BuyComparator>::iterator it = buy_side.begin();
            return it->first;
            }
        }

        std::optional<int> best_ask() {
            if(sell_side.empty()) { return std::nullopt; }
            else {
            std::map<int, PriceLevel>::iterator it = sell_side.begin();
            return it->first;
            }
        }

        bool doesOrderCross(Order order) {
            if(order.getOrderSide() == BUY) {
                std::optional<int> b_ask = best_ask();
                if(b_ask && order.getOrderType() == MARKET) { return true; }
                if(b_ask && order.getOrderPrice() >= b_ask) { return true; }
                else { return false; }
            }
            else if(order.getOrderSide() == SELL) {
                std::optional<int> b_bid = best_bid();
                if(b_bid && order.getOrderType() == MARKET) { return true; }
                if(b_bid && order.getOrderPrice() <= b_bid) { return true; }
                else { return false; }
            }
            return false;
        }

        std::vector<Trade> match(Order new_order) { 
            std::vector<Trade> Trades;
            std::map<int, PriceLevel, BuyComparator>::iterator buy_side_it = buy_side.begin();
            std::map<int, PriceLevel>::iterator sell_side_it = sell_side.begin();


            
            while(new_order.getOrderQuantity() > 0 && doesOrderCross(new_order)) {
                if(new_order.getOrderSide() == BUY) {
                    PriceLevel& level = sell_side_it->second;
                    std::list<Order>::iterator first_ask_to_fill = level.firstOrderOut();
                    while(first_ask_to_fill != level.getLevelEndIt() && new_order.getOwnerId() == (*first_ask_to_fill).getOwnerId()) {
                        first_ask_to_fill++;
                    }
                    if(first_ask_to_fill == level.getLevelEndIt()) { 
                        sell_side_it++;
                        if(sell_side_it == sell_side.end()) { break; }
                        else { 
                            if(new_order.getOrderType() != MARKET && new_order.getOrderPrice() < sell_side_it->first) {
                                return Trades;
                            }
                            continue; }
                    }
                    int fill_size = new_order.getOrderQuantity() > (*first_ask_to_fill).getOrderQuantity() ? (*first_ask_to_fill).getOrderQuantity() : new_order.getOrderQuantity();
                    
                    Trade trade{new_order.getOrderId(), (*first_ask_to_fill).getOrderId(), (*first_ask_to_fill).getOrderPrice(), fill_size};
                    Trades.push_back(trade);

                    int quant_remain_in_new = new_order.getOrderQuantity() - (*first_ask_to_fill).getOrderQuantity();

                    if(quant_remain_in_new == 0) { 
                        int resting_order_id = (*first_ask_to_fill).getOrderId();
                        level.cancelOrder(first_ask_to_fill);
                        std::unordered_map<int, OrderLoc>::iterator it = order_id_hash_map.find(resting_order_id);
                        order_id_hash_map.erase(it);
                        if(level.isLevelEmpty()) { sell_side_it = sell_side.erase(sell_side_it); }
                        new_order.setOrderQuantity(0);

                    }

                    if(quant_remain_in_new < 0) {
                        (*first_ask_to_fill).setOrderQuantity(-quant_remain_in_new);
                        level.setLevelTotalQuantity(level.getLevelTotalQuantity() - fill_size);
                        new_order.setOrderQuantity(0);
                    }

                    if(quant_remain_in_new > 0) {
                        new_order.setOrderQuantity(quant_remain_in_new);
                        int resting_order_id = (*first_ask_to_fill).getOrderId();
                        level.cancelOrder(first_ask_to_fill);
                        std::unordered_map<int, OrderLoc>::iterator it = order_id_hash_map.find(resting_order_id);
                        order_id_hash_map.erase(it);
                        if(level.isLevelEmpty()) { sell_side_it = sell_side.erase(sell_side_it); }
                        new_order.setOrderQuantity(quant_remain_in_new);
                    }


                    

                }
                else if(new_order.getOrderSide() == SELL) {
                    PriceLevel& level = buy_side_it->second;
                    std::list<Order>::iterator first_bid_to_fill = level.firstOrderOut();
                    while(first_bid_to_fill != level.getLevelEndIt() && new_order.getOwnerId() == (*first_bid_to_fill).getOwnerId()) {
                        first_bid_to_fill++;
                    }
                    if(first_bid_to_fill == level.getLevelEndIt()) { 
                        buy_side_it++;
                        if(buy_side_it == buy_side.end()) { break; }
                        else { 
                            if(new_order.getOrderType() != MARKET && new_order.getOrderPrice() > buy_side_it->first) {
                                return Trades;
                            }
                            continue; } 
                    }
                    int fill_size = new_order.getOrderQuantity() > (*first_bid_to_fill).getOrderQuantity() ? (*first_bid_to_fill).getOrderQuantity() : new_order.getOrderQuantity();
                    Trade trade{new_order.getOrderId(), (*first_bid_to_fill).getOrderId(), (*first_bid_to_fill).getOrderPrice(), fill_size};
                    Trades.push_back(trade);

                    int quant_remain_in_new = new_order.getOrderQuantity() - (*first_bid_to_fill).getOrderQuantity();
                    if(quant_remain_in_new == 0) {  
                        int resting_order_id = (*first_bid_to_fill).getOrderId();
                        level.cancelOrder(first_bid_to_fill);
                        std::unordered_map<int, OrderLoc>::iterator it = order_id_hash_map.find(resting_order_id);
                        order_id_hash_map.erase(it);
                        if(level.isLevelEmpty()) { buy_side_it = buy_side.erase(buy_side_it); }
                        new_order.setOrderQuantity(0);
                    }
                    if(quant_remain_in_new < 0) {
                        (*first_bid_to_fill).setOrderQuantity(-quant_remain_in_new);
                        level.setLevelTotalQuantity(level.getLevelTotalQuantity() - fill_size);
                        new_order.setOrderQuantity(0);
                    }

                    if(quant_remain_in_new > 0) {
                        new_order.setOrderQuantity(quant_remain_in_new);
                        int resting_order_id = (*first_bid_to_fill).getOrderId();
                        level.cancelOrder(first_bid_to_fill);
                        std::unordered_map<int, OrderLoc>::iterator it = order_id_hash_map.find(resting_order_id);
                        order_id_hash_map.erase(it);
                        if(level.isLevelEmpty()) { buy_side_it = buy_side.erase(buy_side_it); }
                        new_order.setOrderQuantity(quant_remain_in_new);
                    }
                }
                else { return Trades; }
            }


            if(new_order.getOrderQuantity() > 0 && new_order.getOrderType() == LIMIT) { 
                if(new_order.getOrderSide() == BUY) {
                    std::list<Order>::iterator it = buy_side[new_order.getOrderPrice()].addOrder(new_order);
                    OrderLoc location{it, new_order.getOrderSide(), new_order.getOrderPrice()};
                    order_id_hash_map.insert({new_order.getOrderId(), location});
                }
                else if(new_order.getOrderSide() == SELL) {
                    std::list<Order>::iterator it = sell_side[new_order.getOrderPrice()].addOrder(new_order);
                    OrderLoc location{it, new_order.getOrderSide(), new_order.getOrderPrice()};
                    order_id_hash_map.insert({new_order.getOrderId(), location});
                }
            }

            return Trades;
        }

        std::vector<Trade> modify(Order order) {
            std::vector<Trade> Trades;
            std::unordered_map<int, OrderLoc>::iterator it = order_id_hash_map.find(order.getOrderId());
            if(it == order_id_hash_map.end()) { return Trades; }

            if(order.getOrderQuantity() == 0) { 
                cancel(order.getOrderId());
                return Trades;
            }
             
            std::list<Order>::iterator resting_order_it = it->second.it;

            if(order.getOrderPrice() != (*resting_order_it).getOrderPrice()) {
                cancel((*resting_order_it).getOrderId());
                std::vector<Trade> added_trades = add(order);
                Trades.insert(Trades.end(), added_trades.begin(), added_trades.end());
                return Trades;
            }

            if(order.getOrderQuantity() < (*resting_order_it).getOrderQuantity()) {
                int quantity_dif = (*resting_order_it).getOrderQuantity() - order.getOrderQuantity();
                (*resting_order_it).setOrderQuantity(order.getOrderQuantity());
                if((*resting_order_it).getOrderSide() == BUY) {
                    PriceLevel& level = buy_side.find((*resting_order_it).getOrderPrice())->second;
                    level.setLevelTotalQuantity(level.getLevelTotalQuantity() - quantity_dif);
                    return Trades;
                }
                else if((*resting_order_it).getOrderSide() == SELL) {
                    PriceLevel& level = sell_side.find((*resting_order_it).getOrderPrice())->second;
                    level.setLevelTotalQuantity(level.getLevelTotalQuantity() - quantity_dif);
                    return Trades;
                }
            }
            else if(order.getOrderQuantity() > (*resting_order_it).getOrderQuantity()) {
                cancel((*resting_order_it).getOrderId());
                std::vector<Trade> added_trades = add(order);
                Trades.insert(Trades.end(), added_trades.begin(), added_trades.end());
                return Trades;
            }
            else if(order.getOrderQuantity() == (*resting_order_it).getOrderQuantity()) {
                return Trades;
            }
        }

        void print() {
            std::cout << "bids\n";
            for(std::map<int, PriceLevel, BuyComparator>::iterator it = buy_side.begin(); it != buy_side.end(); ++it) {
                std::cout << it->first << " " << it->second.getLevelTotalQuantity() << "\n";
            }
            std::cout << "asks\n";
            for(std::map<int, PriceLevel>::iterator it = sell_side.begin(); it != sell_side.end(); ++it) {
                std::cout << it->first << " " << it->second.getLevelTotalQuantity() << "\n";
            }
        }
    

};
