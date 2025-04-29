#include "applicationmanager.hpp"

#include <algorithm>
#include <fstream>
#include <random>
#include <ranges>

#include "scalemanager.hpp"

void ApplicationManager::generate_session(size_t number_of_questions,
                                          ScaleManager::Difficulty difficulty)
{
    if (_sm._entries.size() == 0)
    {
        throw std::runtime_error(FORGOT_TO_LOAD_SCALES);
    }

    auto generated_scales =
        _sm.generate_realised_scales_by_difficulty(number_of_questions, difficulty);

    auto rd = std::mt19937{std::random_device{}()};

    for (auto&& scale : generated_scales)
    {
        std::vector<std::string> possible_names{scale.get_name()};

        auto not_same_names = std::views::filter(
            _sm._scale_names, [scale](std::string& name) { return name != scale.get_name(); });

        std::sample(not_same_names.begin(), not_same_names.end(),
                    std::back_inserter(possible_names), NUMBER_OF_CHOICES - 1, rd);

        std::ranges::shuffle(possible_names, rd);
        auto comp = [scale](std::string& str) { return str == scale.get_name(); };

        auto it = std::find_if(possible_names.begin(), possible_names.end(), comp);
        size_t index = static_cast<size_t>(it - possible_names.begin());

        _session.emplace_back(std::move(scale), std::move(possible_names), index);
    }
}

void ApplicationManager::print_header(std::ostream& stream)
{
    stream << "On question " << _question_index + 1 << '/' << _session.size() << std::endl;
}

void ApplicationManager::print_question(std::ostream& stream)
{
    if (_question_index >= _session.size()) throw std::runtime_error(TOO_MANY_QUESTION_PRINTS);
    auto& current_q = _session[_question_index];
    stream << current_q._rs.get_scale() << std::endl;
    for (size_t i = 0; i < NUMBER_OF_CHOICES; ++i)
    {
        stream << i + 1 << ": " << current_q._options[i] << std::endl;
    }
}

void ApplicationManager::load_answer(std::istream& stream)
{
    size_t answer;
    stream >> answer;
    size_t guessed_index = answer - 1;
    if (guessed_index == _session[_question_index]._correct_index)
    {
        ++_correct;
        _correct_questions.emplace_back(true);
    }
    else
    {
        _correct_questions.emplace_back(false);
    }
}

// This is a very dumb way of doing it, but it works for now. Would be changed with proper external
// library support, but this is good enough for now
void ApplicationManager::clear_stream(std::ostream& stream)
{
    for (size_t i = 0; i < 20; ++i) stream << std::endl;
}

void ApplicationManager::save_session_results(const std::string& file_path)
{
    std::ofstream file(file_path);
    if (!file.good())
    {
        throw std::runtime_error(BAD_FILE_OPEN);
    }

    for (size_t i = 0; i < _session.size(); ++i)
    {
        file << _session[i]._rs.get_scale().get_root().get_name() << ' '
             << _session[i]._rs.get_name() << CSV_SEPERATOR
             << (size_t)_session[i]._rs.get_difficulty() << CSV_SEPERATOR
             << (_correct_questions[i] ? CORRECT : INCORRECT) << std::endl;
    }

    file.close();
}