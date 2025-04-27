#include <fstream>
#include <iostream>

// #define GERMAN_NAMING

#include "musiclibrary.hpp"
#include "scalemanager.hpp"

int main()
{
    ScaleManager sm;
    sm.load_scales_from_file("./scales.csv");
    auto&& scale_entry = sm.get_random_scales(1);
    std::cout << scale_entry[0]->get_name() << std::endl;
}