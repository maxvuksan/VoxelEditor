
#pragma once
#include <SFML/Graphics.hpp>
#include <math.h>
#include "Face.h"
#include "../System/Utility/Calc.h"
#include "ScreenData.h"
#include "Renderer.h"
#include "VoxelObject.h"
#include "FaceDirection.h"



/*
    Utility functions for the Renderer
*/
class Util{


    public:

        // @returns the position symmetric to the provided x position
        static int GetSymmetricPositionX(ScreenData& screen_data, int x){
            
            // passed halfway
            if(x >= screen_data.m_half_tile_width){
                return screen_data.m_half_tile_width - (x - screen_data.m_half_tile_width) - 1;
            }
            else{
                return screen_data.m_tile_layers[0].size() - x - 1;
            }
        }

        // @returns the position symmetric to the provided x position
        static int GetSymmetricPositionY(ScreenData& screen_data, int y){

            // passed halfway
            if(y >= screen_data.m_half_tile_height){
                return screen_data.m_half_tile_height - (y - screen_data.m_half_tile_height) - 1;
            }
            else{
                return screen_data.m_tile_layers[0][0].size() - y - 1;
            }

        }

        static sf::Uint32 DepthFromColour(sf::Color colour){
            /*
                we are going to bound the depth range to 0-255 (anything greater will be interpreted as 255)
            */
    
           // scaling by 5 for visable results
            return colour.r / 5.0f;
        }

        static sf::Color ColourFromDepth(sf::Uint32 depth){
            return sf::Color(depth * 5, depth * 5, depth * 5);
        }

        /*
            @returns true if a tile should have its sides rendered
        */
        static bool TileSurrounding(int x, int y, FaceDirection direction, const ScreenData& screen_data, int tile_layer_index){

            switch(direction){
                case FACE_TOP:
                    y--;
                    // cull top 
                    if(y < screen_data.m_half_tile_height){
                        return true;
                    }
                    break;

                case FACE_LEFT:
                    x--;
                    // cull left 
                    if(x < screen_data.m_half_tile_width){
                        return true;
                    }
                    break;

                case FACE_BOTTOM:
                    y++;

                    // cull left 
                    if(y > screen_data.m_half_tile_height){
                        return true;
                    }
                    break;
                
                case FACE_RIGHT:

                    x++;
                    // cull left 
                    if(x > screen_data.m_half_tile_width){
                        return true;
                    }
                    break;    
            }

            // out of bounds
            if(x >= screen_data.m_tile_layers[tile_layer_index].size() || x < 0){
                return true;
            }
            if(y >= screen_data.m_tile_layers[tile_layer_index][0].size() || y < 0){
                return true;
            }

            return screen_data.m_tile_layers[tile_layer_index][x][y].occupied;


        }

        static bool VoxelSurrounding(int x, int y, int z, FaceDirection direction, const ScreenData& screen_data){

            switch(direction){
                case FACE_TOP:
                    y--;
                    // cull top 
                    if(y < screen_data.m_half_canvas_height){
                        return true;
                    }
                    break;

                case FACE_LEFT:
                    x--;
                    // cull left 
                    if(x < screen_data.m_half_canvas_width){
                        return true;
                    }
                    break;

                case FACE_BOTTOM:
                    y++;

                    // cull left 
                    if(y > screen_data.m_half_canvas_height){
                        return true;
                    }
                    break;
                
                case FACE_RIGHT:

                    x++;
                    // cull left 
                    if(x > screen_data.m_half_canvas_width){
                        return true;
                    }
                    break;    
            }

            // out of bounds
            if(x >= screen_data.m_canvas_width || x < 0){
                return true;
            }
            if(y >= screen_data.m_canvas_height || y < 0){
                return true;
            }


            if(screen_data.m_voxel_space[z][x][y].occupied && screen_data.m_voxel_space[z][x][y].draw_sides){
                return true;
            }
            else{
                return false;
            }
        }


        static sf::Uint32 ConvertNormalColourToLightValue(ScreenData& screen_data, const sf::Color& normal_colour){

            if(normal_colour == sf::Color(127, 127, 255)){
                return 205; // flat light value
            }
            if(normal_colour == sf::Color::Green){
                return 255; // highlight colour
            }
            if(normal_colour == sf::Color::Blue){
                return 150; // dark colour
            }

            return 205;
        }


        static sf::Color ColourFromNormal(FaceDirection face_direction){

            switch(face_direction){

                case FACE_FRONT: 
                    return sf::Color(127, 127, 255);
                    break;
                
                case FACE_TOP:
                case FACE_LEFT: 
                    return sf::Color::Green;
                    break;

                case FACE_RIGHT:
                case FACE_BOTTOM: 
                    return sf::Color::Blue;
                    break;               

            }
            // spit out flat colour on bad input
            return sf::Color(127, 127, 255);
        }

        /*
            projects a 2D position into 3D space 
        */
        static sf::Vector2f ShiftVertexOnPerspectiveAxis(sf::Vector2f position, const ScreenData& screen_data, float z_position, bool _floor = true){
            
            float distance = Calc::Distance(position, sf::Vector2f(screen_data.m_half_canvas_width, screen_data.m_half_canvas_height));
            sf::Vector2f shift = Calc::VectorBetween(position, sf::Vector2f(screen_data.m_half_canvas_width, screen_data.m_half_canvas_height)) * distance * z_position;

            sf::Vector2f final = position + shift;

            if(_floor){
                final = sf::Vector2f(floor(final.x), floor(final.y));
            }
            else{
                final = sf::Vector2f(ceil(final.x), ceil(final.y));
            }

            return final;

        }

        static sf::Vector3f NormalFromFace(const Face& face){

            // cross product...

            sf::Vector3f AB(face.m_vert_positions[1].x - face.m_vert_positions[0].x, face.m_vert_positions[1].y - face.m_vert_positions[0].y, face.m_vert_positions[1].z - face.m_vert_positions[0].z);
            sf::Vector3f AC(face.m_vert_positions[2].x - face.m_vert_positions[0].x, face.m_vert_positions[2].y - face.m_vert_positions[0].y, face.m_vert_positions[2].z - face.m_vert_positions[0].z);

            float n_x = AB.y * AC.z - AB.z * AC.y;
            float n_y = AB.z * AC.x - AB.x * AC.z;
            float n_z = AB.x * AC.y - AB.y * AC.x;

            sf::Vector3f cross_product(n_x, n_y, n_z);

            float magnitude = sqrt(pow(n_x, 2) + pow(n_y, 2) + pow(n_z, 2));

            return cross_product / magnitude;
        }


        /*
            @returns true if a position is within a voxel
                    false otherwise
        */
        static bool PositionIsInsideVoxel(ScreenData& screen_data, sf::Vector3f position){

            int thresh_x = floor(position.x);
            int thresh_y = floor(position.y);
            int thresh_z = floor(position.z);


            for(unsigned int z = Calc::Clamp(thresh_z - 1, 0, screen_data.m_voxel_space.size() - 1); z < Calc::Clamp(thresh_z + 1, 0, screen_data.m_voxel_space.size()); z++){
                for(unsigned int x = Calc::Clamp(thresh_x - 1, 0, screen_data.m_canvas_width - 1); x < Calc::Clamp(thresh_x + 1, 0, screen_data.m_canvas_width); x++){
                    for(unsigned int y = Calc::Clamp(thresh_y - 1, 0, screen_data.m_canvas_height - 1); y < Calc::Clamp(thresh_y + 1, 0, screen_data.m_canvas_height); y++){


                        // black is no voxel
                        if(screen_data.m_voxel_space[z][x][y].occupied){

                            // does position fall in voxel
                            if(x <= position.x && (x + 1) >= position.x){
                                if(y <= position.y && (y + 1) >= position.y){
                                    if(z <= position.z && (z + 1) >= position.z){

                                        return true;
                                    }
                                }
                            }
                        }

                    }
                }
                
            }
            return false;

        }

        /*
            DEPRECATED: Should used sf::Image instead of sf::Texture

            wraps a position around a textures dimensions
            @param edge_overflows determines what happens if a position is equal to the edge of a texture

            say a texture is 16x16 pixels,
            and our position_x is 16, does this position stay at 16, or wraps around to 0 (false/true)
        */
        static sf::Vector2f PositionToTexturePosition(sf::Vector2i position, const sf::Texture& texture, bool edge_overflows_x, bool edge_overflows_y){
            
            sf::Vector2f new_position;

            new_position.x = position.x % texture.getSize().x;
            new_position.y = position.y % texture.getSize().y;

            new_position.x = texture.getSize().x - new_position.x;
            new_position.y = texture.getSize().y - new_position.y;
                 

            if(position.x % texture.getSize().x == 0 && !edge_overflows_x){
                new_position.x = 0;//texture.getSize().x;
            }
            if(position.y % texture.getSize().y == 0 && !edge_overflows_y){
                new_position.y = 0;//texture.getSize().y;
            }

            return new_position;
        }
        static sf::Vector2i PositionToTexturePosition(sf::Vector2i position, const sf::Image& texture, bool edge_overflows_x, bool edge_overflows_y){
            
            sf::Vector2i new_position;

            new_position.x = position.x % texture.getSize().x;
            new_position.y = position.y % texture.getSize().y;


            if(position.x % texture.getSize().x == 0){
                new_position.x = 0;//texture.getSize().x;
            }
            if(position.y % texture.getSize().y == 0){
                new_position.y = 0;//texture.getSize().y;
            }

            return new_position;
        }
        // @returns a colour from a hand picked pool of random colours
        static sf::Color GetColourFromColourLoop(int index){

            index %= colour_loop.size();
            return colour_loop[index];
        }

        /*
            @returns true if a voxel material has enough space to be placed

            @param check_surrounding_tiles if false, a object is always valid when in the bounds of the tile space
        */
        static bool ValidateVoxelObjectFitsTileGrid(ScreenData& screen_data, int tile_x, int tile_y, int tile_layer, int object_width, int object_height, bool check_surrounding_tiles){

            for(int x = 0; x < object_width; x++){

                // out of tilemap bounds
                if(tile_x + x >= screen_data.m_tile_layers[tile_layer].size()){
                    return false;
                }

                for(int y = 0; y < object_height; y++){

                    // out of tilemap bounds
                    if(tile_y + y >= screen_data.m_tile_layers[tile_layer][0].size()){
                        return false;
                    }

                    // no tile in place
                    if(!screen_data.m_tile_layers[tile_layer][tile_x + x][tile_y + y].occupied && check_surrounding_tiles){
                        return false;
                    }

                }

            }
            // passed
            return true;
        }

    private:

        /*
            a vector of random colours to distiguish UI elements (hand picked not generated)
        */
        static std::vector<sf::Color> colour_loop;

};