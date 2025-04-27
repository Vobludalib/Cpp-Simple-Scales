#include "scalemanager.hpp"

#include <algorithm>
#include <random>

void ScaleManager::handle_file(std::string path)
{
    std::ifstream file;
    file.open(path);
    if (!file.good())
    {
        throw std::runtime_error(BAD_FILE_OPEN);
    }

    parse_fstream(file);

    file.close();
}

void ScaleManager::parse_fstream(std::ifstream& stream)
{
    std::string line;
    std::string column_string;
    size_t column = 0;
    size_t row = 0;
    // Iterate over all lines
    while (std::getline(stream, line))
    {
        if (row == 0)
        {
            ++row;
            continue;
        }

        std::stringstream line_stream(line);

        // In each line, iterate over columns
        std::string name;
        ScaleManager::Difficulty difficulty;
        Scale scale;

        column = 0;
        while (std::getline(line_stream, column_string, CSV_SEPERATOR))
        {
            switch (column)
            {
                // Reading name
                case (0):
                {
                    name = column_string;
                    break;
                }
                case (1):
                {
                    if (column_string == "Easy")
                    {
                        difficulty = ScaleManager::Difficulty::EASY;
                    }
                    else if (column_string == "Medium")
                    {
                        difficulty = ScaleManager::Difficulty::MEDIUM;
                    }
                    else if (column_string == "Hard")
                    {
                        difficulty = ScaleManager::Difficulty::HARD;
                    }
                    else
                    {
                        throw std::runtime_error(INVALID_DIFFICULTY);
                    }
                    break;
                }
                case (2):
                {
                    scale.clear();
                    std::istringstream sstream(column_string);
                    sstream >> scale;
                    break;
                }
            }
            ++column;
        }

        _entries.emplace_back(std::move(scale), std::move(difficulty), std::move(name));
        ++row;
    }
}

void ScaleManager::build_maps()
{
    for (auto&& entry : _entries)
    {
        _difficulty_map.emplace(entry.get_difficulty(), &entry);
    }
}

void ScaleManager::load_scales_from_file(std::string path)
{
    handle_file(path);
    build_maps();
}

std::vector<ScaleManager::ScaleEntry*> ScaleManager::get_random_scales(size_t number_of_scales)
{
    if (number_of_scales > _entries.size())
    {
        throw std::invalid_argument(TOO_MANY_SAMPLES);
    }

    std::vector<ScaleEntry*> pointers;
    pointers.reserve(_entries.size());

    // Fill with pointers
    for (auto& entry : _entries)
    {
        pointers.push_back(&entry);
    }

    std::vector<ScaleEntry*> result;
    result.reserve(number_of_scales);

    std::random_device rd;
    std::mt19937 gen(rd());

    // Sample n pointers into result
    std::ranges::sample(pointers, std::back_inserter(result), number_of_scales, gen);

    return result;
}