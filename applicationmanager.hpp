#ifndef APPLICATIONMANAGER
#define APPLICATIONMANAGER

#include "musiclibrary.hpp"
#include "scalemanager.hpp"

constexpr char FORGOT_TO_LOAD_SCALES[] = "No scales found while generating session!";
constexpr char TOO_MANY_QUESTION_PRINTS[] =
    "Tried printing next question when there are none left!";

constexpr size_t NUMBER_OF_CHOICES = 4;

// AP is expected to live from start of main till the very end of main
class ApplicationManager
{
   private:
    // AM contains a SM
    ScaleManager _sm;

    struct Question
    {
       private:
        // Each question owns the scale
        ScaleManager::ScaleEntry<RealisedScale> _rs;
        // But only points to the strings of the other scales (and itself)
        std::vector<std::string> _options;
        size_t _correct_index;

       public:
        Question(const ScaleManager::ScaleEntry<RealisedScale>& rs,
                 const std::vector<std::string>& options, size_t correct_index)
            : _rs(rs), _options(options), _correct_index(correct_index)
        {
        }

        Question(ScaleManager::ScaleEntry<RealisedScale>&& rs, std::vector<std::string>&& options,
                 size_t correct_index)
            : _rs(std::move(rs)), _options(std::move(options)), _correct_index(correct_index)
        {
        }

        friend ApplicationManager;
    };

    // AP owns the vector of Questions
    std::vector<Question> _session;
    size_t _question_index = 0;
    size_t _correct = 0;

   public:
    ApplicationManager() = default;
    ~ApplicationManager() = default;

    inline void load_scales(std::string path) { _sm.load_scales_from_file(path); }
    void generate_session(size_t number_of_questions, ScaleManager::Difficulty difficulty);
    void print_header(std::ostream& stream);
    void print_question(std::ostream& stream);
    void load_answer(std::istream& stream);
    inline void next_question() { ++_question_index; }
    void clear_stream(std::ostream& stream);
    inline size_t get_success_percentage()
    {
        return (size_t)(((double)_correct / _session.size()) * 100);
    }
    void save_session_results(const std::string& file_path);

    inline bool can_print_more() { return _question_index < _session.size(); }
};

#endif