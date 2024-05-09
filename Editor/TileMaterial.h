#pragma once
#include <SFML/Graphics.hpp>

struct TileMaterial{

    bool round_edges;    
    sf::Texture texture; // DEPRECATED: Should be using image_texture variable
    sf::Image image_texture;

};
