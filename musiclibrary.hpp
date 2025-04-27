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

constexpr char NO_MIDI_INFORMATION[] = "Trying to get MIDI of a Note without MIDI information.";
constexpr char NO_NAME_INFORMATION[] = "Trying to get name of a Note without name information.";
constexpr char NOT_BOTH_INFORMATION[] =
    "Trying to get MIDI and name of a Note without information for both.";
constexpr char NO_NOTE_INFORMATION[] = "This Note object has no MIDI or name information!";
constexpr char BOTH_ACCIDENTALS_FOUND[] = "Passed note name has both flats and sharps!";
constexpr char NO_SCALE_DEGREE[] = "No scale degree passed in string!";

using midi_value = int;
using accidentals_value = short;
using scale_degree_value = size_t;

constexpr midi_value middle_c_midi = 60;
constexpr int middle_c_octave = 4;
constexpr midi_value notes_per_octave = 12;
constexpr char note_print_seperator = '/';
constexpr char scale_degree_seperator = ',';

// TODO: Change this behaviour into a formatter class
// Setting default for which system to use
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

        NamingInformation(scale_degree_value base_degree, accidentals_value accidentals)
            : base_degree_(base_degree), accidentals_(accidentals)
        {
            if (base_degree > note_names.size() - 1)
            {
                throw std::invalid_argument(
                    "Base degree is set above the amount of note names we have registered!");
            }
        }
    };

    struct MIDIInformation
    {
        midi_value midi_value_;
        int octave_;

        MIDIInformation(midi_value midi_val)
            : midi_value_(midi_val),
              octave_(middle_c_octave + ((midi_val - middle_c_midi) / notes_per_octave) -
                      (midi_val - middle_c_midi < 0 ? 1 : 0))
        {
        }

        // Used when manual octave overriding has to occur
        MIDIInformation(midi_value midi_val, int octave) : midi_value_(midi_val), octave_(octave) {}
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

    std::string get_name();
    std::string get_name() const;
    std::string get_name_and_midi_string();
    std::string get_name_and_midi_string() const;

    ~Note() = default;

    friend std::ostream& operator<<(std::ostream& stream, const Note& note);
};

Note::Note() : midi_(middle_c_midi) { names_ = {NamingInformation{0, 0}}; }

// For any MIDI value that requires an accidental, both variants are generated
std::vector<Note::NamingInformation> Note::generate_naming_information_from_midi(midi_value midi)
{
    midi_value offset_from_middle_c = midi - middle_c_midi;
    midi_value midi_offset_from_c_in_scale =
        (notes_per_octave + (offset_from_middle_c % notes_per_octave)) % notes_per_octave;

    std::vector<NamingInformation> names;

    auto range =
        scale_midi_offset_to_scale_degree_and_accidental.equal_range(midi_offset_from_c_in_scale);

    for (auto it = range.first; it != range.second; ++it)
    {
        auto [base_degree, accidental] = it->second;
        names.emplace_back(base_degree, accidental);
    }

    return names;
}

Note::Note(midi_value midi, bool generate_names) : midi_({midi})
{
    if (generate_names)
    {
        names_ = generate_naming_information_from_midi(midi);
    }
    have_to_regenerate_name = true;
}

void Note::set_note(midi_value midi, bool generate_names)
{
    midi_ = {midi};
    names_ = {};
    if (generate_names)
    {
        names_ = generate_naming_information_from_midi(midi);
    }
    have_to_regenerate_name = true;
}

// Expects name to be in the form [Base Note Name][optional multiple # or b chars][optional number
// for octave]
std::tuple<Note::NamingInformation, std::optional<Note::MIDIInformation>>
Note::generate_naming_and_midi_from_string(const std::string& name)
{
    std::smatch matches;
    std::regex_search(name, matches, note_name_regex);
    size_t i = 0;
    size_t note_name_roots_index = 0;
    accidentals_value accidentals = 0;
    int octave = 0;
    bool octave_found = false;
    for (auto&& match : matches)
    {
        switch (i)
        {
            case 1:  // First match group should be the note name root
            {
                auto it = std::find(note_names.begin(), note_names.end(), match.str());
                auto index = std::distance(note_names.begin(), it);
                if (static_cast<unsigned long>(index) >= note_names.size())
                {
                    bool is_ok = false;
#ifdef GERMAN_NAMING
                    // Special-casing for the German naming system
                    if (match.str() == "B")
                    {
                        note_name_roots_index = 6;
                        accidentals = -1;
                        is_ok = true;
                    }
#endif
                    if (!is_ok)
                        throw std::invalid_argument(
                            "Note name root passed does not appear in the list of valid note name "
                            "roots!");
                }
                note_name_roots_index = static_cast<size_t>(index);
                break;
            }
            case 2:  // Second match group should be the flats
            {
                if (match.length() > 0)
                {
                    accidentals += -static_cast<accidentals_value>(match.length());
                }
                break;
            }
            case 3:  // Third match group should be sharps
            {
                if (match.length() > 0)
                {
                    if (accidentals < 0) throw std::invalid_argument(BOTH_ACCIDENTALS_FOUND);

                    accidentals += static_cast<accidentals_value>(match.length());
                }
                break;
            }
            case 4:  // Fourth match group is for the possible octave
            {
                if (match.length() > 0)
                {
                    // I know that int to int truncation is occuring here. For reasonable octave
                    // values this is not an issue
                    octave = std::stoi(match.str());
                    octave_found = true;
                }
                break;
            }
        }
        ++i;
    }

    NamingInformation ni(note_name_roots_index, accidentals);
    std::optional<MIDIInformation> mi;
    if (octave_found)
    {
        midi_value midi_offset_from_scale_c = static_cast<midi_value>(
            note_name_roots_index * 2 - (note_name_roots_index > 2 ? 1 : 0));
        midi_value midi = middle_c_midi + ((octave - middle_c_octave) * notes_per_octave) +
                          midi_offset_from_scale_c + accidentals;
        mi = MIDIInformation{midi, octave};
    }

    return std::tuple<NamingInformation, std::optional<MIDIInformation>>{ni, mi};
}

Note::Note(const std::string& name)
{
    auto [naming, midi] = generate_naming_and_midi_from_string(name);
    names_ = {naming};
    midi_ = midi;
}

void Note::set_note(const std::string& name)
{
    auto [naming, midi] = generate_naming_and_midi_from_string(name);
    names_ = {naming};
    midi_ = midi;
    have_to_regenerate_name = true;
}

std::tuple<std::optional<Note::NamingInformation>, std::optional<Note::MIDIInformation>>
Note::generate_naming_and_midi_from_root_and_scale_degree(const Note& scale_root,
                                                          scale_degree_value scale_degree,
                                                          accidentals_value accidentals)
{
    // Second scale degree corresponds to index one due to different indexing
    if (scale_degree == 0)
    {
        throw std::invalid_argument(
            "No such thing as as a 0th scale degree. Use 1-based indexing.");
    }

    scale_degree -= 1;

    std::optional<NamingInformation> namei;
    std::optional<MIDIInformation> midii;

    if (scale_root.midi_.has_value())
    {
        midi_value scale_degree_midi_diff =
            scale_degree_to_midi_diff.at(scale_degree % note_names.size()) +
            static_cast<midi_value>((notes_per_octave * (scale_degree / note_names.size())));
        midii = {scale_root.midi_.value().midi_value_ + scale_degree_midi_diff + accidentals};
    }

    if (scale_root.names_.has_value() && scale_root.names_.value().size() == 1)
    {
        size_t number_of_note_names = note_names.size();
        scale_degree_value root_base_degree = scale_root.names_.value()[0].base_degree_;
        scale_degree_value new_base_degree =
            (root_base_degree + scale_degree) % number_of_note_names;
        midi_value root_midi_offset_from_c = scale_degree_to_midi_diff.at(root_base_degree) +
                                             scale_root.names_.value()[0].accidentals_;
        midi_value scale_degree_without_accidentals_midi_offset_from_c =
            scale_degree_to_midi_diff.at(new_base_degree);
        if (scale_degree_without_accidentals_midi_offset_from_c < root_midi_offset_from_c)
        {
            scale_degree_without_accidentals_midi_offset_from_c += notes_per_octave;
        }
        midi_value expected_midi_diff_from_root =
            scale_degree_to_midi_diff.at(scale_degree % number_of_note_names) + accidentals;
        midi_value unaccidented_midi_diff_from_root =
            scale_degree_without_accidentals_midi_offset_from_c - root_midi_offset_from_c;
        accidentals_value needed_accidentals = static_cast<accidentals_value>(
            expected_midi_diff_from_root - unaccidented_midi_diff_from_root);
        namei = {static_cast<scale_degree_value>(new_base_degree), needed_accidentals};
    }

    if (!scale_root.midi_.has_value() && scale_root.names_.has_value() &&
        scale_root.names_.value().size() != 1)
    {
        throw std::invalid_argument(
            "Scale degree constructor would produce a note without a MIDI value and no name. This "
            "is likely because the note passed as scale_root does not have a MIDI value or has "
            "more than one possible name");
    }

    return std::tuple{namei, midii};
}

Note::Note(const Note& scale_root, scale_degree_value scale_degree, accidentals_value accidentals)
{
    auto [naming, midi] =
        generate_naming_and_midi_from_root_and_scale_degree(scale_root, scale_degree, accidentals);
    midi_ = midi;
    if (naming.has_value()) names_ = {naming.value()};
}

void Note::set_note(const Note& scale_root, scale_degree_value scale_degree,
                    accidentals_value accidentals)
{
    auto [naming, midi] =
        generate_naming_and_midi_from_root_and_scale_degree(scale_root, scale_degree, accidentals);
    midi_ = midi;
    if (naming.has_value()) names_ = {naming.value()};
    have_to_regenerate_name = true;
}

std::string Note::generate_name_as_string() const
{
    std::ostringstream stream;
    bool first = true;
    for (auto&& naming_info : names_.value())
    {
        if (!first) stream << note_print_seperator;

#ifdef GERMAN_NAMING
        // German special casing
        if (naming_info.base_degree_ == 6 && naming_info.accidentals_ < 0)
        {
            stream << 'B';
        }
        else
        {
            stream << note_names[naming_info.base_degree_];
        }
#endif
#ifndef GERMAN_NAMING
        stream << note_names[naming_info.base_degree_];
#endif
        std::string accidental =
            naming_info.accidentals_ < 0 ? downward_accidental : upward_accidental;

        int amount_of_accidentals =
            naming_info.accidentals_ < 0 ? naming_info.accidentals_ * -1 : naming_info.accidentals_;

#ifdef GERMAN_NAMING
        // German special casing
        if (naming_info.base_degree_ == 6 && naming_info.accidentals_ < 0)
        {
            amount_of_accidentals -= 1;
        }
#endif

        for (int i = 0; i < amount_of_accidentals; ++i)
        {
            stream << accidental;
        }

        first = false;
    }

    return stream.str();
}

std::string Note::get_name()
{
    if (!(check_has_name())) throw std::runtime_error(NO_NAME_INFORMATION);
    if (have_to_regenerate_name) name_as_string = generate_name_as_string();
    return name_as_string;
}

std::string Note::get_name() const
{
    if (!(check_has_name())) throw std::runtime_error(NO_NAME_INFORMATION);
    if (!have_to_regenerate_name) return name_as_string;
    return generate_name_as_string();
}

std::string Note::generate_complex_name_as_string() const
{
    std::ostringstream stream;
    int octave = midi_.value().octave_;

    bool first = true;
    for (auto&& naming_info : names_.value())
    {
        if (!first) stream << note_print_seperator;

        stream << get_name();

        stream << octave << " (" << midi_.value().midi_value_ << ')';

        first = false;
    }

    return stream.str();
}

std::string Note::get_name_and_midi_string()
{
    if (!check_has_midi() || !check_has_name()) throw std::runtime_error(NOT_BOTH_INFORMATION);
    if (have_to_regenerate_name) complex_name = generate_complex_name_as_string();
    return complex_name;
}

std::string Note::get_name_and_midi_string() const
{
    if (!check_has_midi() || !check_has_name()) throw std::runtime_error(NOT_BOTH_INFORMATION);
    if (!have_to_regenerate_name)
    {
        return complex_name;
    }
    return generate_complex_name_as_string();
}

std::ostream& operator<<(std::ostream& stream, const Note& note)
{
    if (note.check_has_midi() && note.check_has_name())
        stream << note.get_name_and_midi_string();
    else if (note.check_has_midi())
    {
        stream << note.get_midi();
    }
    else if (note.check_has_name())
    {
        stream << note.get_name();
    }
    else
    {
        throw std::runtime_error(NO_NAME_INFORMATION);
    }

    return stream;
}

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
    Scale(const std::vector<scale_degree>& degrees) : _scale_degrees(degrees) {};
    Scale(std::vector<scale_degree>&& degrees) : _scale_degrees(std::move(degrees)) {};

    inline void clear() { return _scale_degrees.clear(); }
    inline size_t size() { return _scale_degrees.size(); }
    friend std::istream& operator>>(std::istream& stream, Scale& scale);
    friend std::ostream& operator<<(std::ostream& stream, const Scale& scale);

    auto begin() { return _scale_degrees.begin(); }
    auto end() { return _scale_degrees.end(); }
    auto begin() const { return _scale_degrees.begin(); }
    auto end() const { return _scale_degrees.end(); }
    auto cbegin() const { return _scale_degrees.cbegin(); }
    auto cend() const { return _scale_degrees.cend(); }

    scale_degree& operator[](size_t index) { return _scale_degrees[index]; }
    const scale_degree& operator[](size_t index) const { return _scale_degrees[index]; }
};

Scale::scale_degree Scale::parse_scale_degree_string(const std::string& input)
{
    std::smatch matches;
    std::regex_search(input, matches, scale_degree_regex);
    size_t i = 0;
    scale_degree_value sd = 0;
    accidentals_value accidentals = 0;
    for (auto&& match : matches)
    {
        switch (i)
        {
            case 1:  // First match group should be the flats
            {
                if (match.length() > 0)
                {
                    accidentals = -static_cast<accidentals_value>(match.length());
                }
                break;
            }
            case 2:  // Second match group should be the sharps
            {
                if (match.length() > 0)
                {
                    if (accidentals < 0)
                    {
                        throw std::invalid_argument(BOTH_ACCIDENTALS_FOUND);
                    }

                    accidentals = static_cast<accidentals_value>(match.length());
                }
                break;
            }
            case 3:  // Third match group should be the scale degree
            {
                if (match.length() > 0)
                {
                    sd = static_cast<scale_degree_value>(std::stoi(match.str()));
                }
                else
                {
                    throw std::invalid_argument(NO_SCALE_DEGREE);
                }
            }
        }
        ++i;
    }

    return {sd, accidentals};
}

std::istream& operator>>(std::istream& stream, Scale& scale)
{
    std::string line;
    std::getline(stream, line);

    std::stringstream line_stream(line);
    std::string scale_degree_str;

    scale.clear();

    while (std::getline(line_stream, scale_degree_str, scale_degree_seperator))
    {
        scale._scale_degrees.emplace_back(Scale::parse_scale_degree_string(scale_degree_str));
    }

    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Scale& scale)
{
    bool first = true;
    for (auto&& sd : scale._scale_degrees)
    {
        if (!first) stream << scale_degree_seperator << ' ';
        if (sd.second < 0)
        {
            for (accidentals_value acc_i = 0; acc_i > sd.second; --acc_i)
            {
                stream << downward_accidental;
            }
        }
        else if (sd.second > 0)
        {
            for (accidentals_value acc_i = 0; acc_i > sd.second; ++acc_i)
            {
                stream << upward_accidental;
            }
        }
        stream << sd.first;

        first = false;
    }
    return stream;
}

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

    friend std::ostream& operator<<(std::ostream& stream, const RealisedScale& scale);
};

std::vector<Note> RealisedScale::realise_scale(const Note& root, const Scale& scale)
{
    std::vector<Note> result;
    for (auto&& sd : scale)
    {
        if (sd.first == 1)
        {
            result.emplace_back(root);
        }
        else
        {
            result.emplace_back(Note{root, sd.first, sd.second});
        }
    }

    return result;
}

RealisedScale::RealisedScale(const Note& root, const Scale& scale)
{
    _notes = std::move(realise_scale(root, scale));
}

std::ostream& operator<<(std::ostream& stream, const RealisedScale& scale)
{
    bool first = true;
    for (auto&& note : scale._notes)
    {
        if (!first) stream << scale_degree_seperator << ' ';
        first = false;
        stream << note;
    }
    return stream;
}

// ====REALISEDSCALE====

#endif