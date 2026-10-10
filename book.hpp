#pragma once

#include <map>
#include <unordered_map>
#include <list>
#include <optional>
#include <vector>

enum Side { BUY, SELL };

enum OrderType { LIMIT, MARKET, IOC };

class Order {
    int order_id;
    int owner_id;
    OrderType type;
    Side side;
    int price;
    int quantity;
    long int timestamp;
    public:
        Order(int mId, int oId, OrderType mType, Side mSide, int mPrice, int mQuantity, long int mTimestamp) 
            : order_id(mId), owner_id(oId), type(mType), side(mSide), price(mPrice), 
            quantity(mQuantity), timestamp(mTimestamp) {}

        int getOrderId() const;
        int getOwnerId() const;
        OrderType getOrderType() const;
        Side getOrderSide() const;
        int getOrderPrice() const;
        int getOrderQuantity() const;
        long int getOrderTime() const;

        void setOrderQuantity(int mQuantity);
};

class Trade {
    int aggressor_id; 
    int resting_id; 
    int price;
    int quantity;
    public:
        Trade(int mAggressor_id, int mResting_id, int mPrice, int mQuantity) 
            : aggressor_id(mAggressor_id), resting_id(mResting_id), 
            price(mPrice), quantity(mQuantity) {}

        int getAggressorId() const;
        int getRestingId() const;
        int getPrice() const;
        int getQuantity() const;
};

class PriceLevel {
    std::list<Order> level;
    int total_quantity = 0;
    public:
        std::list<Order>::iterator addOrder(Order new_order);
        void cancelOrder(std::list<Order>::iterator it);
        std::list<Order>::iterator firstOrderOut();
        std::list<Order>::iterator getLevelEndIt();

        int getLevelTotalQuantity() const;
        bool isLevelEmpty() const;

        void setLevelTotalQuantity(int mQuantity);
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
        std::vector<Trade> add(Order new_order);
        void cancel(int order_id);
        std::vector<Trade> match(Order new_order);
        std::vector<Trade> modify(Order order);

        std::optional<int> best_bid() const;
        std::optional<int> best_ask() const;
        bool doesOrderCross(Order order) const;

        void print() const;
};