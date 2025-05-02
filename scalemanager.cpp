#include "scalemanager.hpp"

#include <algorithm>
#include <format>
#include <random>
#include <set>

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
                        throw std::runtime_error(std::format(INVALID_DIFFICULTY, row, column));
                    }
                    break;
                }
                case (2):
                {
                    try
                    {
                        scale.clear();
                        std::istringstream sstream(column_string);
                        sstream >> scale;
                    }
                    catch (std::exception& e)
                    {
                        throw std::runtime_error(std::format(FAILED_PARSING_SCALE, row));
                    }
                    break;
                }
            }
            ++column;
        }

        if (column != 3)
        {
            throw std::runtime_error(std::format(NOT_ENOUGH_COLUMNS, row));
        }

        _scale_names.emplace_back(name);
        _entries.push_back(
            std::make_shared<ScaleManager::ScaleEntry<Scale>>(scale, difficulty, name));
        ++row;
    }
}

void ScaleManager::build_maps()
{
    for (auto&& entry : _entries)
    {
        _difficulty_map.emplace((*entry).get_difficulty(), entry);
    }
}

void ScaleManager::load_scales_from_file(std::string path)
{
    handle_file(path);
    build_maps();
}

std::vector<std::shared_ptr<ScaleManager::ScaleEntry<Scale>>> ScaleManager::get_random_scales(
    size_t number_of_scales)
{
    if (number_of_scales > _entries.size())
    {
        throw std::invalid_argument(TOO_MANY_SAMPLES);
    }

    std::vector<std::shared_ptr<ScaleEntry<Scale>>> pointers;
    pointers.reserve(_entries.size());

    // Fill with pointers
    for (auto& entry : _entries)
    {
        pointers.push_back(entry);
    }

    std::vector<std::shared_ptr<ScaleEntry<Scale>>> result;
    result.reserve(number_of_scales);

    std::random_device rd;
    std::mt19937 gen(rd());

    // Sample n pointers into result
    // Using the std::ranges sampling method
    std::ranges::sample(pointers, std::back_inserter(result), static_cast<long>(number_of_scales),
                        gen);

    return result;
}

std::vector<std::shared_ptr<ScaleManager::ScaleEntry<Scale>>>
ScaleManager::get_random_scales_by_difficulty(size_t number_of_scales,
                                              ScaleManager::Difficulty difficulty)
{
    std::vector<std::shared_ptr<ScaleManager::ScaleEntry<Scale>>> sampled_scales;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::discrete_distribution<size_t> diff_dist(static_cast<size_t>(difficulty) + 1, 0,
                                                 static_cast<size_t>(difficulty),
                                                 [](double) { return 1.0; });

    std::vector<size_t> diff_counts{_difficulty_map.count(ScaleManager::Difficulty::EASY),
                                    _difficulty_map.count(ScaleManager::Difficulty::MEDIUM),
                                    _difficulty_map.count(ScaleManager::Difficulty::HARD)};

    // In this method we have to the sampling more manually, as std::ranges::sample does not support
    // weighted sampling.

    std::vector<size_t> sampled_difficulties;
    for (size_t i = 0; i < number_of_scales; ++i)
    {
        // Logic to prevent sampling a difficulty without any scales present
        size_t sampled_difficulty;
        bool sampled = false;
        while (!sampled || diff_counts[sampled_difficulty] == 0)
        {
            sampled_difficulty = diff_dist(gen);
            sampled = true;
        }
        sampled_difficulties.emplace_back(sampled_difficulty);
    }

    std::vector<std::discrete_distribution<size_t>> individual_difficulty_distributions;
    for (auto&& diff_count : diff_counts)
    {
        individual_difficulty_distributions.emplace_back(
            diff_count, (double)0, (double)(diff_count - 1), [](double) { return 1.0; });
    }

    for (auto&& d : sampled_difficulties)
    {
        auto scales_for_d = _difficulty_map.equal_range(static_cast<ScaleManager::Difficulty>(d));
        auto chosen_scale_in_diff = individual_difficulty_distributions[d](gen);
        std::advance(scales_for_d.first, chosen_scale_in_diff);
        sampled_scales.push_back(scales_for_d.first->second);
    }

    return sampled_scales;
}

std::vector<Note*> ScaleManager::get_random_roots_by_difficulty(size_t number_of_roots,
                                                                ScaleManager::Difficulty difficulty)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::discrete_distribution<size_t> root_note_dist(
        _possible_roots.size(), 0, _possible_roots.size() - 1, [difficulty](size_t i)
        { return _root_note_weights_by_difficulty[static_cast<size_t>(difficulty)][i]; });

    std::vector<Note*> sampled_notes;

    for (size_t i = 0; i < number_of_roots; ++i)
    {
        size_t sampled_index = root_note_dist(gen);
        sampled_notes.push_back(&_possible_roots[sampled_index]);
    }

    return sampled_notes;
}

std::vector<ScaleManager::ScaleEntry<RealisedScale>>
ScaleManager::generate_realised_scales_by_difficulty(size_t number_of_scales,
                                                     ScaleManager::Difficulty difficulty)
{
    auto scales = get_random_scales_by_difficulty(number_of_scales, difficulty);
    auto roots = get_random_roots_by_difficulty(number_of_scales, difficulty);

    std::vector<ScaleManager::ScaleEntry<RealisedScale>> output;
    for (size_t i = 0; i < number_of_scales; ++i)
    {
        output.emplace_back(RealisedScale{*roots[i], scales[i]->get_scale()},
                            scales[i]->get_difficulty(), scales[i]->get_name());
    }

    return output;
}