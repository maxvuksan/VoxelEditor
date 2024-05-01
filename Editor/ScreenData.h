#pragma once
#include <vector>
#include <iostream>
////////////////////////////////////

#include "../System/include.h"

#include "Rect.h"
#include "TileMaterial.h"

struct TileData{
    bool occupied = false;
    int tile_material_index = 0;
};


/*
    all data to be renderered 
*/
class ScreenData {

    public:

        sf::Image m_canvas_image;
        sf::RenderTexture m_canvas;
        sf::RenderTexture m_canvas_normals;
        sf::RenderTexture m_canvas_depth;
        sf::RenderTexture m_canvas_shadow_map; // black is shadow, white is light

        // for anything that wont show up in final scene (UI helpers etc...)
        sf::RenderTexture m_general_surface;

        sf::Vector3f m_flat_light_direction = sf::Vector3f(0.5,2.0,3.0);
        sf::Uint32 m_shadow_lift = 0;

        int m_tile_size;
        int m_tile_depth;
        float m_half_tile_size;

        int m_canvas_width;
        int m_canvas_height;        

        float m_half_canvas_width;
        float m_half_canvas_height;     

        float m_half_tile_width;
        float m_half_tile_height;    

        std::vector< std::vector<std::vector<TileData>> > m_tile_layers; // the closest to the camera

        std::vector<Rect> m_rectangles;



        /*
            define the size of our workspace

            @param width_in_tiles number of tile columns
            @param height_in_tiles number of tile rows
            @param tile_size the width & height of each tile (tilesize * tilesize)
            @param tile_depth the depth of each tile (Z-axis)
        */
        void Create(int width_in_tiles, int height_in_tiles, int tile_size, int tile_depth){
            m_canvas_width = width_in_tiles * tile_size;
            m_canvas_height = height_in_tiles * tile_size;

            m_canvas_image.create(m_canvas_width, m_canvas_height);

            m_tile_size = tile_size;
            m_tile_depth = tile_depth;
            m_half_tile_size = tile_size / 2.0f;

            m_tile_layers.resize(3);
            for(int i = 0; i < m_tile_layers.size(); i++){

                m_tile_layers[i].resize(width_in_tiles);
                for(int x = 0; x < width_in_tiles; x++){
                    
                    m_tile_layers[i][x].resize(height_in_tiles);
                    
                    for(int y = 0; y < height_in_tiles; y++){
                    
                        m_tile_layers[i][x][y].occupied = false;
                        m_tile_layers[i][x][y].tile_material_index = 0;
                    }
                }
            }

            m_half_canvas_width = m_canvas_width / 2.0f;
            m_half_canvas_height = m_canvas_height / 2.0f;

            m_half_tile_width = width_in_tiles / 2.0f;
            m_half_tile_height = height_in_tiles / 2.0f;


            m_canvas.create(m_canvas_width, m_canvas_height);
            m_general_surface.create(m_canvas_width, m_canvas_height);
            m_canvas_normals.create(m_canvas_width, m_canvas_height);
            m_canvas_shadow_map.create(m_canvas_width, m_canvas_height);
            m_canvas_depth.create(m_canvas_width, m_canvas_height);

            m_flat_light_direction = Calc::Normalize(m_flat_light_direction);
        }



};