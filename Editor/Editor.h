#pragma once

//////////////////////////////

#include "../System/include.h"
#include "Camera.h"
#include "ScreenData.h"


class Editor : public Core {


    
    enum TileMode{
        Tiles, // shows coloured placeholders
        Material,
        Rope,
        Generation, // producally changing the voxel data
        Lightmap,
        Voxel, // shows generated voxel geometry
        NUMBER_OF_VIEWMODES,
    };

    enum TileTool {
        BRUSH,
        RECTANGLE,
        NUMBER_OF_TILE_TOOLS,
    };

    enum MaterialSelectionType{
        TILE_MATERIALS,
        TILE_OBJECTS,
        VOXEL_OBJECTS,
        NUMBER_OF_MATERIAL_SELECT_TYPES,
    };


    public:

        Editor(int width, int height, std::string title = "Voxel Editor"): Core(width, height, title){}
        void Cleanup() override;

        void Start() override;

        void Update() override;


        void DrawPalettePreview();

        void DrawEditor();
        void DrawEditorText();

        void SetTileOccupied(int x, int y, bool occupied);

        // for TileMode == Tiles
        void DrawTileGuides(sf::RenderTarget& surface);
        // for TileMode == Material
        void DrawMaterialGuides(sf::RenderTarget& surface);

        void DrawRopeGuides(sf::RenderTarget& surface);

        void DrawGenerationGuides(sf::RenderTarget& surface);

        void DrawLightmapGuides(sf::RenderTarget& surface);

        // applys weight to pixels by amount (which can be either positive (adding) or negative (erasing) )
        void PaintToGenerationImage(int pixel_x, int pixel_y, sf::Uint8 amount, int brush_size);
        void AverageInbetweenGenerationPoints(int min_x, int max_x, int min_y, int max_y);

        void SetVoxelObject(int x, int y);
        void SetTileObject(int x, int y);
        void SetTileMaterial(int x, int y);

        void MouseHandling();

        void CreateRect(bool remove);

        void CatchEvent(const sf::Event& event) override;


    protected:

        std::vector<std::string> m_keys_for_voxel_object;
        std::vector<std::string> m_keys_for_tile_object;
        std::vector<std::string> m_keys_for_tile_materials;

        int selected_lightmap = 0;
        sf::Sprite m_lightmap_sprite;
        float lightmap_sprite_scale_x = 1.0;
        float lightmap_sprite_scale_y = 1.0;
        int lightmap_sprite_rotation = 0;

        sf::Shader m_colour_level_shader;

        sf::Vector2f m_mouse_position;
        sf::Vector2f m_canvas_position_free;
        sf::Vector2f m_mouse_position_inital; 

        sf::Vector2i m_tile_coord;
        sf::Vector2i m_tile_coord_inital;

        sf::Vector2f m_canvas_coordinate;

        // toggled with tab
        bool palette_preview = true;
        sf::Texture palette_preview_texture;
        sf::Sprite palette_preview_sprite;

        bool m_drawing_place_rect;
        bool m_drawing_remove_rect;

        int m_view_mode;
        TileTool m_tile_tool;
        bool m_symmetric_x = false;
        bool m_symmetric_y = false;

        bool m_material_creates_geometry = false;

        int selected_palette;

        // used for tile_material selection
        int selected_material = 0;
        // uses MaterialSelectionType enum
        int m_material_select_type;

        int selected_generation = 0;
        int selected_generation_stack_item = 0;
        bool viewing_generation_stack; // are we selecting generation effects, or are we viewing the stack

        BezierCurve* rope_being_created = nullptr;
        BezierCurve* rope_being_moved = nullptr;
        sf::Vector2f* rope_position_being_moved;

        int m_brush_size; // used for generation editing
        int m_generation_brush_density; // 5 - 255

        int m_current_tile_layer;

        sf::Text m_text;
        sf::Font m_font;

        sf::Texture palette_texture;

        sf::RectangleShape cursor_outline;
        sf::RectangleShape canvas_outline;
        sf::CircleShape cursor_brush_circle;

        sf::RenderTexture m_tile_guides;

        ScreenData m_screen_data;

        Camera m_camera;


};