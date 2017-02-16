#include <iostream>
#include "bpTree.h"

int main(void){
    BpTree<int, int> t;
    for (int i = 0; i < 30; i++) {
        t.insert(i, i);
    }
    t.print();
    for (int i = 0; i < 10; ++i) {
        t.remove(i);
    }
    t.print();

    return 0;
}
