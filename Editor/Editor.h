#pragma once

//////////////////////////////

#include "../System/include.h"
#include "Camera.h"
#include "ScreenData.h"


class Editor : public Core {


    
    enum TileMode{
        Tiles, // shows coloured placeholders
        Material,
        Voxel, // shows generated voxel geometry
        NUMBER_OF_VIEWMODES,
    };

    enum TileTool {
        BRUSH,
        RECTANGLE,
        NUMBER_OF_TILE_TOOLS,
    };


    public:

        Editor(int width, int height, std::string title = "Voxel Editor"): Core(width, height, title){}

        void Start() override;

        void Update() override;

        void Drawing();
        void DrawText();

        // for TileMode == Tiles
        void DrawTileGuides(sf::RenderTarget& surface);
        // for TileMode == Material
        void DrawMaterialGuides(sf::RenderTarget& surface);

        void MouseHandling();

        void CreateRect(bool remove);

        void CatchEvent(const sf::Event& event) override;


    protected:

        sf::Vector2f m_mouse_position;
        sf::Vector2f m_mouse_position_inital; 

        sf::Vector2i m_canvas_coordinate;
        sf::Vector2i m_canvas_coordinate_inital;

        bool m_drawing_place_rect;
        bool m_drawing_remove_rect;

        int m_view_mode;
        TileTool m_tile_tool;
        //ubScreen m_current_screen;

        // used for tile_material selection
        int selected_index = 0;

        int m_current_tile_layer;

        sf::Text m_text;
        sf::Font m_font;

        sf::Texture tile_texture;

        sf::RectangleShape cursor_outline;
        sf::RectangleShape canvas_outline;

        sf::RenderTexture m_tile_guides;

        ScreenData m_screen_data;

        Camera m_camera;


};