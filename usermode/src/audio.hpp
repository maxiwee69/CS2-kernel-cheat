#include <SFML/Audio.hpp>

extern sf::Sound sound;  // Declare the sound object
void playSound(const std::vector<sf::Int16>& soundData);
std::vector<sf::Int16>& getRandomSoundFile();