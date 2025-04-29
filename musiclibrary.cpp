#include "musiclibrary.hpp"

// ====NOTE====

Note::NamingInformation::NamingInformation(scale_degree_value base_degree,
                                           accidentals_value accidentals)
    : base_degree_(base_degree), accidentals_(accidentals)
{
    if (base_degree > note_names.size() - 1)
    {
        throw std::invalid_argument(BAD_SCALE_DEGREE_INDEX);
    }
}

Note::MIDIInformation::MIDIInformation(midi_value midi_val)
    : midi_value_(midi_val),
      octave_(MIDDLE_C_OCTAVE + ((midi_val - MIDDLE_C_MIDI) / NOTES_PER_OCTAVE) -
              (midi_val - MIDDLE_C_MIDI < 0 ? 1 : 0))
{
}

Note::Note() : midi_(MIDDLE_C_MIDI) { names_ = {NamingInformation{0, 0}}; }

// For any MIDI value that requires an accidental, both variants are generated
std::vector<Note::NamingInformation> Note::generate_naming_information_from_midi(midi_value midi)
{
    midi_value offset_from_middle_c = midi - MIDDLE_C_MIDI;
    midi_value midi_offset_from_c_in_scale =
        (NOTES_PER_OCTAVE + (offset_from_middle_c % NOTES_PER_OCTAVE)) % NOTES_PER_OCTAVE;

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
        midi_value midi = MIDDLE_C_MIDI + ((octave - MIDDLE_C_OCTAVE) * NOTES_PER_OCTAVE) +
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
        throw std::invalid_argument(INDEX_BASE_ERROR);
    }

    scale_degree -= 1;

    std::optional<NamingInformation> namei;
    std::optional<MIDIInformation> midii;

    if (scale_root.midi_.has_value())
    {
        midi_value scale_degree_midi_diff =
            scale_degree_to_midi_diff.at(scale_degree % note_names.size()) +
            static_cast<midi_value>((NOTES_PER_OCTAVE * (scale_degree / note_names.size())));
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
            scale_degree_without_accidentals_midi_offset_from_c += NOTES_PER_OCTAVE;
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
        throw std::invalid_argument(CREATION_NOT_BOTH_INFORMATION);
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
        if (!first) stream << NOTE_PRINT_SEPERATOR;

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

std::string& Note::get_name()
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
    for (auto&& naming_information : names_.value())
    {
        if (!first) stream << NOTE_PRINT_SEPERATOR;

        stream << get_name();

        stream << octave << " (" << midi_.value().midi_value_ << ')';

        first = false;
    }

    return stream.str();
}

std::string& Note::get_name_and_midi_string()
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
    std::string scale_degree_str;

    scale.clear();

    while (std::getline(stream, scale_degree_str, SCALE_DEGREE_SEPERATOR))
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
        if (!first) stream << SCALE_DEGREE_SEPERATOR << ' ';
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
    _notes = realise_scale(root, scale);
}

std::ostream& operator<<(std::ostream& stream, const RealisedScale& scale)
{
    bool first = true;
    for (auto&& note : scale._notes)
    {
        if (!first) stream << SCALE_DEGREE_SEPERATOR << ' ';
        first = false;
        stream << note.get_name();
    }
    return stream;
}

// ====REALISEDSCALE====
