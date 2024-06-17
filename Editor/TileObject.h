#pragma once
#include <SFML/Graphics.hpp>


struct TileObject{

    sf::Texture* texture = nullptr;
    sf::Image* image_texture = nullptr;
    sf::Sprite sprite_texture; // references texture

    // for tile objects we only have 1 layer
    int layer_width = 0;
    int layer_height = 0;

    int tile_height = 0;
    int tile_width = 0;

    int texture_atlas_position_x = 0;
    int texture_atlas_position_y = 0;
};