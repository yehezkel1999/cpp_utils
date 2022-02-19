
#include <iostream>
#include <queue>
#include "stl/queue/ThreadSafeQueue.h"

int main(int argc, char **argv) {
    std::cout << sizeof(bool) << std::endl;
    std::queue<int> q;
    q.push(1);
    q.pop();

    utils::safe_queue<int> sq;
    auto pair1 = std::make_pair(1,1);
    return 0;
}