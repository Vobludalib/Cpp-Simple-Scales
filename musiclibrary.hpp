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

constexpr char NO_NOTE_INFORMATION[] = "This Note object has no MIDI or name information!";

using midi_value = int;

constexpr midi_value middle_c_midi = 60;
constexpr int middle_c_octave = 4;
constexpr midi_value notes_per_octave = 12;
constexpr char seperator = '/';

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

const std::multimap<midi_value, std::tuple<midi_value, midi_value>>
    scale_midi_offset_to_scale_degree_and_accidental = {
        {0, {0, 0}}, {1, {0, 1}},  {1, {1, -1}}, {2, {1, 0}},  {3, {1, 1}},   {3, {2, -1}},
        {4, {2, 0}}, {5, {3, 0}},  {6, {3, 1}},  {6, {4, -1}}, {7, {4, 0}},  // Perfect fifth
        {8, {4, 1}}, {8, {5, -1}}, {9, {5, 0}},  {10, {5, 1}}, {10, {6, -1}}, {11, {6, 0}}};

const std::map<midi_value, midi_value> scale_degree_to_midi_diff = {{0, 0}, {1, 2}, {2, 4}, {3, 5},
                                                                    {4, 7}, {5, 9}, {6, 11}};

constexpr char note_name_regex_pattern[] = "([a|c-zA-Z]*)(b*)(#*)([\\-|\\d]*)";
const std::regex note_name_regex{note_name_regex_pattern};

class Note
{
   private:
    struct NamingInformation
    {
        unsigned int base_degree_;
        char accidentals_;

        NamingInformation(unsigned int base_degree, char accidentals)
            : base_degree_(base_degree), accidentals_(accidentals)
        {
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
        MIDIInformation(midi_value midi_val, midi_value octave)
            : midi_value_(midi_val), octave_(octave)
        {
        }
    };

    std::optional<MIDIInformation> midi_;
    std::optional<std::vector<NamingInformation>> names_;

    std::vector<NamingInformation> generate_naming_information_from_midi(midi_value midi);
    std::tuple<NamingInformation, std::optional<MIDIInformation>>
    generate_naming_and_midi_from_string(const std::string& name);

   public:
    // Creates a note representing middle C - has MIDI information and a name
    Note();

    // Creates a note representing a MIDI value
    // Possible names are generated if generate_names is enabled.
    // Multiple names can be generated due to enharmonics (enharmonics only up to one accidental are
    // created)
    Note(midi_value midi, bool generate_names = true);

    // Creates a note representing a name
    // If a number within the valid octave range is present, then MIDI information is also generated
    Note(const std::string& name);

    // Creates a note representing a specific scale degree based off the scale root.
    // If the scale_root has a MIDI value, then the resulting note will also have a MIDI value.
    // If the scale_root has a unique name, then the resulting note will also have a unique name.
    Note(const Note& scale_root, unsigned int scale_degree, char accidentals);

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
    int accidentals = 0;
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
                if (index >= note_names.size())
                    throw std::invalid_argument(
                        "Note name root passed does not appear in the list of valid note name "
                        "roots!");
                note_name_roots_index = index;
                break;
            }
            case 2:  // Second match group should be the flats
            {
                if (match.length() > 0)
                {
                    accidentals = -match.length();
                }
                break;
            }
            case 3:  // Third match group should be sharps
            {
                if (match.length() > 0)
                {
                    if (accidentals < 0)
                        throw std::invalid_argument("Passed note name has both flats and sharps!");

                    accidentals = match.length();
                }
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
            }
        }
        ++i;
    }

    NamingInformation ni(note_name_roots_index, accidentals);
    std::optional<MIDIInformation> mi;
    if (octave_found)
    {
        int midi_offset_from_scale_c =
            note_name_roots_index * 2 - (note_name_roots_index > 2 ? 1 : 0);
        int midi = middle_c_midi + ((octave - middle_c_octave) * notes_per_octave) +
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

Note::Note(const Note& scale_root, unsigned int scale_degree, char accidentals)
{
    // Second scale degree corresponds to index one due to different indexing
    if (scale_degree == 0)
    {
        throw std::invalid_argument(
            "No such thing as as a 0th scale degree. Use 1-based indexing.");
    }

    scale_degree -= 1;

    std::optional<NamingInformation> ni;
    std::optional<MIDIInformation> midii;

    midi_value scale_degree_midi_diff =
        scale_degree_to_midi_diff.at(scale_degree % note_names.size()) +
        (notes_per_octave * (scale_degree / note_names.size()));

    if (scale_root.midi_.has_value())
    {
        midii = {scale_root.midi_.value().midi_value_ + scale_degree_midi_diff + accidentals};
    }

    if (scale_root.names_.has_value() && scale_root.names_.value().size() == 1)
    {
        midi_value root_base_degree = scale_root.names_.value()[0].base_degree_;
        midi_value new_base_degree = (root_base_degree + scale_degree) % note_names.size();
        midi_value root_midi_offset = scale_degree_to_midi_diff.at(root_base_degree) +
                                      scale_root.names_.value()[0].accidentals_;
        bool octave_offset_present = (scale_degree > 6);
        midi_value new_note_without_accidentals_midi_offset =
            scale_degree_to_midi_diff.at(new_base_degree) +
            (notes_per_octave * (scale_degree / note_names.size()));
        midi_value expected_midi_diff_from_root =
            scale_degree_to_midi_diff.at(scale_degree % note_names.size()) + accidentals +
            (notes_per_octave * (scale_degree / note_names.size()));
        midi_value midi_diff_from_root =
            new_note_without_accidentals_midi_offset - root_midi_offset;
        while (midi_diff_from_root < 0)
        {
            midi_diff_from_root = midi_diff_from_root + 12;
        }
        char needed_accidentals =
            (expected_midi_diff_from_root - midi_diff_from_root) % notes_per_octave;
        ni = {static_cast<unsigned int>(new_base_degree), needed_accidentals};
    }

    if (!scale_root.midi_.has_value() && scale_root.names_.has_value() &&
        scale_root.names_.value().size() != 1)
    {
        throw std::invalid_argument(
            "Scale degree constructor would produce a note without a MIDI value and no name. This "
            "is likely because the note passed as scale_root does not have a MIDI value or has "
            "more than one possible name");
    }

    midi_ = midii;
    if (ni.has_value()) names_ = {ni.value()};
}

std::ostream& operator<<(std::ostream& stream, const Note& note)
{
    if (note.names_.has_value())
    {
        std::optional<int> octave;
        if (note.midi_.has_value())
        {
            octave = note.midi_.value().octave_;
        }

        bool first = true;
        for (auto&& naming_info : note.names_.value())
        {
            if (!first) stream << seperator;

            stream << note_names[naming_info.base_degree_];
            std::string accidental =
                naming_info.accidentals_ < 0 ? downward_accidental : upward_accidental;

            int amount_of_accidentals = naming_info.accidentals_ < 0 ? naming_info.accidentals_ * -1
                                                                     : naming_info.accidentals_;

            for (int i = 0; i < amount_of_accidentals; ++i)
            {
                stream << accidental;
            }

            if (octave.has_value())
                stream << octave.value() << " (" << note.midi_.value().midi_value_ << ')';

            first = false;
        }
    }
    else if (note.midi_.has_value())
    {
        stream << note.midi_.value().midi_value_;
    }
    else
    {
        throw std::runtime_error(NO_NOTE_INFORMATION);
    }

    return stream;
}

// ====NOTE====

#endif