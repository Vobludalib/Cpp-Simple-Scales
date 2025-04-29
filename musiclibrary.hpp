#ifndef MUSICLIBRARY
#define MUSICLIBRARY

#include <algorithm>
#include <exception>
#include <iostream>
#include <map>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

// Constants kept together here, as this library should be able to used without being attached to
// this specific application
constexpr char NO_MIDI_INFORMATION[] = "Trying to get MIDI of a Note without MIDI information.";
constexpr char NO_NAME_INFORMATION[] = "Trying to get name of a Note without name information.";
constexpr char NOT_BOTH_INFORMATION[] =
    "Trying to get MIDI and name of a Note without information for both.";
constexpr char CREATION_NOT_BOTH_INFORMATION[] =
    "Scale degree constructor would produce a note without a MIDI value and no name. This "
    "is likely because the note passed as scale_root does not have a MIDI value or has "
    "more than one possible name";
constexpr char NO_NOTE_INFORMATION[] = "This Note object has no MIDI or name information!";
constexpr char BOTH_ACCIDENTALS_FOUND[] = "Passed note name has both flats and sharps!";
constexpr char NO_SCALE_DEGREE[] = "No scale degree passed in string!";
constexpr char BAD_SCALE_DEGREE_INDEX[] = "Scale degree is not a valid index!";
constexpr char INDEX_BASE_ERROR[] = "No such thing as as a 0th scale degree. Use 1-based indexing.";

using midi_value = int;
using accidentals_value = short;
using scale_degree_value = size_t;

constexpr midi_value MIDDLE_C_MIDI = 60;
constexpr int MIDDLE_C_OCTAVE = 4;
constexpr midi_value NOTES_PER_OCTAVE = 12;
constexpr char NOTE_PRINT_SEPERATOR = '/';
constexpr char SCALE_DEGREE_SEPERATOR = ',';

#if !defined(GERMAN_NAMING) && !defined(FRENCH_NAMING)
#define ENGLISH_NAMING
#endif

// Setting accidental style
#if defined(GERMAN_NAMING) || defined(ENGLISH_NAMING)
const std::string downward_accidental = "b";
const std::string upward_accidental = "#";
#endif

#if defined(FRENCH_NAMING)
const std::string downward_accidental = " bemol";
const std::string upward_accidental = " diese";
#endif

// Setting note names style
#ifdef ENGLISH_NAMING
const std::vector<std::string> note_names = {"C", "D", "E", "F", "G", "A", "B"};
#endif

#ifdef GERMAN_NAMING
const std::vector<std::string> note_names = {"C", "D", "E", "F", "G", "A", "H"};
#endif

#ifdef FRENCH_NAMING
const std::vector<std::string> note_names = {"Do", "Re", "Mi", "Fa", "Sol", "La", "Si"};
#endif

// ====NOTE====

class Note
{
   private:
    // The following are for internal use only, so use 0-based indexing
    inline static const std::multimap<midi_value, std::tuple<midi_value, accidentals_value>>
        scale_midi_offset_to_scale_degree_and_accidental{
            {0, {0, 0}}, {1, {0, 1}},  {1, {1, -1}}, {2, {1, 0}},  {3, {1, 1}},   {3, {2, -1}},
            {4, {2, 0}}, {5, {3, 0}},  {6, {3, 1}},  {6, {4, -1}}, {7, {4, 0}},  // Perfect fifth
            {8, {4, 1}}, {8, {5, -1}}, {9, {5, 0}},  {10, {5, 1}}, {10, {6, -1}}, {11, {6, 0}}};

    inline static const std::map<scale_degree_value, midi_value> scale_degree_to_midi_diff{
        {0, 0}, {1, 2}, {2, 4}, {3, 5}, {4, 7}, {5, 9}, {6, 11}};

    inline static const char note_name_regex_pattern[] = "([a|c-zA-Z]*)(b*)(#*)([\\-|\\d]*)";
    inline static const std::regex note_name_regex{note_name_regex_pattern};

    // base_degree_ here is an index to the note_names std::vector object, and so uses 0-based
    // indexing
    struct NamingInformation
    {
        scale_degree_value base_degree_;
        accidentals_value accidentals_;

        NamingInformation(scale_degree_value base_degree, accidentals_value accidentals);
    };

    struct MIDIInformation
    {
        midi_value midi_value_;
        int octave_;

        MIDIInformation(midi_value midi_val);

        // Used when manual octave overriding has to occur
        inline MIDIInformation(midi_value midi_val, int octave)
            : midi_value_(midi_val), octave_(octave)
        {
        }
    };

    std::optional<MIDIInformation> midi_;
    std::optional<std::vector<NamingInformation>> names_;

    std::vector<NamingInformation> generate_naming_information_from_midi(midi_value midi);
    std::tuple<NamingInformation, std::optional<MIDIInformation>>
    generate_naming_and_midi_from_string(const std::string& name);
    std::tuple<std::optional<NamingInformation>, std::optional<MIDIInformation>>
    generate_naming_and_midi_from_root_and_scale_degree(const Note& scale_root,
                                                        scale_degree_value scale_degree,
                                                        accidentals_value accidentals);

    inline bool check_has_midi() const { return midi_.has_value(); }
    inline bool check_has_name() const
    {
        return !(!names_.has_value() || names_.value().size() == 0);
    }

    bool have_to_regenerate_name = true;
    std::string name_as_string;
    std::string generate_name_as_string() const;

    std::string complex_name;
    std::string generate_complex_name_as_string() const;

   public:
    // Creates a note representing middle C - has MIDI information and a name
    Note();

    // Creates a note representing a MIDI value
    // Possible names are generated if generate_names is enabled.
    // Multiple names can be generated due to enharmonics (enharmonics only up to one accidental are
    // created)
    Note(midi_value midi, bool generate_names = true);
    void set_note(midi_value midi, bool generate_names = true);

    // Creates a note representing a name
    // If a number within the valid octave range is present, then MIDI information is also generated
    Note(const std::string& name);
    void set_note(const std::string& name);

    // Creates a note representing a specific scale degree based off the scale root.
    // If the scale_root has a MIDI value, then the resulting note will also have a MIDI value.
    // If the scale_root has a unique name, then the resulting note will also have a unique name.
    Note(const Note& scale_root, scale_degree_value scale_degree, accidentals_value accidentals);
    void set_note(const Note& scale_root, scale_degree_value scale_degree,
                  accidentals_value accidentals);

    inline midi_value get_midi() const
    {
        if (midi_.has_value())
            return midi_.value().midi_value_;
        else
            throw std::runtime_error(NO_MIDI_INFORMATION);
    }

    std::string& get_name();
    std::string get_name() const;
    std::string& get_name_and_midi_string();
    std::string get_name_and_midi_string() const;

    ~Note() = default;

    friend std::ostream& operator<<(std::ostream& stream, const Note& note);
};

// ====NOTE====
// ====SCALE====

class Scale
{
   private:
    using scale_degree = std::pair<scale_degree_value, accidentals_value>;

    inline static const char scale_degree_regex_pattern[] = "(b*)(#*)([\\-|\\d]*)";
    inline static const std::regex scale_degree_regex{scale_degree_regex_pattern};

    std::vector<scale_degree> _scale_degrees;
    static scale_degree parse_scale_degree_string(const std::string& input);

   public:
    Scale() = default;
    inline Scale(const std::vector<scale_degree>& degrees) : _scale_degrees(degrees) {};
    inline Scale(std::vector<scale_degree>&& degrees) : _scale_degrees(std::move(degrees)) {};

    inline void clear() { return _scale_degrees.clear(); }
    inline size_t size() { return _scale_degrees.size(); }
    friend std::istream& operator>>(std::istream& stream, Scale& scale);
    friend std::ostream& operator<<(std::ostream& stream, const Scale& scale);

    inline auto begin() { return _scale_degrees.begin(); }
    inline auto end() { return _scale_degrees.end(); }
    inline auto begin() const { return _scale_degrees.begin(); }
    inline auto end() const { return _scale_degrees.end(); }
    inline auto cbegin() const { return _scale_degrees.cbegin(); }
    inline auto cend() const { return _scale_degrees.cend(); }

    inline scale_degree& operator[](size_t index) { return _scale_degrees[index]; }
    inline const scale_degree& operator[](size_t index) const { return _scale_degrees[index]; }
};

// ====SCALE====
// ====REALISEDSCALE====

class RealisedScale
{
   private:
    std::vector<Note> _notes;
    std::vector<Note> realise_scale(const Note& root, const Scale& scale);

   public:
    RealisedScale() = default;
    RealisedScale(const Note& root, const Scale& scale);
    inline const Note& get_root() const { return _notes[0]; }

    friend std::ostream& operator<<(std::ostream& stream, const RealisedScale& scale);
};

// ====REALISEDSCALE====

#endif