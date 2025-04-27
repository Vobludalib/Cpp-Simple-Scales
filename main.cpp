#include <fstream>
#include <iostream>

// #define GERMAN_NAMING

#include "musiclibrary.hpp"

int main()
{
    std::ifstream file;
    file.open("./scales.txt");

    if (!file.good())
    {
        throw std::exception();
    }

    Scale scale;
    while (file)
    {
        file >> scale;
        std::cout << scale << std::endl;
        auto realised_scale = RealisedScale{Note{"C4"}, scale};
        std::cout << realised_scale << std::endl;
    }

    file.close();
}