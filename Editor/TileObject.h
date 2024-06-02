#pragma once
#include <SFML/Graphics.hpp>


struct TileObject{

    sf::Texture* texture;
    sf::Image* image_texture;
    sf::Sprite sprite_texture; // references texture

    // for tile objects we only have 1 layer
    int layer_width;
    int layer_height;

    int tile_height;
    int tile_width;

    int texture_atlas_position_x = 0;
    int texture_atlas_position_y = 0;
};