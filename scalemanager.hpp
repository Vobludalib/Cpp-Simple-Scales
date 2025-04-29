#ifndef SCALEMANAGER
#define SCALEMANAGER

#include <fstream>
#include <map>
#include <memory>
#include <string>

#include "constants.hpp"
#include "musiclibrary.hpp"

class ScaleManager
{
   public:
    enum class Difficulty
    {
        EASY = 0,
        MEDIUM,
        HARD
    };

   private:
    template <typename T>
    struct ScaleEntry
    {
       private:
        T _scale;
        ScaleManager::Difficulty _difficulty;
        std::string _name;

       public:
        inline ScaleEntry(const T& scale, const ScaleManager::Difficulty& difficulty,
                          const std::string& name)
            : _scale(scale), _difficulty(difficulty), _name(name)
        {
        }

        inline ScaleEntry(T&& scale, ScaleManager::Difficulty&& difficulty, std::string&& name)
            : _scale(std::move(scale)), _difficulty(std::move(difficulty)), _name(std::move(name))
        {
        }

        inline const T& get_scale() const { return _scale; }
        inline const ScaleManager::Difficulty& get_difficulty() const { return _difficulty; }
        inline const std::string& get_name() const { return _name; }
    };

    // Vector actually owns all the entries, the other containers just reference elements of the
    // vector
    std::vector<std::shared_ptr<ScaleEntry<Scale>>> _entries;
    std::multimap<Difficulty, std::shared_ptr<ScaleEntry<Scale>>> _difficulty_map;
    std::vector<std::string> _scale_names;

    inline static Note _middle_c{};
    inline static std::vector<Note> _possible_roots{
        {_middle_c, 1, 0}, {_middle_c, 2, -1}, {_middle_c, 2, 0}, {_middle_c, 3, -1},
        {_middle_c, 3, 0}, {_middle_c, 4, 0},  {_middle_c, 4, 1}, {_middle_c, 5, -1},
        {_middle_c, 5, 0}, {_middle_c, 6, -1}, {_middle_c, 6, 0}, {_middle_c, 7, -1},
        {_middle_c, 7, 0}};

    inline static std::vector<std::vector<double>> _root_note_weights_by_difficulty{
        {1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0},
        {1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0},
        {2, 1, 2, 2, 2, 2, 1, 1, 2, 1, 2, 2, 1}};

    void handle_file(std::string path);
    void parse_fstream(std::ifstream& stream);
    void build_maps();

    std::vector<std::shared_ptr<ScaleEntry<Scale>>> get_random_scales(size_t number_of_scales);
    std::vector<std::shared_ptr<ScaleEntry<Scale>>> get_random_scales_by_difficulty(
        size_t number_of_scales, ScaleManager::Difficulty difficulty);
    // We can use raw pointers here, as the vector of Notes _possible_roots is constructed once and
    // never changes, so the vector entries don't move
    std::vector<Note*> get_random_roots_by_difficulty(size_t number_of_roots,
                                                      ScaleManager::Difficulty difficulty);

   public:
    void load_scales_from_file(std::string path);
    std::vector<ScaleManager::ScaleEntry<RealisedScale>> generate_realised_scales_by_difficulty(
        size_t number_of_scales, ScaleManager::Difficulty difficulty);

    friend class ApplicationManager;
};

#endif