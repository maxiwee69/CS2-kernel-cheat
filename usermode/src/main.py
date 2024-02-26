import os
import re

# Define the pattern for the audio data lines
pattern = re.compile(r'std::vector<sf::Int16> audioData\d+ = \{.*?\};', re.DOTALL)

# Define the names of the hitsound files
hitsound_files = ['hitsound.wav_1.cpp', 'hitsound.wav_2.cpp', 'hitsound.wav_3.cpp', 'hitsound.wav_4.cpp', 'hitsound.wav_5.cpp']

# Open the sounds.cpp file in write mode
with open('sounds.cpp', 'w') as sounds_file:
    # Write the includes at the top of the file
    sounds_file.write('#include <vector>\n')
    sounds_file.write('#include <SFML/Audio.hpp>\n')

    # Loop over the hitsound files
    for hitsound_file in hitsound_files:
        # Open the hitsound file in read mode
        with open(hitsound_file, 'r') as file:
            # Read the contents of the file
            contents = file.read()

            # Search for the audio data line
            match = pattern.search(contents)

            # If the audio data line was found, write it to the sounds.cpp file
            if match:
                sounds_file.write(match.group() + '\n')