#pragma once
#include <SFML/Graphics.hpp>

struct TileMaterial{

    bool round_edges = false;    
    
    sf::Texture* texture = nullptr; 
    sf::Image* image_texture = nullptr;

};
