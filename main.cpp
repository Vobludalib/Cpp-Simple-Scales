#include <fstream>
#include <iostream>

#include "applicationmanager.hpp"
#include "argparse.hpp"
#include "musiclibrary.hpp"
#include "scalemanager.hpp"

struct MyArgs : public argparse::Args
{
    size_t& number_of_questions = kwarg("n", "Number of questions in this session").set_default(5);
    std::string& input_path = kwarg("i", "Path to the scales files").set_default("./scales.csv");
    std::string& output_path =
        kwarg("o", "Path to the output .csv file").set_default("./results.csv");
    size_t& difficulty =
        kwarg("d", "Question difficulty (0 = Easy, 1 = Medium, 2 = Hard)").set_default(1);
};

int main(int argc, char* argv[])
{
    // Using existing argparse library for command-line arguments. I really don't want to write
    // another parser like in the homeworks
    auto args = argparse::parse<MyArgs>(argc, argv);

    ApplicationManager am;
    am.load_scales(args.input_path);
    am.generate_session(args.number_of_questions,
                        (ScaleManager::Difficulty)(args.difficulty > 2 ? 2 : args.difficulty));

    while (am.can_print_more())
    {
        am.print_header(std::cout);
        am.print_question(std::cout);
        am.load_answer(std::cin);
        am.next_question();
        am.clear_stream(std::cout);
    }

    am.save_session_results(args.output_path);
}