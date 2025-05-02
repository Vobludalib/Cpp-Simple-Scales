#ifndef CONSTANTS
#define CONSTANTS

// Exception-related
constexpr char FORGOT_TO_LOAD_SCALES[] = "No scales found while generating session!";
constexpr char TOO_MANY_QUESTION_PRINTS[] =
    "Tried printing next question when there are none left!";
constexpr char BAD_FILE_OPEN[] = "Unable to open/write the file!";
constexpr char INVALID_DIFFICULTY[] =
    "Invalid difficulty value found during parsing file! Row: {}, Column: {}";
constexpr char TOO_MANY_SAMPLES[] = "Too many samples requested!";
constexpr char NOT_ENOUGH_COLUMNS[] = "Didn't read the expected three columns on Row: {}";
constexpr char FAILED_PARSING_SCALE[] = "Failed parsing the scale on Row: {}";

// CSV-related
constexpr char CORRECT[] = "CORRECT";
constexpr char INCORRECT[] = "INCORRECT";
constexpr char CSV_SEPERATOR = ';';

#endif