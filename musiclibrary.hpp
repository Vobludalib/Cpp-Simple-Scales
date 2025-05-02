#ifndef MUSICLIBRARY
#define MUSICLIBRARY

#include <algorithm>
#include <array>
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
constexpr char INVALID_NOTE_NAME_FOUND[] =
    "Note name root passed does not appear in the list of valid note name roots!";
constexpr char NO_SCALE_DEGREE[] = "No scale degree passed in string!";
constexpr char BAD_SCALE_DEGREE_INDEX[] = "Scale degree is not a valid index!";

constexpr char INDEX_BASE_ERROR[] = "No such thing as as a 0th scale degree. Use 1-based indexing.";

// Type aliases
using midi_value = int;
using accidentals_value = short;
using scale_degree_value = size_t;

// Music-related constants
constexpr midi_value MIDDLE_C_MIDI = 60;
constexpr int MIDDLE_C_OCTAVE = 4;
constexpr midi_value NOTES_PER_OCTAVE = 12;
constexpr scale_degree_value NUMBER_OF_SCALE_DEGREES = 7;

// Output-related constants
constexpr char NOTE_PRINT_SEPERATOR = '/';
constexpr char SCALE_DEGREE_SEPERATOR = ',';

// I use this method of setting the specific constants based on the style. This is a compile-time
// change, and not something for users to mess with, but for the developers to change based on their
// needs
#if !defined(GERMAN_NAMING) && !defined(FRENCH_NAMING)
#define ENGLISH_NAMING
#endif

// Setting accidental style
#if defined(GERMAN_NAMING) || defined(ENGLISH_NAMING)
constexpr char downward_accidental[] = "b";
const char upward_accidental[] = "#";
#endif

#if defined(FRENCH_NAMING)
constexpr char downward_accidental[] = " bemol";
constexpr char upward_accidental[] = " diese";
#endif

// ====NOTE====

/**
 * @brief Class representing a musical note
 *
 * The Note contains either MIDI information or name information(s) (or both).
 * This is because a musical note can either represent a specific pitch (MIDI value 60) or an
 * 'abstract' concept of a given note (a note C, or C4).
 * A single Note can have multiple names, due to enharmonics. This is especially important to note
 * when creating/setting the note using a MIDI value.
 * This ability to (not) have either piece of information is dealt with by having both of these
 * pieces of information be std::optional<>, with runtime-checks used to ensure that nothing fails.
 * This could have been dealt with using inheritance, but it would struggle from expandability
 * issues, as every possible combination would have to be treated; if we wanted to add timing
 * information in a more complex library, we would run into issues.
 *
 * Much of this approach is somewhat un-C++-ish, but it resembles the way the Python music21 library
 * operates, which is currently a gold standard for symbolic music manipulation, and so I opted to
 * loosely follow their ideas.
 */
class Note
{
   private:
    // Setting note names style
#ifdef ENGLISH_NAMING
    inline static const std::array<std::string, NUMBER_OF_SCALE_DEGREES> note_names = {
        "C", "D", "E", "F", "G", "A", "B"};
#endif

#ifdef GERMAN_NAMING
    inline static const std::array<std::string, NUMBER_OF_SCALE_DEGREES> note_names = {
        "C", "D", "E", "F", "G", "A", "H"};
#endif

#ifdef FRENCH_NAMING
    inline static const std::array<std::string, NUMBER_OF_SCALE_DEGREES> note_names = {
        "Do", "Re", "Mi", "Fa", "Sol", "La", "Si"};
#endif

    // The following are for internal use only, so use 0-based indexing
    // We need both, as I need both directions indexed for different purposes

    /**
     * @brief Mapping from a given MIDI offset from the scale root to what scale degree (and
     * accidentals) a note is
     *
     */
    inline static const std::multimap<midi_value, std::tuple<midi_value, accidentals_value>>
        scale_midi_offset_to_scale_degree_and_accidental{
            {0, {0, 0}}, {1, {0, 1}},  {1, {1, -1}}, {2, {1, 0}},  {3, {1, 1}},   {3, {2, -1}},
            {4, {2, 0}}, {5, {3, 0}},  {6, {3, 1}},  {6, {4, -1}}, {7, {4, 0}},  // Perfect fifth
            {8, {4, 1}}, {8, {5, -1}}, {9, {5, 0}},  {10, {5, 1}}, {10, {6, -1}}, {11, {6, 0}}};

    /**
     * @brief Mapping from a given scale degree (without accidentals) to the MIDI offset from the
     * scale root
     *
     */
    inline static const std::map<scale_degree_value, midi_value> scale_degree_to_midi_diff{
        {0, 0}, {1, 2}, {2, 4}, {3, 5}, {4, 7}, {5, 9}, {6, 11}};

    /**
     * @brief Regex pattern for parsing a Note string (e.g. 'Db5')
     *
     */
    inline static const char note_name_regex_pattern[] = "([a|c-zA-Z]*)(b*)(#*)([\\-|\\d]*)";
    inline static const std::regex note_name_regex{note_name_regex_pattern};

    /**
     * @brief Struct holding information about the note name (i.e. what 'letter' and accidental)
     *
     */
    struct NamingInformation
    {
        // _base_degree here is an index to the note_names std::arrat object, and so uses 0-based
        // indexing
        scale_degree_value _base_degree;
        accidentals_value _accidentals;

        NamingInformation(scale_degree_value base_degree, accidentals_value accidentals);
    };

    /**
     * @brief Struct holding MIDI information about the note
     *
     */
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

    /**
     * @brief Parses a given MIDI value and generates all possible names for the note
     *
     * This only covers enharmonics up to one accidental (F#/Gb, but not Abbb or E##)
     *
     * @param midi
     * @return std::vector<NamingInformation>
     */
    std::vector<NamingInformation> generate_naming_information_from_midi(midi_value midi);

    /**
     * @brief Parses a note name string and generates the NameInformation object (and possible
     * MIDIInformation)
     *
     * The MIDIInformation is only generated if the note is in the form "C{octave within MIDI
     * range}", and not just "C".
     *
     * @param name
     * @return std::tuple<NamingInformation, std::optional<MIDIInformation>>
     */
    std::tuple<NamingInformation, std::optional<MIDIInformation>>
    generate_naming_and_midi_from_string(const std::string& name);

    /**
     * @brief Generates NameInformation and MIDIInformation (under certain conditions) based off a
     * scale root, a scale degree, and accidental number.
     *
     * NamingInformation only gets generated if scale_root (which we base calculation off) contains
     * one NameInformation struct. If scale_root has no NameInformation, we do not know what to base
     * it off of. If scale_root has multiple possible names (i.e. unclarified enharmonic
     * possibilites), we do not know which way to do pitch spelling.
     *
     * MIDIInformation is only generated if scale_root has MIDIInformation we can work from.
     *
     * @param scale_root
     * @param scale_degree
     * @param accidentals
     * @return std::tuple<std::optional<NamingInformation>, std::optional<MIDIInformation>>
     */
    std::tuple<std::optional<NamingInformation>, std::optional<MIDIInformation>>
    generate_naming_and_midi_from_root_and_scale_degree(const Note& scale_root,
                                                        scale_degree_value scale_degree,
                                                        accidentals_value accidentals);

    /**
     * @brief Runtime check if Note contains MIDIInformation.
     *
     * @return true
     * @return false
     */
    inline bool check_has_midi() const { return midi_.has_value(); }

    /**
     * @brief Runtime check if Note contains at least one NameInformation.
     *
     * @return true
     * @return false
     */
    inline bool check_has_name() const
    {
        return !(!names_.has_value() || names_.value().size() == 0);
    }

    /**
     * @brief Variable used to inform caching of the generated name.
     *
     */
    bool have_to_regenerate_name = true;

    /**
     * @brief Used to cache the generated name (no MIDI information).
     *
     */
    std::string name_as_string;

    /**
     * @brief Used to generate a 'simple' name (i.e. no MIDI information).
     *
     * @return std::string
     */
    std::string generate_name_as_string() const;

    /**
     * @brief Used to cache the generated complex name (Name and MIDI information)
     *
     */
    std::string complex_name;

    /**
     * @brief Used to generate a 'complex' name (both Name and MIDI information where relevant).
     *
     * @return std::string
     */
    std::string generate_complex_name_as_string() const;

   public:
    // Every constructor may create a note with or without both types of information!

    /**
     * @brief Construct a new Note object - becomes middle C with both MIDI and name information
     *
     */
    Note();

    /**
     * @brief Construct a new Note object that represents a given MIDI value
     *
     * Possible names are generated if generate_names is enabled.
     * Multiple names can be generated due to enharmonics (enharmonics only up to one accidental are
     * created)
     *
     * @param midi
     * @param generate_names
     */
    Note(midi_value midi, bool generate_names = true);

    /**
     * @brief Set the note object to a given MIDI value
     *
     * Possible names are generated if generate_names is enabled.
     * Multiple names can be generated due to enharmonics (enharmonics only up to one accidental are
     * created)
     *
     * @param midi
     * @param generate_names
     */
    void set_note(midi_value midi, bool generate_names = true);

    /**
     * @brief Construct a new Note object from a string. Will contain name information.
     *
     * If a number within the valid octave range is present, then MIDI information is also
     * generated
     *
     * @param name
     */

    Note(const std::string& name);

    /**
     * @brief Set a Note object from a string. Will contain name information.
     *
     * If a number within the valid octave range is present, then MIDI information is also
     * generated
     *
     * @param name
     */
    void set_note(const std::string& name);

    /**
     * @brief Construct a new Note object representing a specific scale degree based off the scale
     * root.
     *
     * If the scale_root has a MIDI value, then the resulting note will also have a MIDI value.
     * If the scale_root has a unique name, then the resulting note will also have a unique name.
     *
     * @param scale_root
     * @param scale_degree
     * @param accidentals
     */
    Note(const Note& scale_root, scale_degree_value scale_degree, accidentals_value accidentals);

    /**
     * @brief Set a Note object representing a specific scale degree based off the scale
     * root.
     *
     * If the scale_root has a MIDI value, then the resulting note will also have a MIDI value.
     * If the scale_root has a unique name, then the resulting note will also have a unique name.
     *
     * @param scale_root
     * @param scale_degree
     * @param accidentals
     */
    void set_note(const Note& scale_root, scale_degree_value scale_degree,
                  accidentals_value accidentals);

    // The default copying and moving constructors/assignment operators work just fine.

    /**
     * @brief Get the MIDI value of the MIDIInformation
     *
     * Throws an std::runtime_error exception if no MIDIInformation is present.
     *
     * @return midi_value
     */
    inline midi_value get_midi() const
    {
        if (midi_.has_value())
            return midi_.value().midi_value_;
        else
            throw std::runtime_error(NO_MIDI_INFORMATION);
    }

    /**
     * @brief Get the 'simple' (no MIDI information) name as a string. Performs caching.
     *
     * @return std::string&
     */
    std::string& get_name();

    /**
     * @brief Get the 'simple' (no MIDI information) name as a string. Does not perform caching.
     *
     * @return std::string
     */
    std::string get_name() const;

    /**
     * @brief Get the 'complex' (name and MIDI information where relevant) name as a string.
     * Performs caching.
     *
     * @return std::string&
     */
    std::string& get_name_and_midi_string();

    /**
     * @brief Get the 'complex' (name and MIDI information where relevant) name as a string.
     * Does not perform caching.
     *
     * @return std::string&
     */
    std::string get_name_and_midi_string() const;

    /**
     * @brief Destroy the Note object
     *
     */
    ~Note() = default;

    /**
     * @brief Printing to stream operator.
     *
     * Tries to print the 'complex' name if possible, otherwise prints what is possible to print.
     *
     * @param stream
     * @param note
     * @return std::ostream&
     */
    friend std::ostream& operator<<(std::ostream& stream, const Note& note);
};

// ====NOTE====
// ====SCALE====

/**
 * @brief Class representing an 'abstract' musical scale.alignas
 *
 * Supports scales that go past an octave (i.e. 9ths and onwards).
 * This represents the idea of a scale (e.g. 'Major'); for a specific scale (e.g. 'C Major'), see
 * RealisedScale.
 *
 * Made to work as a wrapper over a vector and tries to forward as much as possible from the vector
 * such as iterators etc. More could be added here, but this is sufficient for now.
 *
 */
class Scale
{
   private:
    // Type aliasing
    using scale_degree = std::pair<scale_degree_value, accidentals_value>;

    /**
     * @brief Regex pattern for parsing a scale degree (e.g. 'b3' or '#6').
     *
     */
    inline static const char scale_degree_regex_pattern[] = "(b*)(#*)([\\-|\\d]*)";
    inline static const std::regex scale_degree_regex{scale_degree_regex_pattern};

    /**
     * @brief The underlying std::vector container used to store the scale degrees.
     *
     */
    std::vector<scale_degree> _scale_degrees;

    /**
     * @brief Parsing method for turning a scale degree string (e.g. 'b3' or '#6') into a
     * scale_degree.
     *
     * @param input
     * @return scale_degree
     */
    static scale_degree parse_scale_degree_string(const std::string& input);

   public:
    /**
     * @brief Construct a new empty Scale object
     *
     */
    Scale() = default;

    /**
     * @brief Construct a new Scale object from an input stream using the >> operator.
     *
     * @param stream
     */
    inline Scale(std::istream& stream) { stream >> *this; }

    /**
     * @brief Construct a new Scale object by copying an existing std::vector<scale_degree>
     *
     * @param degrees
     */
    inline Scale(const std::vector<scale_degree>& degrees) : _scale_degrees(degrees) {};

    /**
     * @brief Construct a new Scale object by stealing an existing std::vector<scale_degree>
     *
     * @param degrees
     */
    inline Scale(std::vector<scale_degree>&& degrees) noexcept
        : _scale_degrees(std::move(degrees)) {};

    // The default copying and moving constructors/assignment operators work just fine.

    /**
     * @brief Operator for parsing a csv input stream into a scale object.
     *
     * @param stream
     * @param scale
     * @return std::istream&
     */
    friend std::istream& operator>>(std::istream& stream, Scale& scale);

    /**
     * @brief Operator for writing a string representation of a scale to an output stream.
     *
     * @param stream
     * @param scale
     * @return std::ostream&
     */
    friend std::ostream& operator<<(std::ostream& stream, const Scale& scale);

    /**
     * @brief Clears the underlying std::vector.
     *
     */
    inline void clear() { return _scale_degrees.clear(); }

    /**
     * @brief Returns the amount of scale degrees.
     *
     * @return size_t
     */
    inline size_t size() { return _scale_degrees.size(); }

    /**
     * @brief Retrieves the .begin() iterator of the underlying std::vector.
     *
     * @return auto
     */
    inline auto begin() { return _scale_degrees.begin(); }

    /**
     * @brief Retrieves the .end() iterator of the underlying std::vector.
     *
     * @return auto
     */
    inline auto end() { return _scale_degrees.end(); }

    /**
     * @brief Retrieves the const .begin() iterator of the underlying std::vector.
     *
     * @return auto
     */
    inline auto begin() const { return _scale_degrees.begin(); }

    /**
     * @brief Retrieves the const .end() iterator of the underlying std::vector.
     *
     * @return auto
     */
    inline auto end() const { return _scale_degrees.end(); }

    /**
     * @brief Retrieves the const .cbegin() iterator of the underlying std::vector.
     *
     * @return auto
     */
    inline auto cbegin() const { return _scale_degrees.cbegin(); }

    /**
     * @brief Retrieves the const .cend() iterator of the underlying std::vector.
     *
     * @return auto
     */
    inline auto cend() const { return _scale_degrees.cend(); }

    /**
     * @brief Retrieves the index'th element of the underlying std::vector.
     *
     * @param index
     * @return scale_degree&
     */
    inline scale_degree& operator[](size_t index) { return _scale_degrees[index]; }

    /**
     * @brief Retrieves the const index'th element of the underlying std::vector.
     *
     * @param index
     * @return scale_degree&
     */
    inline const scale_degree& operator[](size_t index) const { return _scale_degrees[index]; }
};

// ====SCALE====
// ====REALISEDSCALE====

/**
 * @brief Class representing a realised scale (e.g. 'C Major')
 *
 */
class RealisedScale
{
   private:
    /** Underlying container holding the Note objects */
    std::vector<Note> _notes;

    /**
     * @brief Method for generating the whole list of Notes in the scale for a given root note and
     * scale.
     *
     * @param root
     * @param scale
     * @return std::vector<Note>
     */
    std::vector<Note> realise_scale(const Note& root, const Scale& scale);

   public:
    /**
     * @brief Construct a new Realised Scale object
     *
     */
    RealisedScale() = default;

    /**
     * @brief Construct a new Realised Scale object from a root note and scale.
     *
     * @param root
     * @param scale
     */
    RealisedScale(const Note& root, const Scale& scale);

    /**
     * @brief Get the root note (1st note in the scale)
     *
     * Technically, you could create a scale that does not contain the tonic; in that case this
     * method will return the 'incorrect' value.
     * In that case though, you're already doing something very very strange and illogical, so tough
     * luck buddy.
     *
     * @return const Note&
     */
    inline const Note& get_root() const { return _notes[0]; }

    /**
     * @brief Operator for printing a realised scale into an output stream.
     *
     * @param stream
     * @param scale
     * @return std::ostream&
     */
    friend std::ostream& operator<<(std::ostream& stream, const RealisedScale& scale);

    /**
     * @brief Clears the underlying std::vector.
     *
     */
    inline void clear() { return _notes.clear(); }

    /**
     * @brief Returns the amount of scale degrees.
     *
     * @return size_t
     */
    inline size_t size() { return _notes.size(); }

    /**
     * @brief Retrieves the .begin() iterator of the underlying std::vector.
     *
     * @return auto
     */
    inline auto begin() { return _notes.begin(); }

    /**
     * @brief Retrieves the .end() iterator of the underlying std::vector.
     *
     * @return auto
     */
    inline auto end() { return _notes.end(); }

    /**
     * @brief Retrieves the const .begin() iterator of the underlying std::vector.
     *
     * @return auto
     */
    inline auto begin() const { return _notes.begin(); }

    /**
     * @brief Retrieves the const .end() iterator of the underlying std::vector.
     *
     * @return auto
     */
    inline auto end() const { return _notes.end(); }

    /**
     * @brief Retrieves the const .cbegin() iterator of the underlying std::vector.
     *
     * @return auto
     */
    inline auto cbegin() const { return _notes.cbegin(); }

    /**
     * @brief Retrieves the const .cend() iterator of the underlying std::vector.
     *
     * @return auto
     */
    inline auto cend() const { return _notes.cend(); }

    /**
     * @brief Retrieves the index'th element of the underlying std::vector.
     *
     * @param index
     * @return scale_degree&
     */
    inline Note& operator[](size_t index) { return _notes[index]; }

    /**
     * @brief Retrieves the const index'th element of the underlying std::vector.
     *
     * @param index
     * @return scale_degree&
     */
    inline const Note& operator[](size_t index) const { return _notes[index]; }
};

// ====REALISEDSCALE====

#endif