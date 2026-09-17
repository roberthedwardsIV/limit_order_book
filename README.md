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

Default configure is Release if you did not pass a build type. Bench:

```
cmake --build build --target bench
./build/bench
```

Replay the committed sample (run this from the repo root so `sample_tape.txt` opens):

```
cmake --build build --target replay
./build/replay
```

It prints leftover bid/ask price and size. The file is invented events, not a venue capture.

CI on GitHub Actions runs the tests on Ubuntu, not the bench or replay.

## Tests

gtest covers empty BBO, rest and cancel, a full cross, both partial-fill shapes, FIFO at one price, a two-level walk, self-trade (no fill, both sides remain), IOC and market leftovers, modify (shrink, zero, price change), and cancel of an unknown id.

## Benchmark

`bench.cpp` times a tight loop: 100,000 resting limit buys at one tick, each cancelled immediately. That is 200,000 calls (add + cancel). Clock is `steady_clock` around the whole loop, not per call. One “op” is one `add` or one `cancel`.

On this laptop, Release, one run: 33 ms (about 6.1 million ops/s). Debug was ~160 ms on the same loop. Re-run `./build/bench` if you want your box’s number.

This is in-process book work on one thread. It is not a quote-to-ack time and not anything an exchange would print.

## What I would change

`std::map` plus `std::list` is the right first book: unknown price range, cancel in the middle of a level, tests you can read. It is not the fastest layout.

If ticks lived in a small known band, I would store levels in an array and keep a best-bid / best-ask cursor instead of walking a tree. If add/cancel rate was the bottleneck, I would allocate orders from a pool (reuse slots) so every rest is not a heap hit. I did not ship that second book. The map version is the one the tests and the bench run.

I would keep the map if the instrument can print any tick, or if I still needed to be able to explain every pointer in an interview.

## Limitations

- Single instrument, in process. No sockets, no persistence. Tape is `sample_tape.txt` (hand-written POC), not a live feed.
- No stop, GTC/GTD flags beyond limit / market / IOC, no lots or tick-size table — you pass ticks yourself.
- One owner id is the whole self-trade key. No accounts or clearing.
- Matching is correctness-first. Allocations and cache layout are not tuned.
- `main` is a stub. Tests, `bench`, and `replay` are how you run the book.
