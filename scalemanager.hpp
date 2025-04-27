#ifndef SCALEMANAGER
#define SCALEMANAGER

#include <fstream>
#include <map>
#include <string>

#include "musiclibrary.hpp"

constexpr char BAD_FILE_OPEN[] = "Unable to open the file!";
constexpr char INVALID_DIFFICULTY[] = "Invalid difficulty value found during parsing file!";
constexpr char TOO_MANY_SAMPLES[] = "Too many samples requested!";
constexpr char CSV_SEPERATOR = ';';

class ScaleManager
{
   private:
    enum class Difficulty
    {
        EASY = 0,
        MEDIUM,
        HARD
    };

    struct ScaleEntry
    {
       private:
        Scale _scale;
        ScaleManager::Difficulty _difficulty;
        std::string _name;

       public:
        inline ScaleEntry(const Scale& scale, const ScaleManager::Difficulty& difficulty,
                          const std::string& name)
            : _scale(scale), _difficulty(difficulty), _name(name)
        {
        }

        inline ScaleEntry(Scale&& scale, ScaleManager::Difficulty&& difficulty, std::string&& name)
            : _scale(std::move(scale)), _difficulty(std::move(difficulty)), _name(std::move(name))
        {
        }

        inline const Scale& get_scale() const { return _scale; }
        inline const ScaleManager::Difficulty& get_difficulty() const { return _difficulty; }
        inline const std::string& get_name() const { return _name; }
    };

    // Vector actually owns all the entries, the other containers just reference elements of the
    // vector
    std::vector<ScaleEntry> _entries;
    std::multimap<Difficulty, ScaleEntry*> _difficulty_map;

    void handle_file(std::string path);
    void parse_fstream(std::ifstream& stream);
    void build_maps();

   public:
    void load_scales_from_file(std::string path);
    std::vector<ScaleEntry*> get_random_scales(size_t number_of_scales);
};

#endif