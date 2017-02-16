#include <iostream>
#include "bpTree.h"

int main(void){
    BpTree<int, int> t;
    for (int i = 0; i < 20; i++) {
        t.insert(i, i);
    }
    t.remove(3);
    t.remove(4);
    t.remove(2);
    t.remove(0);
    t.remove(1);
    t.remove(5);
    t.remove(11);
    t.print();

    return 0;
}
