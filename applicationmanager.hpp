#ifndef APPLICATIONMANAGER
#define APPLICATIONMANAGER

#include "constants.hpp"
#include "musiclibrary.hpp"
#include "scalemanager.hpp"

constexpr size_t NUMBER_OF_CHOICES = 4;

/**
 * @brief Class handling the entire application logic
 *
 * This class is expected to live from start of main till the very end of main
 *
 */
class ApplicationManager
{
   private:
    /**
     * @brief Scale-related things are delegated to the contained ScaleManager.
     *
     */
    ScaleManager _sm;

    /**
     * @brief
     *
     */
    struct Question
    {
       private:
        // Each question owns the ScaleEntry
        ScaleManager::ScaleEntry<RealisedScale> _rs;
        // And a vector of strings of multiple choice names
        // These are copied - again, not a huge problem for reasonable lenghts of a session
        std::vector<std::string> _options;
        size_t _correct_index;

       public:
        /**
         * @brief Construct a new Question object (copying)
         *
         * @param rs
         * @param options
         * @param correct_index
         */
        Question(const ScaleManager::ScaleEntry<RealisedScale>& rs,
                 const std::vector<std::string>& options, size_t correct_index)
            : _rs(rs), _options(options), _correct_index(correct_index)
        {
        }

        /**
         * @brief Construct a new Question object (stealing)
         *
         * @param rs
         * @param options
         * @param correct_index
         */
        Question(ScaleManager::ScaleEntry<RealisedScale>&& rs, std::vector<std::string>&& options,
                 size_t correct_index)
            : _rs(std::move(rs)), _options(std::move(options)), _correct_index(correct_index)
        {
        }

        friend ApplicationManager;
    };

    // AP owns the vector of Questions
    std::vector<Question> _session;
    // Stores the index of the current question
    size_t _question_index = 0;
    // We store which questions were answered correctly or not
    std::vector<bool> _correct_questions;
    // And we keep a running sum
    size_t _correct = 0;

   public:
    /**
     * @brief Construct a new Application Manager object
     *
     */
    ApplicationManager() = default;

    /**
     * @brief Destroy the Application Manager object
     *
     */
    ~ApplicationManager() = default;

    /**
     * @brief Loads the scales from the scales .csv file.
     *
     * This is simply a wrapper for the contained ScaleManager.
     *
     * @param path
     */
    inline void load_scales(std::string path) { _sm.load_scales_from_file(path); }

    /**
     * @brief Generates the list of questions for this given session.
     *
     * @param number_of_questions
     * @param difficulty
     */
    void generate_session(size_t number_of_questions, ScaleManager::Difficulty difficulty);

    /**
     * @brief Used for printing the command line header to a stream.
     *
     * @param stream
     */
    void print_header(std::ostream& stream);

    /**
     * @brief Prints the current question to a stream.
     *
     * @param stream
     */
    void print_question(std::ostream& stream);

    /**
     * @brief Parses the current question's submitted answer from a stream.
     *
     * @param stream
     */
    void load_answer(std::istream& stream);

    /**
     * @brief Moves to the next question
     *
     */
    inline void next_question() { ++_question_index; }

    /**
     * @brief 'Clears' the stream (used for the terminal here).
     *
     * @param stream
     */
    void clear_stream(std::ostream& stream);

    /**
     * @brief Returns the percentage of successful answers.
     *
     * @return size_t
     */
    inline size_t get_success_percentage()
    {
        return (size_t)(((double)_correct / _session.size()) * 100);
    }

    /**
     * @brief Used to save the results of this session to a .csv file.
     *
     * @param file_path
     */
    void save_session_results(const std::string& file_path);

    /**
     * @brief Returns if there are still questions left to answer.
     *
     * @return true
     * @return false
     */
    inline bool can_print_more() { return _question_index < _session.size(); }
};

#endif