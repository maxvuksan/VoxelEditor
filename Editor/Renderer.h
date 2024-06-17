#pragma once

#include <SFML/Graphics.hpp>

/////////////////////////////

#include "ScreenData.h"
#include "Face.h"
#include "GenerationType.h"
#include "FaceDirection.h"
#include "../System/Utility/PerlinNoise.h"



enum DrawMode{
    NORMALS,
    DEPTH,
};

struct NormalPreset{
    sf::Vector3f normal;
    sf::Color normal_colour;
};

enum RenderStep{
    Inital,
    DrawNormals,
    DrawDepth,
    ComputeLight,
    RaycastLight,
    Finished,
};

class Renderer {


    public:

        static void Init();

        static void ClearVoxels(ScreenData& screen_data);
        static void CreateVoxelsFromScreenData(ScreenData& screen_data);
        static void CreateVoxelFromTile(ScreenData& screen_data, int tile_x, int tile_y, int tile_layer);
        static void CreateVoxelFromVoxelObject(ScreenData& screen_data, int tile_x, int tile_y, int tile_layer);
        static void CreateVoxelFromTileObject(ScreenData& screen_data, int tile_x, int tile_y, int tile_layer);
        static void CreateVoxelsFromRopes(ScreenData& screen_data);

        static void DrawVoxelLayer(ScreenData& screen_data, int y_position, int z_position, sf::RenderTarget& surface, DrawMode draw_mode = NORMALS);

        static void Update(ScreenData& screen_data, sf::RenderTarget& surface);
        static void DrawScreenData(ScreenData& screen_data, sf::RenderTarget& surface);

        static void DrawTileLayer(int tile_layer_index, ScreenData& screen_data, sf::RenderTarget& surface, DrawMode draw_mode = NORMALS);
        
        static bool ToggleFog(bool fog_state);
        /*
            goes over the screen_data.normal_canvas interpreting each normal value as a light value (dot product from the sun)
        */
        static void ComputeNormalCanvas(ScreenData& screen_data, int y_position);
        static void ComputeRaycastedLighting(ScreenData& screen_data);

        static void StartRender(){m_render_progress = RenderStep::Inital;}
        static void StopRender(){m_render_progress = RenderStep::Finished;}

        static bool m_demo_fog;



        static void ManipulateVoxelsThroughGeneration(ScreenData& screen_data);

        static void MarkVoxelsAsUntouchedByGeneration(ScreenData& screen_data);
        // strength 0 -> 1

        static void Generation_Shadow(ScreenData& screen_data, sf::Image* gen_canvas, int x, int y, int z, float percent);
        static void Generation_Overshadow(ScreenData& screen_data, sf::Image* gen_canvas, int x, int y, int z, float percent);
        static void Generation_Melt(ScreenData& screen_data, sf::Image* gen_canvas, int x, int y, int z, float percent);
        static void Generation_Roots(ScreenData& screen_data, sf::Image* gen_canvas, int x, int y, int z, float percent, bool CHAOS, bool THICK);

    private:

        static PerlinNoise m_perlin; 

        static RenderStep m_render_progress;
        static int m_progress_increment_y; // the y position of computation, essentially scans the voxel grid with this value 
        static int m_progress_goal_y; // what the increment must reach
        static int m_progress_increment_z; // the y position of computation, essentially scans the voxel grid with this value 
        static int m_progress_goal_z; // what the increment must reach

        static sf::Image m_spare_image;
        static NormalPreset m_normal_dictionary[FaceDirection::NUMBER_OF_FACES];

};