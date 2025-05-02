#ifndef SCALEMANAGER
#define SCALEMANAGER

#include <fstream>
#include <map>
#include <memory>
#include <string>

#include "constants.hpp"
#include "musiclibrary.hpp"

/**
 * @brief Class responsible for handling the loading of scales and generating questions
 *
 */
class ScaleManager
{
   public:
    /**
     * @brief Enum for representing difficulty
     *
     */
    enum class Difficulty
    {
        EASY = 0,
        MEDIUM,
        HARD
    };

   private:
    /**
     * @brief Templated structure for holding information about a scale and its associated name and
     * difficulty
     *
     * @tparam T
     */
    template <typename T>
    struct ScaleEntry
    {
       private:
        T _scale;
        ScaleManager::Difficulty _difficulty;
        std::string _name;

       public:
        /**
         * @brief Construct a new Scale Entry object (copying)
         *
         * @param scale
         * @param difficulty
         * @param name
         */
        inline ScaleEntry(const T& scale, const ScaleManager::Difficulty& difficulty,
                          const std::string& name)
            : _scale(scale), _difficulty(difficulty), _name(name)
        {
        }

        /**
         * @brief Construct a new Scale Entry object (stealing)
         *
         * @param scale
         * @param difficulty
         * @param name
         */
        inline ScaleEntry(T&& scale, ScaleManager::Difficulty&& difficulty, std::string&& name)
            : _scale(std::move(scale)), _difficulty(std::move(difficulty)), _name(std::move(name))
        {
        }

        /**
         * @brief Get the scale object
         *
         * @return const T&
         */
        inline const T& get_scale() const { return _scale; }

        /**
         * @brief Get the difficulty value
         *
         * @return const ScaleManager::Difficulty&
         */
        inline const ScaleManager::Difficulty& get_difficulty() const { return _difficulty; }

        /**
         * @brief Get the scale name string
         *
         * @return const std::string&
         */
        inline const std::string& get_name() const { return _name; }
    };

    /**
     * @brief Vector holding shared_ptr to all the scales that were loaded
     *
     */
    std::vector<std::shared_ptr<ScaleEntry<Scale>>> _entries;

    /**
     * @brief Multimap for easier filtering by difficulty
     *
     */
    std::multimap<Difficulty, std::shared_ptr<ScaleEntry<Scale>>> _difficulty_map;

    /**
     * @brief Holds copies of the names (strings) of all loaded scales
     *
     * The fact that this holds the strings as a copy and not as pointer to the ScaleEntries
     * themselves is simply because this is more resilient to changes in the code and the copying
     * here relatively cheap. The amount of possible scale entries is not large enough for this to
     * be a problem where some memory is being doubled. Can be refactored when/if needed.
     *
     *
     */
    std::vector<std::string> _scale_names;

    /**
     * @brief Used static middle C Note for use when generating the roots
     *
     */
    inline static Note _middle_c{};

    /**
     * @brief Map of all possible roots of the scales that correspond to the most common scale
     * roots. For example, we do not have Fb major, as E major exists. We are not here to test the
     * most obscure of enharmonics.
     *
     */
    inline static std::vector<Note> _possible_roots{
        {_middle_c, 1, 0}, {_middle_c, 2, -1}, {_middle_c, 2, 0}, {_middle_c, 3, -1},
        {_middle_c, 3, 0}, {_middle_c, 4, 0},  {_middle_c, 4, 1}, {_middle_c, 5, -1},
        {_middle_c, 5, 0}, {_middle_c, 6, -1}, {_middle_c, 6, 0}, {_middle_c, 7, -1},
        {_middle_c, 7, 0}};

    /**
     * @brief Certain roots are more difficult due to a lack of exposure or a larger amount of
     * accidentals, so they only appear in higher difficulties.
     *
     */
    inline static std::vector<std::vector<double>> _root_note_weights_by_difficulty{
        {1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0},
        {1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0},
        {2, 1, 2, 2, 2, 2, 1, 1, 2, 1, 2, 2, 1}};

    /**
     * @brief Wrapper function around the file opening and closing procedure.
     *
     * @param path
     */
    void handle_file(std::string path);

    /**
     * @brief Implements the actual parsing of the file stream into a bunch of ScaleEntry objects.
     *
     * @param stream
     */
    void parse_fstream(std::ifstream& stream);

    /**
     * @brief Used to build the difficulty to ScaleEntry map after all scales are loaded.
     *
     */
    void build_maps();

    /**
     * @brief Get a set amount of random (shared_ptrs to) ScaleEntries of Scales.
     *
     * Pointers are used so we don't keep copying ScaleEntries.
     * This function is generally not used, as we don't have control of difficulty.
     *
     * @param number_of_scales
     * @return std::vector<std::shared_ptr<ScaleEntry<Scale>>>
     */
    std::vector<std::shared_ptr<ScaleEntry<Scale>>> get_random_scales(size_t number_of_scales);

    /**
     * @brief Get a set amount of random (shared_ptrs to) ScaleEntreis of Scales sampled by
     * difficulty.
     *
     * Pointers are used so we don't keep copying ScaleEntries.
     * We first sample questions by difficulty (each difficulty up to the set one has an equal
     * likelihood), then for each question we sample from scales within the question's difficulty.
     *
     * @param number_of_scales
     * @param difficulty
     * @return std::vector<std::shared_ptr<ScaleEntry<Scale>>>
     */
    std::vector<std::shared_ptr<ScaleEntry<Scale>>> get_random_scales_by_difficulty(
        size_t number_of_scales, ScaleManager::Difficulty difficulty);

    /**
     * @brief Get a set amount of random root notes, sampled by the given difficulty's weights.
     *
     * We can use raw pointers here, as the vector of Notes _possible_roots is constructed once and
     * never changes, so the vector entries don't move. This is not the case for get_random_scales
     * methods, as they emplace into a new vector, where the container might move during the
     * sampling process.
     *
     * @param number_of_roots
     * @param difficulty
     * @return std::vector<Note*>
     */
    std::vector<Note*> get_random_roots_by_difficulty(size_t number_of_roots,
                                                      ScaleManager::Difficulty difficulty);

   public:
    /**
     * @brief Public calling function to load the scales from a .csv file.
     *
     * @param path
     */
    void load_scales_from_file(std::string path);

    /**
     * @brief Generates a set amount of RealisedScale ScaleEntries as questions.
     *
     * This combines difficulty-based sampling of scales and root notes.
     *
     * @param number_of_scales
     * @param difficulty
     * @return std::vector<ScaleManager::ScaleEntry<RealisedScale>>
     */
    std::vector<ScaleManager::ScaleEntry<RealisedScale>> generate_realised_scales_by_difficulty(
        size_t number_of_scales, ScaleManager::Difficulty difficulty);

    friend class ApplicationManager;
};

#endif