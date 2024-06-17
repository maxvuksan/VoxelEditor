#pragma once
#include <vector>
#include <iostream>
////////////////////////////////////

#include "../System/include.h"

#include "Rect.h"
#include "BezierCurve.h"
#include "TileMaterial.h"
#include "GenerationType.h"

struct TileData{
    bool occupied = false;
    int tile_material_index = 0;

    int tile_object_index = -1;
    int voxel_object_index = -1; // no voxel material by default
    /*
        when a voxel material is placed, we only care about the top left tile, 
        all others are not drawn to make way for this top left tile,
    
        so... if (voxel_object_index != -1 && !is_top_left_of_voxel_material){
            do not draw tile
        }
    */
    int is_topleft_of_object = false; 
};

struct VoxelData{

    bool occupied = false;
    sf::Color normal_colour = sf::Color::Black; // normal vector as colour
    bool draw_sides = true; // if false, the voxel only has its front face drawn (essentially as a 2d plane)
    bool modified_by_generation = false; // prevents generation from applying effects twice voxels
};

struct GenerationCanvas{

    GenerationType m_type;
    std::vector<sf::Image*> m_tile_layer_images; // each layer of generation, is stored as a pointer to allow us to rearrange the generation stack
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
        sf::RenderTexture m_light_map;
        sf::Image m_raycasted_shadows;

        // for anything that wont show up in final scene (UI helpers etc...)
        sf::RenderTexture m_general_surface;

        sf::Vector3f m_flat_light_direction = sf::Vector3f(1.0,1.0, 1.5);
        sf::Uint32 m_shadow_lift = 0;

        float perspective_constant = 0.0015f;


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

        std::vector<std::vector<std::vector<VoxelData>>> m_voxel_space;

        std::vector<GenerationCanvas> m_generation_canvas; // we paint our generative effects on each layer, [generation type][tile_layer][x][y]


        // each vector represents a different tile_layer
        std::vector<std::vector<BezierCurve>> m_ropes;

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
                        m_tile_layers[i][x][y].is_topleft_of_object = false;
                        m_tile_layers[i][x][y].voxel_object_index = -1;
                    }
                }
            }


            m_raycasted_shadows.create(m_canvas_width, m_canvas_height, sf::Color::White);

            m_half_canvas_width = m_canvas_width / 2.0f;
            m_half_canvas_height = m_canvas_height / 2.0f;

            m_half_tile_width = width_in_tiles / 2.0f;
            m_half_tile_height = height_in_tiles / 2.0f;


            m_canvas.create(m_canvas_width, m_canvas_height);
            m_general_surface.create(m_canvas_width, m_canvas_height);
            m_canvas_normals.create(m_canvas_width, m_canvas_height);
            m_canvas_shadow_map.create(m_canvas_width, m_canvas_height);
            m_canvas_depth.create(m_canvas_width, m_canvas_height);

            m_light_map.create(m_canvas_width, m_canvas_height);
            m_light_map.clear(sf::Color(80,80,80));

            m_voxel_space.resize(tile_depth * m_tile_layers.size());
            for(int i = 0; i < tile_depth * m_tile_layers.size(); i++){

                m_voxel_space[i].resize(m_canvas_width);
                for(int x = 0; x < m_canvas_width; x++){
                    m_voxel_space[i][x].resize(m_canvas_height);
                }
            }

            m_flat_light_direction = Calc::Normalize(m_flat_light_direction);

            m_ropes.resize(m_tile_layers.size());
        }



};