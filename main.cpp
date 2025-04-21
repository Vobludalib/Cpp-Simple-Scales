#include <iostream>

#include "musiclibrary.hpp"

int main()
{
    auto root = Note("Bb4");
    std::cout << "Root: " << root << std::endl;
    auto sd = Note(root, 11, 0);
    std::cout << sd << std::endl;
    // for (size_t i = 1; i < 16; ++i)
    // {
    //     auto sd = Note(root, i, 0);
    //     std::cout << i << ": " << sd << std::endl;
    // }
}