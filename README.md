# MPMC-Queue
Educational C++17 implementation of a thread-safe bounded MPMC queue using mutexes and condition variables. 
Things I picked up along the way:)
- RAII
- Concurrency and blocking/timed operations
- CMake "magic"

# Quick Start
## 1) Add the header
Place `MPMCQueue.h` in your project’s include path

## 2) Example usage
````
#include <iostream>
#include <thread>
#include <vector>
#include "MPMCQueue.h"

int main() {
    MPMCQueue<int> q{1024};

    // Producers
    std::thread p1([&]{
        for (int i = 0; i < 10000; ++i) q.push(i);      // blocks when full
    });
    std::thread p2([&]{
        for (int i = 10000; i < 20000; ++i) q.push(i);
    });

    // Consumers
    std::atomic<long long> sum{0};
    std::thread c1([&]{
        int v{};
        while (q.pop_for(v, std::chrono::milliseconds(50))) { // times out if no data & not closed
            sum += v;
        }
    });
    std::thread c2([&]{
        int v{};
        while (q.try_pop(v)) sum += v; // non-blocking variant
    });

    p1.join(); p2.join();
    c1.join(); c2.join();

    std::cout << "sum=" << sum.load() << "\n";
}

````
Take a look at `MPMCQueue_main.cpp` for more examples including:
- Usage with simple types
- Move-types with unique and shared ptrs
# Design Notes
- `std::deque` for O(1) and FIFO semantics
- Single mutex + two condition variables:
  - `cv_not_full` - producers wait when full; consumers will notify after each pop
  - `cv_not_empty` - consumers wait when empty; producers will notify after each push
- `close()` prevents adding more elements and notifies all threads to release them
