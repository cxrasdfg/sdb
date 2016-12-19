#include <iostream>
#include <unordered_set>
#include "lexer.hpp"

int main(int argc, char *argv[])
{
    std::unordered_set<int> us = {1,2,3,4,5};
    auto ab = us;
    std::cout << "hello world" << std::endl;
    return 0;
}
