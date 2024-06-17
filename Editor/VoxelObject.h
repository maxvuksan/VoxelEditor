#pragma once
#include <SFML/Graphics.hpp>

/*
    a 3d material, made up of pixel layers (which are interpreted as the z axis of the object)

    when a voxel image is loaded we read from the bottom where the schematic is located, at x position 0, 
    we travel up the y axis ^ until we encounter a non-white pixel, this defines height of the object (tells us how high each layer is, 
    the width can then just be deduced from the width of the image)
*/
struct VoxelObject{

    sf::Texture* texture = nullptr;
    sf::Image* image_texture = nullptr;
    
    sf::Sprite sprite_texture; // references texture

    int layer_height = 0; 
    int layer_width = 0;

    int tile_height = 0;
    int tile_width = 0;

    bool draw_sides = true;

    // excluding schematic 
    int layer_count = 0;

    int texture_atlas_position_x = 0;
    int texture_atlas_position_y = 0;
    

};