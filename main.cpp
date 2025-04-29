#include <fstream>
#include <iostream>

// #define GERMAN_NAMING

#include "applicationmanager.hpp"
#include "musiclibrary.hpp"
#include "scalemanager.hpp"

int main(int argc, char* argv[])
{
    ApplicationManager am;
    am.load_scales("./scales.csv");
    am.generate_session((size_t)2, ScaleManager::Difficulty::HARD);
    while (am.can_print_more())
    {
        am.print_header(std::cout);
        am.print_question(std::cout);
        am.load_answer(std::cin);
        am.next_question();
        am.clear_stream(std::cout);
    }

    std::cout << "You got " << am.get_success_percentage() << "% correct!" << std::endl;
}