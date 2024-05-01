#pragma once
#include <SFML/Graphics.hpp>

/*
    a 3d material, made up of pixel layers (which are interpreted as the z axis of the object)

    when a voxel image is loaded we read from the bottom where the schematic is located, at x position 0, 
    we travel up the y axis ^ until we encounter a red pixel, this defines height of the object (tells us how high each layer is, 
    the width can then just be deduced from the width of the image)
*/
struct VoxelMaterial{

    sf::Texture texture;
    int tile_width; // texture size / tile size
    int tile_height; 
};