#pragma once

#include <SFML/Graphics.hpp>

/////////////////////////////

#include "ScreenData.h"
#include "Face.h"


enum FaceDirection{
    FACE_FRONT,
    FACE_TOP,
    FACE_LEFT,
    FACE_RIGHT,
    FACE_BOTTOM,
    NUMBER_OF_FACES
};

enum DrawMode{
    NORMALS,
    DEPTH,
};

struct NormalPreset{
    sf::Vector3f normal;
    sf::Color normal_colour;
};

class Renderer {



    public:

        static void Init();

        static void DrawScreenData(ScreenData& screen_data, sf::RenderTarget& surface);
        static void DrawTileLayer(int tile_layer_index, ScreenData& screen_data, sf::RenderTarget& surface, DrawMode draw_mode = NORMALS);
        
        static bool ToggleFog(bool fog_state);
        /*
            goes over the screen_data.normal_canvas interpreting each normal value as a light value (dot product from the sun)
        */
        static void ComputeNormalCanvas(ScreenData& screen_data);
        static void ComputeRaycastedLighting(ScreenData& screen_data);

        static bool m_demo_fog;

    private:


        static sf::Image m_spare_image;
        static NormalPreset m_normal_dictionary[NUMBER_OF_FACES];

};