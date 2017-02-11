#include <iostream>
#include "bpTree.h"

int main(void){
    BpTree<int, int> t;
    t.insert(2, 3);
    t.insert(5, 3);
    t.insert(6, 3);
    t.insert(7, 3);
    t.insert(1, 3);
    t.insert(4, 3);
    t.insert(8, 3);
    t.insert(9, 3);
    t.insert(10, 3);
    t.insert(3, 3);
    t.print();

    auto x = t.find(3);
    std::cout << x << std::endl;
    return 0;
}
