#include <gtest/gtest.h>
#include "book.hpp"

TEST(EmptyBook, NoBestBidOrAsk) {
    Book book;
    EXPECT_FALSE(book.best_bid());
    EXPECT_FALSE(book.best_ask());
}

TEST(OrderPrice, MatchingBestBid) {
    Book book;
    Order order{123, 1, LIMIT, BUY, 100, 5, 123456789};
    book.add(order);

    EXPECT_EQ(book.best_bid(), 100);
    EXPECT_FALSE(book.best_ask());
}

TEST(OrderPrice, MatchingBestAsk) {
    Book book;
    Order order{123, 1, LIMIT, SELL, 100, 5, 123456789};
    book.add(order);

    EXPECT_EQ(book.best_ask(), 100);
    EXPECT_FALSE(book.best_bid());
}

TEST(CanceledBuy, RemovedFromBook) {
    Book book;
    Order order{123, 1, LIMIT, BUY, 100, 5, 123456789};
    book.add(order);
    book.cancel(order.getOrderId());

    EXPECT_FALSE(book.best_bid());
    EXPECT_FALSE(book.best_ask());
}

TEST(CanceledSell, RemovedFromBook) {
    Book book;
    Order order{123, 1, LIMIT, SELL, 100, 5, 123456789};
    book.add(order);
    book.cancel(order.getOrderId());

    EXPECT_FALSE(book.best_bid());
    EXPECT_FALSE(book.best_ask());
}


TEST(MatchingLimitOrder, MatchesAndFills) {
    Book book;
    Order sell_order{123, 1, LIMIT, SELL, 100, 5, 123456789};
    book.add(sell_order);

    Order buy_order{124, 2, LIMIT, BUY, 100, 5, 123456790};
    std::vector Trades = book.add(buy_order);

    EXPECT_EQ(size(Trades), 1);
    EXPECT_EQ((*Trades.begin()).getPrice(), 100);
    EXPECT_FALSE(book.best_bid());
    EXPECT_FALSE(book.best_ask());
}

TEST(RestingPartiallyFilled, PartialMatchAndFill) {
    Book book;
    Order sell_order{123, 1, LIMIT, SELL, 100, 5, 123456789};
    book.add(sell_order);

    Order buy_order{124, 2, LIMIT, BUY, 100, 3, 123456790};
    std::vector Trades = book.add(buy_order);

    EXPECT_EQ(size(Trades), 1);
    EXPECT_EQ((*Trades.begin()).getPrice(), 100);
    EXPECT_EQ((*Trades.begin()).getQuantity(), 3);
    EXPECT_FALSE(book.best_bid());
    EXPECT_EQ(book.best_ask(), 100);
}

TEST(NewPartiallyFilled, PartialMatchAndFill) {
    Book book;
    Order sell_order{123, 1, LIMIT, SELL, 100, 5, 123456789};
    book.add(sell_order);

    Order buy_order{124, 2, LIMIT, BUY, 100, 8, 123456790};
    std::vector Trades = book.add(buy_order);

    EXPECT_EQ(size(Trades), 1);
    EXPECT_EQ((*Trades.begin()).getPrice(), 100);
    EXPECT_EQ((*Trades.begin()).getQuantity(), 5);
    EXPECT_EQ(book.best_bid(), 100);
    EXPECT_FALSE(book.best_ask());
}

TEST(MultipleRestingOrders, FIFOobserved) {
    Book book;
    Order sell_order_1{123, 1, LIMIT, SELL, 100, 5, 123456789};
    book.add(sell_order_1);
    Order sell_order_2{124, 2, LIMIT, SELL, 100, 3, 123456790};
    book.add(sell_order_2);

    Order buy_order{125, 3, LIMIT, BUY, 100, 5, 123456791};
    std::vector Trades = book.add(buy_order);

    EXPECT_EQ(size(Trades), 1);
    EXPECT_EQ((*Trades.begin()).getRestingId(), 123);
    EXPECT_EQ(book.best_ask(), 100);
}

TEST(SelfTradeAttempt, SelfTradingBlocked) {
    Book book; 
    Order sell_order{123, 1, LIMIT, SELL, 100, 5, 123456789};
    book.add(sell_order);
    Order buy_order{124, 1, LIMIT, BUY, 100, 5, 123456790};
    std::vector<Trade> Trades = book.add(buy_order);

    EXPECT_EQ(size(Trades), 0);
    EXPECT_EQ(book.best_ask(), 100);
    EXPECT_EQ(book.best_bid(), 100);
}

TEST(PartialIOC, IOCobserved) {
    Book book; 
    Order sell_order{123, 1, LIMIT, SELL, 100, 3, 123456789};
    book.add(sell_order);

    Order buy_order{124, 2, IOC, BUY, 100, 8, 123456790};
    std::vector<Trade> Trades = book.add(buy_order);

    EXPECT_EQ(size(Trades), 1);
    EXPECT_EQ(Trades[0].getQuantity(), 3);
    EXPECT_FALSE(book.best_bid());
    EXPECT_FALSE(book.best_ask());
}

TEST(PartialMarket, MarketObserved) {
    Book book; 
    Order sell_order{123, 1, LIMIT, SELL, 100, 3, 123456789};
    book.add(sell_order);

    Order buy_order{124, 2, MARKET, BUY, 100, 8, 123456790};
    std::vector<Trade> Trades = book.add(buy_order);

    EXPECT_EQ(size(Trades), 1);
    EXPECT_EQ(Trades[0].getQuantity(), 3);
    EXPECT_FALSE(book.best_bid());
    EXPECT_FALSE(book.best_ask());
}

TEST(LimitOrder, MultiLevelWalk) {
    Book book; 
    Order sell_order_1{123, 1, LIMIT, SELL, 100, 2, 123456789};
    book.add(sell_order_1);
    Order sell_order_2{124, 2, LIMIT, SELL, 101, 2, 123456790};
    book.add(sell_order_2);
    Order buy_order{125, 3, LIMIT, BUY, 101, 4, 123456791};
    std::vector<Trade> Trades = book.add(buy_order);

    EXPECT_EQ(size(Trades), 2);
    EXPECT_FALSE(book.best_bid());
    EXPECT_FALSE(book.best_ask());
}

TEST(ReducedOrder, OrderModified) {
    Book book;
    Order buy_order_1{123, 1, LIMIT, BUY, 100, 5, 123456789};
    book.add(buy_order_1);
    Order buy_order_2{123, 1, LIMIT, BUY, 100, 2, 123456790};
    book.modify(buy_order_2);

    EXPECT_EQ(book.best_bid(), 100); 
    EXPECT_FALSE(book.best_ask());   
}

TEST(ZeroedOrder, OrderCancelled) {
    Book book;
    Order buy_order_1{123, 1, LIMIT, BUY, 100, 5, 123456789};
    book.add(buy_order_1);
    Order buy_order_2{123, 1, LIMIT, BUY, 100, 0, 123456790};
    book.modify(buy_order_2);

    EXPECT_FALSE(book.best_bid()); 
    EXPECT_FALSE(book.best_ask());  
}

TEST(ChangedPriceOrder, OrderModified) {
    Book book;
    Order buy_order_1{123, 1, LIMIT, BUY, 100, 5, 123456789};
    book.add(buy_order_1);
    Order buy_order_2{123, 1, LIMIT, BUY, 99, 5, 123456790};
    book.modify(buy_order_2);
    
    EXPECT_EQ(book.best_bid(), 99); 
    EXPECT_FALSE(book.best_ask());
}

TEST(CancelBeforeAdd, EmptyBookNoCrash) {
    Book book;
    Order order{123, 1, LIMIT, BUY, 100, 5, 123456789};
    book.cancel(order.getOrderId());

    EXPECT_FALSE(book.best_bid());
    EXPECT_FALSE(book.best_ask());
}