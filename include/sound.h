#ifndef SOUND_H
#define SOUND_H

#include <SFML/Audio.hpp>
#include <iostream>
#define SOUND_DIR "../assets/sounds/"

bool loadSound(sf::Sound& sound, sf::SoundBuffer& soundBuffer, const std::string& path){
    bool isLoaded = soundBuffer.loadFromFile(SOUND_DIR + path);
    sound.setBuffer(soundBuffer);
    return isLoaded;
}

void playSound(sf::Sound& sound , int vol = 35 , bool loop = true) {
    if (!sound.getBuffer()) {
        std::cerr << "Sound not available.\n";  // Optional fallback behavior
    }
    sound.setVolume(vol);
    sound.setLoop(loop);
    sound.play();
}

void stopSound(sf::Sound& sound) {
    if (!sound.getBuffer()){
        std::cerr << "Sound not available.\n";
    }
    sound.stop();
}
#endif
