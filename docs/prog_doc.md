# Overall structure

The project is broken up into two primary sections:

1. The ```musiclibrary.hpp``` and ```musiclibrary.cpp``` that are responsible for providing an interface and library for the music-related parts of this project. This can act as a standalone library outside of the context of this project. It is limited to just handling Notes and Scales without any kind of other information. If you are for some reason here looking for *complete* C++ music library, then you are sorely mistaken, my friend.
2. The rest of the project, dealing with handling the logic for this specific application.

# The music library

Most of the design decisions are documented in the code, so read through that for more details if you find them missing here. 

The library consists of a Note class, a Scale class, and a RealisedScale class.

A Note can correspond to either a given MIDI value (i.e. a pitch with MIDI value 60), or a given musical note name ('C'), or both ('C4'). This is all handled at runtime, trying to somewhat mirror the way ```music21``` works.

The most nasty musical-related things are handled in the constructor for Note using a scale_root Note reference and a scale degree and accidentals value, as pitch spelling has to be handled here to match the way musical scales are notated.

Accidentals are internally represented as an +/- short value that says how many half steps the note is sharper or flatter than the base scale degree.

A Scale represents an abstract scale without a given root, so it just holds a vector of scale degrees tied to accidentals values (i.e. just a sequence in the style of 1, flat 2, flat 3, sharp 4, double sharp 5 etc.).

A RealisedScale is created by pairing a Scale with a Note that acts as its root, generating a sequence of Notes following pitch spelling rules from western common notation.

In reality, both Scale and RealisedScale are just wrappers over an std::vector.

# The application logic

The application logic is handled by ApplicationManager from ```applicationmanager.hpp```, which handles the reading of files, generation of questions, correctly outputting questions to the console, reading user input, and saving the results.

The ApplicationManager contains an instance of ScaleManager, which is responsible for interacting with all the ````musiclibrary.hpp``` classes and the lower-level logic of how to correctly generate the questions.

# Input .csv format

The scales.csv file is where the list of available scales is stored. The format is as follows:

```
Name;Difficulty;Scale
Major;Easy;1,2,3,4,5,6,7
...
```

Each entry is in the form name, difficulty (a string that is either Easy, Medium, or Hard), and the scale, all seperated by ';'.

The scale is a comma-seperated list (without spaces) of entries in the form 
```{b's or #'s}{scale degree}```

You are not able to combine #s and bs in the accidentals (i.e. '#b3' is invalid). No negative values are allowed in the scale degree field. There is no limit on the amount of accidentals (only what you would find useful). There is no limitation that scale degrees have to be increasing (but it doesn't really make sense for that not to be the case).

# Output .csv format

The results.csv file is where the outputs of session are stored.

The format is:

```
Name;Difficulty;Correctness
Eb Harmonic Minor;0;CORRECT
...
```

The name here now includes the name of the root note + the scale name. The difficulty is saved as a value (0 for Easy, 1 for Medium, 2 for Hard). The correctness is stored as either 'CORRECT' for correct answers or 'INCORRECT' for incorrect answers.

This format was chosen as it was personally the most convenient to process in Microsoft Excel.

# Technical documentation
I have tried to make the doxygen comments/documentation and code contain comments explaining the design decisions as explanatory as I could; I find that reading them in that format is better for understanding the design than in a document like this one.

If in doubt, look at the code. I am not doing anything groundbreaking, so the code should be self-explanatory.