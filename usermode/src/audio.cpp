#include <sodium.h>
#include "audio.hpp"
#include "sounds.hpp"
#include <iostream>
#include <SFML/Audio.hpp>

sf::Sound sound;

std::vector<sf::Int16>& getRandomSoundFile() {
    std::vector<std::vector<sf::Int16>*> sound_files = { &audioData1, &audioData2, &audioData4, &audioData5 };

    uint32_t index;
    randombytes_buf(&index, sizeof(index));
    index %= sound_files.size(); // to get a number in the range [0, sound_files.size() - 1]

    return *sound_files[index];
}

void playSound(const std::vector<sf::Int16>& soundData) {
    sf::SoundBuffer buffer;
    if (!buffer.loadFromSamples(&soundData[0], soundData.size(), 2, 44100)) {
        std::cout << "Failed to load samples\n";
        return;
    }

    // Use the global sound object
    sound.setBuffer(buffer);
    sound.play();

    // Check the status of the sound
    if (sound.getStatus() != sf::Sound::Playing) {
        std::cout << "Sound is not playing\n";
    }

    // Wait for the sound to finish playing
    while (sound.getStatus() == sf::Sound::Playing) {
        sf::sleep(sf::milliseconds(100));
    }
}