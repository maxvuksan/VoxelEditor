
#pragma once
#include <SFML/Graphics.hpp>
#include <math.h>
#include "Face.h"
#include "../System/Utility/Calc.h"
#include "ScreenData.h"
#include "Renderer.h"

/*
    Utility functions for the Renderer
*/
class Util{

    public:

        static sf::Uint32 DepthFromColour(sf::Color colour){
            /*
                we are going to bound the depth range to 0-255 (anything greater will be interpreted as 255)
            */
    
            return colour.r;
        }

        static sf::Color ColourFromDepth(sf::Uint32 depth){
            return sf::Color(depth, depth, depth);
        }

        /*
            @returns true if a voxel should have its sides rendered
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


        static sf::Uint32 ConvertNormalColourToLightValue(ScreenData& screen_data, const sf::Color& normal_colour){

            float x = normal_colour.r / 255.0f;
            float y = normal_colour.g / 255.0f;
            float z = normal_colour.b / 255.0f;

            x -= 0.5f;
            y -= 0.5f;
            z -= 0.5f;

            sf::Vector3f normal_vector(x, y, z);
            normal_vector = Calc::Normalize(normal_vector);

            float dot_product = Calc::DotProduct(normal_vector, screen_data.m_flat_light_direction);
            
            dot_product += 0.5f;
            dot_product *= 0.5f;

            sf::Uint32 flat_light_value =  round(dot_product * 255);
            return flat_light_value;
        }


        static sf::Color ColourFromNormal(sf::Vector3f normal){

            normal.x += 1.0f;
            normal.y += 1.0f;
            normal.z += 1.0f;

            normal /= 2.0f;

            return sf::Color(normal.x * 255, normal.y * 255, normal.z * 255);
        }

        /*
            projects a 2D position into 3D space 
        */
        static sf::Vector2f ShiftVertexOnPerspectiveAxis(sf::Vector2f position, const ScreenData& screen_data, float z_position){
            
            float distance = Calc::Distance(position, sf::Vector2f(screen_data.m_half_canvas_width, screen_data.m_half_canvas_height));
            sf::Vector2f shift = Calc::VectorBetween(position, sf::Vector2f(screen_data.m_half_canvas_width, screen_data.m_half_canvas_height)) * distance * z_position;
            return  position + shift;
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

            int tile_map_width = screen_data.m_tile_layers[0].size();
            int tile_map_height = screen_data.m_tile_layers[0][0].size();

            int start_x = Calc::Clamp(floor(position.x / screen_data.m_tile_size), 0, tile_map_width);
            int start_y = Calc::Clamp(floor(position.y / screen_data.m_tile_size), 0, tile_map_height);


            for(int i = screen_data.m_tile_layers.size() - 1; i >= 0; i--){

                for(unsigned int x = start_x; x < start_x + 1; x++){
                    for(unsigned int y = start_y; y < start_y + 1; y++){

                        if(screen_data.m_tile_layers[i][x][y].occupied){
                            // does position fall in voxel
                            if(screen_data.m_tile_size * x < position.x && screen_data.m_tile_size * (x + 1) > position.x){
                                if(screen_data.m_tile_size * y < position.y && screen_data.m_tile_size * (y + 1) > position.y){
                                    if(screen_data.m_tile_depth * i < position.z && screen_data.m_tile_depth * (i + 1) > position.z){

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

        // @returns a colour from a hand picked pool of random colours
        static sf::Color GetColourFromColourLoop(int index){

            index %= colour_loop.size();
            return colour_loop[index];
        }

    private:

        /*
            a vector of random colours to distiguish UI elements (hand picked not generated)
        */
        static std::vector<sf::Color> colour_loop;

};