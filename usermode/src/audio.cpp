#include "audio.hpp"
#include "sounds.hpp"
#include <iostream>
#include <random>
#include <SFML/Audio.hpp>

sf::Sound sound;

std::vector<sf::Int16>& getRandomSoundFile() {
    std::vector<std::vector<sf::Int16>*> sound_files = { &audioData1, &audioData2, &audioData4, &audioData5 };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, sound_files.size() - 1);

    return *sound_files[distr(gen)];
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