# Limit order book

C++17 price-level book: rest, cancel, modify, and match with price-time priority. Built as a data-structure project, not a trading strategy and not a live venue.

Prices are integer ticks. One book is one instrument.

## How it is put together

Each price holds a `std::list` of orders (FIFO). A hash map from order id to a list iterator is how cancel and modify find a live order in constant time after the lookup. Bids sort high to low so the first bid is the best bid; asks sort low to high.

Incoming limits that do not cross rest at the back of their level. Incoming orders that cross walk the opposite side from the touch, oldest first. Trades print at the resting price. A limit leftover after matching sits on the book. Market and IOC leftovers are discarded.

Self-trade is prevented: an aggressor does not fill its own resting orders and will walk to the next price if that price still crosses. Modify shrinks quantity in place (time priority kept). A size-up or a price change is cancel-then-add, so priority is lost.

## Order types

| Type | Behavior |
|------|----------|
| Limit | Cross if the price is through the touch; leftover rests |
| Market | Cross any opposite quote; leftover is cancelled |
| IOC | Cross only at its limit or better; leftover is cancelled |

`add`, `match`, and `modify` return the trades they produce (empty if none).

## Build and test

Needs CMake 3.16+, a C++17 compiler, and network the first time (googletest is fetched).

```
cmake -S . -B build
cmake --build build --target book_tests
./build/book_tests
```

CI on GitHub Actions runs that same path on Ubuntu.

## Tests

gtest covers empty BBO, rest and cancel, a full cross, both partial-fill shapes, FIFO at one price, a two-level walk, self-trade (no fill, both sides remain), IOC and market leftovers, modify (shrink, zero, price change), and cancel of an unknown id.

There is no benchmark number in this repo yet. When there is, it will say how it was measured. Nothing here is exchange RTT or “HFT latency.”

## Limitations

- Single instrument, in process. No tape, no sockets, no persistence.
- No stop, GTC/GTD flags beyond limit / market / IOC, no lots or tick-size table — you pass ticks yourself.
- One owner id is the whole self-trade key. No accounts or clearing.
- Matching is correctness-first. Allocations and cache layout are not tuned.
- `main` is a stub. The test binary is how you run the book.
