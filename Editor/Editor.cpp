#include "Editor.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "Util.h"

void Editor::Start(){

    
    if(!m_font.loadFromFile("Assets/Roboto-Medium.ttf")){
        //std::cout << "[ERROR]: Failed to load font, Editor::Start()\n";
    }

    const std::string frag_shader = \

        "uniform sampler2D u_texture;"\
        "uniform sampler2D u_palette_texture;"\ 

        "void main()"\
        "{"\
            "vec4 sampled_pixel = texture2D(u_texture, gl_TexCoord[0].xy);"\
            "vec4 fog_colour = texture2D(u_palette_texture, vec2(0,0.8));"\
            "fog_colour.a = 1.0;"

            "vec4 solid_colour = texture2D(u_palette_texture, vec2(sampled_pixel.r, sampled_pixel.b * 0.35));"\
            "solid_colour.a = 1.0;"\


            "gl_FragColor = mix(solid_colour, fog_colour, (1.0 - sampled_pixel.a * sampled_pixel.a));"\
        "}";

    if(!m_colour_level_shader.loadFromMemory(frag_shader, sf::Shader::Fragment)){
        //std::cout << "[ERROR]: Failed to load font, Editor::Start()\n";
    }

    palette_texture.loadFromFile("Assets/Palettes/palette_1.png");


    m_text.setFont(m_font);
    m_text.setCharacterSize(16);

    m_camera.m_perspective_factor = 0.1;

    m_window.setMouseCursorVisible(false);

    // should be even?

    m_screen_data.Create(110, 54, 16, 16);

    canvas_outline.setOutlineColor(sf::Color::White);
    canvas_outline.setOutlineThickness(1);
    canvas_outline.setFillColor(sf::Color::Transparent);
    canvas_outline.setSize(sf::Vector2f(m_screen_data.m_canvas_width, m_screen_data.m_canvas_height));
    canvas_outline.setOrigin(sf::Vector2f(m_screen_data.m_canvas_width / 2.0f, m_screen_data.m_canvas_height / 2.0f));

    cursor_outline.setOutlineColor(sf::Color::White);
    cursor_outline.setOutlineThickness(2);
    cursor_outline.setSize(sf::Vector2f(m_screen_data.m_tile_size, m_screen_data.m_tile_size));
    cursor_outline.setFillColor(sf::Color(0,0,15, 50));

    cursor_brush_circle.setOutlineColor(sf::Color::White);
    cursor_brush_circle.setFillColor(sf::Color::Transparent);
    cursor_brush_circle.setOutlineThickness(2);

    m_current_tile_layer = 0;
    m_view_mode = TileMode::Tiles;
    m_tile_tool = TileTool::BRUSH;

    rope_position_being_moved = nullptr;
    rope_being_created = nullptr;

    m_brush_size = 1;
    selected_palette = 0;

    palette_preview = false;
    m_material_select_type = MaterialSelectionType::TILE_MATERIALS;

    Renderer::Init();
    AssetManager::Init(m_screen_data);

    m_keys_for_voxel_object = AssetManager::GetAllVoxelObjectNames();
    m_keys_for_tile_object = AssetManager::GetAllTileObjectNames();
    m_keys_for_tile_materials = AssetManager::GetAllTileMaterialNames();
}

void Editor::Cleanup(){
    AssetManager::Destruct();
}

void Editor::Update(){

    m_window.clear(sf::Color(176, 157, 166));

    if(palette_preview){
        DrawPalettePreview();
    }
    else{
        MouseHandling();
        DrawEditor();
        DrawEditorText();
    }
}

void Editor::DrawPalettePreview(){


    palette_preview_sprite.setOrigin(sf::Vector2f(floor(m_screen_data.m_canvas_width / 2.0f), floor(m_screen_data.m_canvas_height / 2.0f)));
    palette_preview_sprite.setPosition(floor(m_window.getSize().x / 2.0f), floor(m_window.getSize().y / 2.0f));

    m_colour_level_shader.setParameter("u_texture", sf::Shader::CurrentTexture);
    m_colour_level_shader.setParameter("u_palette_texture", *AssetManager::GetPalettes()[selected_palette]);

    m_window.draw(palette_preview_sprite, &m_colour_level_shader);

    sf::Sprite palette_sprite(*AssetManager::GetPalettes()[selected_palette]);
    palette_sprite.setPosition(sf::Vector2f(32, 64));
    palette_sprite.setScale(sf::Vector2f(32,32));

    m_text.setPosition(sf::Vector2f(32,32));
    m_text.setColor(sf::Color::White);
    m_text.setString("(E/D) Cycle Palettes");
    m_window.draw(m_text);

    m_window.draw(palette_sprite);
}

void Editor::DrawEditorText(){
    
    // tool text
    for(int i = 0; i < NUMBER_OF_TILE_TOOLS; i++){

        m_text.setPosition(sf::Vector2f(32,32 * (i + 1)));
        m_text.setColor(sf::Color::White);
        switch(i){
            case TileTool::BRUSH:{
                m_text.setString("(B) Brush");
                break;
            }
            case TileTool::RECTANGLE:{
                m_text.setString("(R) Rectangle");
                break;
            }
        }

        if(i == m_tile_tool){
            m_text.setColor(sf::Color::Red);
        }

        m_window.draw(m_text);
    }

    // tile layers
    
    m_text.setColor(sf::Color::White);
    m_text.setString("(Esc) Palette Preview");
    m_text.setPosition(sf::Vector2f(32, 32 * 3));
    m_window.draw(m_text);
    
    if(m_symmetric_x){
        m_text.setColor(sf::Color::Green);
    }
    else{
        m_text.setColor(sf::Color::White);
    }
    m_text.setString("(1) X Symmetric");
    m_text.setPosition(sf::Vector2f(32, 32 * 4));
    m_window.draw(m_text);

    if(m_symmetric_y){
        m_text.setColor(sf::Color::Green);
    }
    else{
        m_text.setColor(sf::Color::White);
    }
    m_text.setString("(2) Y Symmetric");
    m_text.setPosition(sf::Vector2f(32, 32 * 5));
    m_window.draw(m_text);
    if(m_material_creates_geometry){
        m_text.setColor(sf::Color::Green);
    }
    else{
        m_text.setColor(sf::Color::White);
    }
    m_text.setString("(3) Material Creates Tiles");
    m_text.setPosition(sf::Vector2f(32, 32 * 6));
    m_window.draw(m_text);



    m_text.setColor(sf::Color::White);
    m_text.setString("(W/S) Tile Layer");
    m_text.setPosition(sf::Vector2f(32, 232));
    m_window.draw(m_text);
    
    for(int i = 0; i < m_screen_data.m_tile_layers.size(); i++){

        m_text.setString("Layer " + std::to_string(i + 1));

        m_text.setColor(sf::Color::White);
        m_text.setPosition(sf::Vector2f(32, 200 + 32 * (i + 2)));

        if(i == m_current_tile_layer){
            m_text.setColor(sf::Color::Red);
        }

        m_window.draw(m_text);
    }

    // view mode 
    m_text.setColor(sf::Color::White);
    m_text.setString("(Up/Down) View Mode");
    m_text.setPosition(sf::Vector2f(32, 365 + 32));
    m_window.draw(m_text);
    
    for(int i = 0; i < TileMode::NUMBER_OF_VIEWMODES; i++){

        switch (i){
            case TileMode::Tiles:
                m_text.setString("Tiles");
                break;
            case TileMode::Material:
                m_text.setString("Material");
                break;         
            case TileMode::Rope:
                m_text.setString("Rope");
                break;
            case TileMode::Generation:
                m_text.setString("Generation");
                break;             
            case TileMode::Lightmap:
                m_text.setString("Lightmap");
                break;    
            case TileMode::Voxel:
                m_text.setString("Renderer");
                break;           
        }

        if(i == m_view_mode){
            m_text.setColor(sf::Color::Red);
        }
        else{
            m_text.setColor(sf::Color::White);
        }

        m_text.setPosition(sf::Vector2f(32, 365 + 32 * (i + 2)));

        m_window.draw(m_text);
    }

    // draw tile material list
    if(m_view_mode == TileMode::Material){
        std::vector<std::string>* materials; 
        
        
        switch(m_material_select_type){

            case TILE_MATERIALS: {

                materials = &m_keys_for_tile_materials;
                m_text.setString("(E/D) Tile Materials");
                break;
            }
            case VOXEL_OBJECTS: {

                materials = &m_keys_for_voxel_object;
                m_text.setString("(E/D) Voxel Objects");
                break;
            }
            case TILE_OBJECTS: {

                materials = &m_keys_for_tile_object;
                m_text.setString("(E/D) Tile Objects");
                break;
            }
        }
        m_text.setPosition(sf::Vector2f(260, 32 ));
        m_window.draw(m_text);

        m_text.setString("(Tab) Switch Material Mode");
        m_text.setPosition(sf::Vector2f(260, 64 ));
        m_window.draw(m_text);

        for(int i = 0; i < materials->size(); i++){

            m_text.setString(materials->at(i));

            if(i == selected_material){
                m_text.setColor(sf::Color::Green);
            }
            else{
                m_text.setColor(sf::Color::White);

            }

            m_text.setPosition(sf::Vector2f(260, 32 * (i + 4)));

            m_window.draw(m_text);
        }
    }

    if(m_view_mode == TileMode::Rope){
        m_text.setString("(C) Clear Ropes");
        m_text.setPosition(sf::Vector2f(260, 32 ));
        m_window.draw(m_text);

        m_text.setString("(E/D) Adjusted Selected Rope Slack");
        m_text.setPosition(sf::Vector2f(260, 32 * 2 ));
        m_window.draw(m_text);

        m_text.setString("(MMB) Select Rope Point");
        m_text.setPosition(sf::Vector2f(260, 32 * 3 ));
        m_window.draw(m_text);


        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::C)){
            m_screen_data.m_ropes[m_current_tile_layer].clear();
        }
    }

    if(m_view_mode == TileMode::Lightmap){
        m_text.setString("(C) Clear Lightmap");
        m_text.setPosition(sf::Vector2f(260, 32 * 2 ));
        m_window.draw(m_text);

        m_text.setString("(F) Fill Lightmap");
        m_text.setPosition(sf::Vector2f(260, 32 * 3 ));
        m_window.draw(m_text);

        m_text.setString("(E/D) Cycle Sprites");
        m_text.setPosition(sf::Vector2f(260, 32 * 4 ));
        m_window.draw(m_text);
        
        m_text.setString("(MMB) Rotate");
        m_text.setPosition(sf::Vector2f(260, 32 * 5 ));
        m_window.draw(m_text);

        m_text.setString("(MScroll) Scale Y");
        m_text.setPosition(sf::Vector2f(260, 32 * 6 ));
        m_window.draw(m_text);

        m_text.setString("(MScroll + Shift) Scale X");
        m_text.setPosition(sf::Vector2f(260, 32 * 7 ));
        m_window.draw(m_text);

    }


}


void Editor::DrawTileGuides(sf::RenderTarget& surface){

    for(int i = 0; i < m_screen_data.m_tile_layers.size(); i++){

        sf::VertexArray tile_guides;
        tile_guides.setPrimitiveType(sf::PrimitiveType::Quads);
        
        sf::Vertex vertex;

        switch(i){

            // collision layer
            case 0:
                vertex.color = sf::Color::Black;
                break;
    
            case 1: 
                vertex.color = sf::Color(255, 0, 0, 100);
                break;

            case 2:
                vertex.color = sf::Color(0, 0, 255, 100);
                break;
        }

        for(int x = 0; x < m_screen_data.m_tile_layers[i].size(); x++){         
            for(int y = 0; y < m_screen_data.m_tile_layers[i].size(); y++){
                
                if(!m_screen_data.m_tile_layers[i][x][y].occupied){
                    continue;
                }

                float _x = x * m_screen_data.m_tile_size;
                float _y = y * m_screen_data.m_tile_size;

                // construct square
                vertex.position.x = _x;
                vertex.position.y = _y;
                tile_guides.append(vertex);

                vertex.position.x = _x + m_screen_data.m_tile_size;
                vertex.position.y = _y; 
                tile_guides.append(vertex);

                vertex.position.x = _x + m_screen_data.m_tile_size;
                vertex.position.y = _y + m_screen_data.m_tile_size;
                tile_guides.append(vertex);

                vertex.position.x = _x;
                vertex.position.y = _y + m_screen_data.m_tile_size;
                tile_guides.append(vertex);
            }
        }
        
        surface.draw(tile_guides);
    }

}

void Editor::DrawMaterialGuides(sf::RenderTarget& surface){

    int quarter_tile_size = m_screen_data.m_tile_size / 4.0f;

    for(int i = 0; i < m_screen_data.m_tile_layers.size(); i++){

        sf::VertexArray tile_guides;
        tile_guides.setPrimitiveType(sf::PrimitiveType::Quads);

        sf::Vertex vertex;

        for(int x = 0; x < m_screen_data.m_tile_layers[i].size(); x++){         
            for(int y = 0; y < m_screen_data.m_tile_layers[i].size(); y++){
                
                if(!m_screen_data.m_tile_layers[i][x][y].occupied){
                    continue;
                }

                float _x = x * m_screen_data.m_tile_size;
                float _y = y * m_screen_data.m_tile_size;

                // only show selected layer (to draw materials on)
                if(i == m_current_tile_layer){
                    vertex.color = sf::Color(0,0,0,150);
                }
                else{
                    vertex.color = sf::Color(0,0,0,20);
                }


                // construct square for tile
                vertex.position.x = _x;
                vertex.position.y = _y;
                tile_guides.append(vertex);

                vertex.position.x = _x + m_screen_data.m_tile_size;
                vertex.position.y = _y; 
                tile_guides.append(vertex);

                vertex.position.x = _x + m_screen_data.m_tile_size;
                vertex.position.y = _y + m_screen_data.m_tile_size;
                tile_guides.append(vertex);

                vertex.position.x = _x;
                vertex.position.y = _y + m_screen_data.m_tile_size;
                tile_guides.append(vertex);

            }
        }
        
        surface.draw(tile_guides);
        tile_guides.clear();

        sf::VertexArray object_guides;
        object_guides.setPrimitiveType(sf::PrimitiveType::Quads);


        for(int x = 0; x < m_screen_data.m_tile_layers[i].size(); x++){         
            for(int y = 0; y < m_screen_data.m_tile_layers[i][x].size(); y++){
                
                // is there a tile
                if(!m_screen_data.m_tile_layers[i][x][y].occupied){
                    continue;
                }
                // are we on the selected layer?
                if(i != m_current_tile_layer){
                    continue;
                }
                // is part of a voxel material
                if(m_screen_data.m_tile_layers[i][x][y].voxel_object_index != -1 && !m_screen_data.m_tile_layers[i][x][y].is_topleft_of_object){
                    continue;
                }

                float _x = x * m_screen_data.m_tile_size;
                float _y = y * m_screen_data.m_tile_size;

                // voxel objects
                if(m_screen_data.m_tile_layers[i][x][y].voxel_object_index != -1){
                    if(m_screen_data.m_tile_layers[i][x][y].is_topleft_of_object){

                        auto voxel_obj = AssetManager::GetVoxelObject(m_screen_data.m_tile_layers[i][x][y].voxel_object_index);
                        vertex.color = sf::Color::White;

                        vertex.position.x = _x;
                        vertex.position.y = _y;
                        vertex.texCoords.x = voxel_obj.texture_atlas_position_x; 
                        vertex.texCoords.y = voxel_obj.texture_atlas_position_y;
                        object_guides.append(vertex);

                        vertex.position.x = _x + voxel_obj.layer_width;
                        vertex.position.y = _y;
                        vertex.texCoords.x = voxel_obj.texture_atlas_position_x + voxel_obj.layer_width; 
                        vertex.texCoords.y = voxel_obj.texture_atlas_position_y;
                        object_guides.append(vertex);

                        vertex.position.x = _x + voxel_obj.layer_width;
                        vertex.position.y = _y + voxel_obj.layer_height;
                        vertex.texCoords.x = voxel_obj.texture_atlas_position_x + voxel_obj.layer_width; 
                        vertex.texCoords.y = voxel_obj.texture_atlas_position_y + voxel_obj.layer_height; 
                        object_guides.append(vertex);

                        vertex.position.x = _x;
                        vertex.position.y = _y + voxel_obj.layer_height;
                        vertex.texCoords.x = voxel_obj.texture_atlas_position_x;
                        vertex.texCoords.y = voxel_obj.texture_atlas_position_y + voxel_obj.layer_height; 
                        object_guides.append(vertex);
                    }
                } 
                // tile objects
                else if(m_screen_data.m_tile_layers[i][x][y].tile_object_index != -1){
                    if(m_screen_data.m_tile_layers[i][x][y].is_topleft_of_object){

                        vertex.color = sf::Color::White;

                        auto tile_obj = AssetManager::GetTileObject(m_screen_data.m_tile_layers[i][x][y].voxel_object_index);

                        vertex.position.x = _x;
                        vertex.position.y = _y;
                        vertex.texCoords.x = tile_obj.texture_atlas_position_x; 
                        vertex.texCoords.y = tile_obj.texture_atlas_position_y;
                        object_guides.append(vertex);

                        vertex.position.x = _x + tile_obj.layer_width;
                        vertex.position.y = _y;
                        vertex.texCoords.x = tile_obj.texture_atlas_position_x + tile_obj.layer_width; 
                        vertex.texCoords.y = tile_obj.texture_atlas_position_y;
                        object_guides.append(vertex);

                        vertex.position.x = _x + tile_obj.layer_width;
                        vertex.position.y = _y + tile_obj.layer_height;
                        vertex.texCoords.x = tile_obj.texture_atlas_position_x + tile_obj.layer_width; 
                        vertex.texCoords.y = tile_obj.texture_atlas_position_y + tile_obj.layer_height; 
                        object_guides.append(vertex);

                        vertex.position.x = _x;
                        vertex.position.y = _y + tile_obj.layer_height;
                        vertex.texCoords.x = tile_obj.texture_atlas_position_x;
                        vertex.texCoords.y = tile_obj.texture_atlas_position_y + tile_obj.layer_height; 
                        object_guides.append(vertex);
                    }
                }
                // tile materials
                else{

                    vertex.color = Util::GetColourFromColourLoop(m_screen_data.m_tile_layers[i][x][y].tile_material_index);

                    // construct square for material
                    vertex.position.x = _x + quarter_tile_size;
                    vertex.position.y = _y + quarter_tile_size;
                    tile_guides.append(vertex);

                    vertex.position.x = _x + m_screen_data.m_tile_size - quarter_tile_size;
                    vertex.position.y = _y + quarter_tile_size; 
                    tile_guides.append(vertex);

                    vertex.position.x = _x + m_screen_data.m_tile_size - quarter_tile_size;
                    vertex.position.y = _y + m_screen_data.m_tile_size - quarter_tile_size;
                    tile_guides.append(vertex);

                    vertex.position.x = _x + quarter_tile_size;
                    vertex.position.y = _y + m_screen_data.m_tile_size - quarter_tile_size;
                    tile_guides.append(vertex);
                }
            }
        }
        
        surface.draw(tile_guides);

        /*

            to reduce texture swaps when drawing for material guides, all guides are stored in a large texture atlas,
            this allows all guides to be drawn in a single draw call
        
        */
        sf::RenderStates texture_atlas_rs;
        texture_atlas_rs.texture = &AssetManager::GetTextureAtlas().getTexture();
        surface.draw(object_guides, texture_atlas_rs);
    
    }

}

void Editor::DrawRopeGuides(sf::RenderTarget& surface){

    // move rope being created to mouse pos
    if(rope_being_created != nullptr){
        
        rope_being_created->end = m_canvas_coordinate;
        
        // adjust slack
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::E)){
            rope_being_created->slack -= 3.0f;
        }    
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)){
            rope_being_created->slack += 3.0f;
        }
    }
    // moving rope
    else if(rope_being_moved != nullptr){
        
        *rope_position_being_moved = m_canvas_coordinate;
        // adjust slack
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::E)){
            rope_being_moved->slack -= 3.0f;
        }    
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)){
            rope_being_moved->slack += 3.0f;
        }
    }
    for(int i = 0; i < m_screen_data.m_tile_layers.size(); i++){

        sf::VertexArray tile_guides;
        tile_guides.setPrimitiveType(sf::PrimitiveType::Quads);
        
        sf::Vertex vertex;

        for(int x = 0; x < m_screen_data.m_tile_layers[i].size(); x++){         
            for(int y = 0; y < m_screen_data.m_tile_layers[i].size(); y++){
                
                if(!m_screen_data.m_tile_layers[i][x][y].occupied){
                    continue;
                }

                float _x = x * m_screen_data.m_tile_size;
                float _y = y * m_screen_data.m_tile_size;

                // only show selected layer (to draw materials on)
                if(i == m_current_tile_layer){
                    vertex.color = sf::Color(0,0,0,150);
                }
                else{
                    vertex.color = sf::Color(0,0,0,20);
                }


                // construct square for tile
                vertex.position.x = _x;
                vertex.position.y = _y;
                tile_guides.append(vertex);

                vertex.position.x = _x + m_screen_data.m_tile_size;
                vertex.position.y = _y; 
                tile_guides.append(vertex);

                vertex.position.x = _x + m_screen_data.m_tile_size;
                vertex.position.y = _y + m_screen_data.m_tile_size;
                tile_guides.append(vertex);

                vertex.position.x = _x;
                vertex.position.y = _y + m_screen_data.m_tile_size;
                tile_guides.append(vertex);

            }
        }
        
        surface.draw(tile_guides);
        tile_guides.clear();


        sf::Vertex rope_vertex;
        sf::VertexArray rope_vertex_array;
        rope_vertex_array.setPrimitiveType(sf::PrimitiveType::Lines);

        if(i == m_current_tile_layer){
            rope_vertex.color = sf::Color::Yellow;
        }
        else{
            rope_vertex.color = sf::Color(0,0,0,50);
        }

        sf::CircleShape rope_circle;
        rope_circle.setFillColor(sf::Color::Transparent);
        rope_circle.setRadius(6);
        rope_circle.setOrigin(sf::Vector2f(6,6));
        rope_circle.setOutlineColor(sf::Color::White);
        rope_circle.setOutlineThickness(2);

        for(auto& rope : m_screen_data.m_ropes[i]){

            auto positions = rope.SamplePositions();

            if(i == m_current_tile_layer){
                if(Calc::Distance(rope.start, m_canvas_coordinate) < 8){
                    rope_circle.setFillColor(sf::Color(100,100,255, 150));
                    rope_circle.setOutlineColor(sf::Color(100,100,255));
                }
                else{
                    rope_circle.setFillColor(sf::Color::Transparent);
                    rope_circle.setOutlineColor(sf::Color::White);
                }

                // grab points
                rope_circle.setPosition(rope.start);
                surface.draw(rope_circle);

                if(Calc::Distance(rope.end, m_canvas_coordinate) < 8){
                    rope_circle.setFillColor(sf::Color(100,100,255, 150));
                    rope_circle.setOutlineColor(sf::Color(100,100,255));
                }
                else{
                    rope_circle.setFillColor(sf::Color::Transparent);
                    rope_circle.setOutlineColor(sf::Color::White);
                }

                rope_circle.setPosition(rope.end);
                surface.draw(rope_circle);
            }

            sf::Vector2f last_pos = rope.start;
            for(auto& pos : positions){

                rope_vertex.position = last_pos;
                rope_vertex_array.append(rope_vertex);

                rope_vertex.position = pos;
                rope_vertex_array.append(rope_vertex);

                last_pos = pos;
            }
        }
        
        surface.draw(rope_vertex_array);
    }






}


void Editor::DrawLightmapGuides(sf::RenderTarget& surface){

    surface.clear(sf::Color(100,100,100));

    // draw tiles
    for(int i = 0; i < m_screen_data.m_tile_layers.size(); i++){

        sf::VertexArray tile_guides;
        tile_guides.setPrimitiveType(sf::PrimitiveType::Quads);
        
        sf::Vertex vertex;

        switch(i){

            // collision layer
            case 0:
                vertex.color = sf::Color(0,0,0,100);
                break;
    
            case 1: 
                vertex.color = sf::Color(0,0,0,65);
                break;

            case 2:
                vertex.color = sf::Color(0,0,0,35);
                break;
        }

        for(int x = 0; x < m_screen_data.m_tile_layers[i].size(); x++){         
            for(int y = 0; y < m_screen_data.m_tile_layers[i].size(); y++){
                
                if(!m_screen_data.m_tile_layers[i][x][y].occupied){
                    continue;
                }

                float _x = x * m_screen_data.m_tile_size;
                float _y = y * m_screen_data.m_tile_size;

                // construct square
                vertex.position.x = _x;
                vertex.position.y = _y;
                tile_guides.append(vertex);

                vertex.position.x = _x + m_screen_data.m_tile_size;
                vertex.position.y = _y; 
                tile_guides.append(vertex);

                vertex.position.x = _x + m_screen_data.m_tile_size;
                vertex.position.y = _y + m_screen_data.m_tile_size;
                tile_guides.append(vertex);

                vertex.position.x = _x;
                vertex.position.y = _y + m_screen_data.m_tile_size;
                tile_guides.append(vertex);
            }
        }
        
        surface.draw(tile_guides);
    }

    if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)){
        lightmap_sprite_rotation += 1;
        lightmap_sprite_rotation %= 360;
    }


    m_screen_data.m_light_map.display();
    sf::Sprite lightmap(m_screen_data.m_light_map.getTexture());
    surface.draw(lightmap, sf::BlendMultiply);

    m_lightmap_sprite.setTexture(*AssetManager::GetLightShapes()[selected_lightmap], true);
    m_lightmap_sprite.setOrigin(sf::Vector2f(m_lightmap_sprite.getTexture()->getSize().x / 2.0f, m_lightmap_sprite.getTexture()->getSize().y / 2.0f));
    m_lightmap_sprite.setPosition(m_canvas_position_free);
    m_lightmap_sprite.setColor(sf::Color(255,255,255,150));
    m_lightmap_sprite.setScale(sf::Vector2f(lightmap_sprite_scale_x, lightmap_sprite_scale_y));
    m_lightmap_sprite.setRotation(lightmap_sprite_rotation);
    surface.draw(m_lightmap_sprite);

    if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)){
        m_lightmap_sprite.setColor(sf::Color::White);
        m_screen_data.m_light_map.draw(m_lightmap_sprite);
    }
    if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)){
        m_lightmap_sprite.setColor(sf::Color(80,80,80));
        m_lightmap_sprite.setPosition(m_canvas_position_free);
        m_screen_data.m_light_map.draw(m_lightmap_sprite);
    }
}


void Editor::DrawGenerationGuides(sf::RenderTarget& surface){

    // draw tile guides
    for(int i = 0; i < m_screen_data.m_tile_layers.size(); i++){

        sf::VertexArray tile_guides;
        tile_guides.setPrimitiveType(sf::PrimitiveType::Quads);
        
        sf::Vertex vertex;

        // only show selected layer (to draw materials on)
        if(i == m_current_tile_layer){
            vertex.color = sf::Color(0,0,0,150);
        }
        else{
            vertex.color = sf::Color(0,0,0,20);
        }

        for(int x = 0; x < m_screen_data.m_tile_layers[i].size(); x++){         
            for(int y = 0; y < m_screen_data.m_tile_layers[i].size(); y++){
                
                if(!m_screen_data.m_tile_layers[i][x][y].occupied){
                    continue;
                }

                float _x = x * m_screen_data.m_tile_size;
                float _y = y * m_screen_data.m_tile_size;


                // construct square for tile
                vertex.position.x = _x;
                vertex.position.y = _y;
                tile_guides.append(vertex);

                vertex.position.x = _x + m_screen_data.m_tile_size;
                vertex.position.y = _y; 
                tile_guides.append(vertex);

                vertex.position.x = _x + m_screen_data.m_tile_size;
                vertex.position.y = _y + m_screen_data.m_tile_size;
                tile_guides.append(vertex);

                vertex.position.x = _x;
                vertex.position.y = _y + m_screen_data.m_tile_size;
                tile_guides.append(vertex);

            }
        }
        
        surface.draw(tile_guides);
        tile_guides.clear();
    }

    sf::Texture focused_generation_map;
    focused_generation_map.loadFromImage(m_screen_data.m_generation_canvas[0][m_current_tile_layer]);

    // draw generation maps
    surface.draw(sf::Sprite(focused_generation_map));
}

void Editor::PaintToGenerationImage(int pixel_x, int pixel_y, sf::Uint8 amount, int brush_size){

    brush_size = Calc::Clamp(brush_size - 1, 0, 999);

    // paint in an area
    for(int x = -brush_size; x <= brush_size; x ++){
        for(int y = -brush_size; y <= brush_size; y++){
        
            int real_x = x * m_screen_data.m_tile_size + pixel_x;
            int real_y = y * m_screen_data.m_tile_size + pixel_y; 
            // ensure set pixel is within canvas boundss
            if(real_x < 0 || real_x >= m_screen_data.m_canvas_width){
                continue;
            }
            if(real_y < 0 || real_y >= m_screen_data.m_canvas_height){
                continue;
            }

            if(Calc::Distance(sf::Vector2f(pixel_x, pixel_y), sf::Vector2f(real_x, real_y)) > brush_size * m_screen_data.m_tile_size){
                continue;
            }

            
            /*

            float dis = Calc::Distance(sf::Vector2f(0,0), sf::Vector2f(x, y)) / brush_size;
            dis = Calc::Clamp(dis, 0, 1);
            float _amount = Calc::Lerp(amount, 0, dis);

            int new_amount = round(m_screen_data.m_generation_canvas[0][m_current_tile_layer].getPixel(real_x, real_y).r + _amount);
            new_amount = Calc::Clamp(new_amount, 0, 255);

            m_screen_data.m_generation_canvas[0][m_current_tile_layer].setPixel(real_x, real_y, sf::Color(new_amount, 0, 0, m_screen_data.m_generation_canvas[0][m_current_tile_layer].getPixel(real_x, real_y).a));
            */
           m_screen_data.m_generation_canvas[0][m_current_tile_layer].setPixel(real_x, real_y, sf::Color(amount, 0, 0, m_screen_data.m_generation_canvas[0][m_current_tile_layer].getPixel(real_x, real_y).a));
        }
    }

    brush_size++;
    AverageInbetweenGenerationPoints(pixel_x - brush_size * m_screen_data.m_tile_size,
                                    pixel_x + brush_size * m_screen_data.m_tile_size,
                                    pixel_y - brush_size * m_screen_data.m_tile_size,
                                    pixel_y + brush_size * m_screen_data.m_tile_size);
}

void Editor::AverageInbetweenGenerationPoints(int min_x, int max_x, int min_y, int max_y){

    min_x = Calc::Clamp(min_x, 0, m_screen_data.m_canvas_width);
    max_x = Calc::Clamp(max_x, 0, m_screen_data.m_canvas_width);
    min_y = Calc::Clamp(min_y, 0, m_screen_data.m_canvas_height);
    max_y = Calc::Clamp(max_y, 0, m_screen_data.m_canvas_height);

    for(int x = min_x; x < max_x; x++){
        for(int y = min_y; y < max_y; y++){

            int floor_x = floor(x / (float)m_screen_data.m_tile_size) * m_screen_data.m_tile_size;
            int floor_y = floor(y / (float)m_screen_data.m_tile_size) * m_screen_data.m_tile_size;
            int ceil_x = ceil(x / (float)m_screen_data.m_tile_size) * m_screen_data.m_tile_size;
            int ceil_y = ceil(y / (float)m_screen_data.m_tile_size) * m_screen_data.m_tile_size;

            // skip corners, where we pull values from
            if(floor_x == x && ceil_x == x && floor_y == y && ceil_y == y){
                continue;
            }

            // clamp to canvas
            if(floor_x < 0){
                floor_x = 0;
            }
            else if(ceil_x >= m_screen_data.m_canvas_width){
                ceil_x = m_screen_data.m_canvas_width - 1;
            }
            if(floor_y < 0){
                floor_y = 0;
            }
            else if(ceil_y >= m_screen_data.m_canvas_height){
                ceil_y = m_screen_data.m_canvas_height - 1;
            }

            sf::Uint8 top_floor_col = m_screen_data.m_generation_canvas[0][m_current_tile_layer].getPixel(floor_x, floor_y).r;
            sf::Uint8 top_ceil_col = m_screen_data.m_generation_canvas[0][m_current_tile_layer].getPixel(ceil_x, floor_y).r;
            
            sf::Uint8 bot_floor_col = m_screen_data.m_generation_canvas[0][m_current_tile_layer].getPixel(floor_x, ceil_y).r;
            sf::Uint8 bot_ceil_col = m_screen_data.m_generation_canvas[0][m_current_tile_layer].getPixel(ceil_x, ceil_y).r;

            
            sf::Uint8 final_col = Calc::Lerp(
                Calc::Lerp(top_floor_col, top_ceil_col, (x - floor_x) / (float)m_screen_data.m_tile_size),
                Calc::Lerp(bot_floor_col, bot_ceil_col, (x - floor_x) / (float)m_screen_data.m_tile_size),
                (y - floor_y) / (float)m_screen_data.m_tile_size);
                

            m_screen_data.m_generation_canvas[0][m_current_tile_layer].setPixel(x, y, 
                        sf::Color(final_col, 0, 0, 
                                                m_screen_data.m_generation_canvas[0][m_current_tile_layer].getPixel(x,y).a));


        }
    }
}

void Editor::SetTileMaterial(int tile_x, int tile_y){

    // remove exisiting voxel material if present
    if(m_screen_data.m_tile_layers[m_current_tile_layer][tile_x][tile_y].is_topleft_of_object){
        
        auto voxel_obj = AssetManager::GetVoxelObject(m_screen_data.m_tile_layers[m_current_tile_layer][tile_x][tile_y].voxel_object_index);


        for(int x = 0; x < voxel_obj.tile_width; x++){
            for(int y = 0; y < voxel_obj.tile_height; y++){

                m_screen_data.m_tile_layers[m_current_tile_layer][tile_x + x][tile_y + x].voxel_object_index = -1;
                m_screen_data.m_tile_layers[m_current_tile_layer][tile_x + y][tile_y + y].is_topleft_of_object = false;
            }
        }
    }
    // set tile
    m_screen_data.m_tile_layers[m_current_tile_layer][tile_x][tile_y].tile_material_index = selected_material;
}
void Editor::SetVoxelObject(int tile_x, int tile_y){
    
    auto voxel_obj = AssetManager::GetVoxelObject(selected_material);

    // mark each tile with appropriate voxel index
    for(int x = 0; x < voxel_obj.tile_width; x++){
        for(int y = 0; y < voxel_obj.tile_height; y++){

            m_screen_data.m_tile_layers[m_current_tile_layer][tile_x + x][tile_y + y].occupied = true;
            m_screen_data.m_tile_layers[m_current_tile_layer][tile_x + x][tile_y + y].voxel_object_index = selected_material;
            m_screen_data.m_tile_layers[m_current_tile_layer][tile_x + x][tile_y + y].is_topleft_of_object = false;
        }
    }

    // mark top corner as origin 
    m_screen_data.m_tile_layers[m_current_tile_layer][tile_x][tile_y].is_topleft_of_object = true;
}
void Editor::SetTileObject(int tile_x, int tile_y){
    
    auto tile_obj = AssetManager::GetTileObject(selected_material);

    // mark each tile with appropriate voxel index
    for(int x = 0; x < tile_obj.tile_width; x++){
        for(int y = 0; y < tile_obj.tile_height; y++){
            m_screen_data.m_tile_layers[m_current_tile_layer][tile_x + x][tile_y + y].occupied = true;
            m_screen_data.m_tile_layers[m_current_tile_layer][tile_x + x][tile_y + y].tile_object_index = selected_material;
            m_screen_data.m_tile_layers[m_current_tile_layer][tile_x + x][tile_y + y].is_topleft_of_object = false;
        }
    }

    // mark top corner as origin 
    m_screen_data.m_tile_layers[m_current_tile_layer][tile_x][tile_y].is_topleft_of_object = true;
}

void Editor::MouseHandling(){

    // fetch mouse pos
    sf::Vector2i mouse_pos = sf::Mouse::getPosition(m_window);
    
    // snapping position to tilesize grid
    sf::Vector2f mouse_position_snapped = sf::Vector2f(round(mouse_pos.x / (float)m_screen_data.m_tile_size) * m_screen_data.m_tile_size,
                                                  round(mouse_pos.y / (float)m_screen_data.m_tile_size) * m_screen_data.m_tile_size);


    // where the top left of the canvas is located
    sf::Vector2f canvas_position = sf::Vector2f(m_window.getSize().x / 2.0f - m_screen_data.m_canvas.getTexture().getSize().x / 2.0f, 
                                                m_window.getSize().y / 2.0f - m_screen_data.m_canvas.getTexture().getSize().x / 2.0f);

    sf::Vector2f canvas_tilesize_offset = canvas_position - sf::Vector2f(floor(canvas_position.x / (float)m_screen_data.m_tile_size) * m_screen_data.m_tile_size,
                                                                        floor(canvas_position.y / (float)m_screen_data.m_tile_size) * m_screen_data.m_tile_size);

    // shifted to fit on canvas grid, find the tilesize offset of said canvas_position
    m_mouse_position = mouse_position_snapped + canvas_tilesize_offset;

    m_canvas_position_free = sf::Vector2f(mouse_pos.x, mouse_pos.y) - sf::Vector2f(m_window.getSize().x / 2.0f - m_screen_data.m_half_canvas_width,
                                                                                m_window.getSize().y / 2.0f - m_screen_data.m_half_canvas_height);


    m_canvas_coordinate = m_mouse_position - sf::Vector2f(m_window.getSize().x / 2.0f - m_screen_data.m_half_canvas_width,
                                                                                m_window.getSize().y / 2.0f - m_screen_data.m_half_canvas_height);

    cursor_brush_circle.setPosition(sf::Vector2f(mouse_pos.x, mouse_pos.y) + canvas_tilesize_offset);


    m_tile_coord = sf::Vector2i(round(m_canvas_coordinate.x / (float)m_screen_data.m_tile_size),
                                                  round(m_canvas_coordinate.y / (float)m_screen_data.m_tile_size));




    if(m_view_mode == TileMode::Generation){

        if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)){
            PaintToGenerationImage(m_tile_coord.x * m_screen_data.m_tile_size, m_tile_coord.y * m_screen_data.m_tile_size, 255, m_brush_size);
        }
        if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)){
            PaintToGenerationImage(m_tile_coord.x * m_screen_data.m_tile_size, m_tile_coord.y * m_screen_data.m_tile_size, 0, m_brush_size);
        }
    }



    if(m_tile_tool == TileTool::BRUSH){
        // coordinate is within bounds
        if(m_tile_coord.x >= 0 && m_tile_coord.x < m_screen_data.m_tile_layers[m_current_tile_layer].size()){
            if(m_tile_coord.y >= 0 && m_tile_coord.y < m_screen_data.m_tile_layers[m_current_tile_layer][0].size()){
                
                // we are editing materials
                if(m_view_mode == TileMode::Material){

                        
                    switch(m_material_select_type){

                        case MaterialSelectionType::TILE_MATERIALS: {
                            if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)){
                                SetTileMaterial(m_tile_coord.x, m_tile_coord.y);
                            }
                            break;
                        }
                        case MaterialSelectionType::VOXEL_OBJECTS: {
                            // drawing voxel
                            if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && 
                                Util::ValidateVoxelObjectFitsTileGrid(m_screen_data, m_tile_coord.x, m_tile_coord.y, m_current_tile_layer, 
                                                                AssetManager::GetVoxelObject(selected_material).tile_width, 
                                                                AssetManager::GetVoxelObject(selected_material).tile_height, !m_material_creates_geometry)){ 
                                SetVoxelObject(m_tile_coord.x, m_tile_coord.y);
                            }
                            break;
                        }
                        case MaterialSelectionType::TILE_OBJECTS: {
                            auto tile_objects = AssetManager::GetAllTileObjects();
                            // drawing voxel
                            if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && 
                                Util::ValidateVoxelObjectFitsTileGrid(m_screen_data, m_tile_coord.x, m_tile_coord.y, m_current_tile_layer, 
                                                                AssetManager::GetTileObject(selected_material).tile_width, 
                                                                AssetManager::GetTileObject(selected_material).tile_height, !m_material_creates_geometry)){
                                SetTileObject(m_tile_coord.x, m_tile_coord.y);
                            }
                            break;
                        }
                    }

                }
                else if(m_view_mode == TileMode::Tiles){ // we are editing tiles
                    if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)){
                        // set tile
                        SetTileOccupied(m_tile_coord.x, m_tile_coord.y, true);

                    }
                    else if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)){
                        // remove tile
                        SetTileOccupied(m_tile_coord.x, m_tile_coord.y, false);
                    }
                }
            }
        }
    }
}

void Editor::SetTileOccupied(int x, int y, bool occupied){
    m_screen_data.m_tile_layers[m_current_tile_layer][x][y].occupied = occupied;

    // should we set in the opposing sides? (symmetry)
    if(m_symmetric_x && m_symmetric_y){
        m_screen_data.m_tile_layers[m_current_tile_layer][Util::GetSymmetricPositionX(m_screen_data, x)][Util::GetSymmetricPositionY(m_screen_data, y)].occupied = occupied;
    }
    if(m_symmetric_x){
        m_screen_data.m_tile_layers[m_current_tile_layer][Util::GetSymmetricPositionX(m_screen_data, x)][y].occupied = occupied;
    }
    if(m_symmetric_y){
        m_screen_data.m_tile_layers[m_current_tile_layer][x][Util::GetSymmetricPositionY(m_screen_data, y)].occupied = occupied;
    }
}

void Editor::CreateRect(bool remove){

    sf::Vector2i start = m_tile_coord_inital;
    sf::Vector2i end = m_tile_coord;

    if(start.x > end.x){
        int temp = start.x;
        start.x = end.x;
        end.x = temp;
    }
    if(start.y > end.y){
        int temp = start.y;
        start.y = end.y;
        end.y = temp;
    }

    start.x = Calc::Clamp(start.x, 0, m_screen_data.m_tile_layers[m_current_tile_layer].size());
    start.y = Calc::Clamp(start.y, 0, m_screen_data.m_tile_layers[m_current_tile_layer][0].size());
    end.x = Calc::Clamp(end.x, 0, m_screen_data.m_tile_layers[m_current_tile_layer].size());
    end.y = Calc::Clamp(end.y, 0, m_screen_data.m_tile_layers[m_current_tile_layer][0].size());


    if(m_view_mode == TileMode::Material){ // we are drawing materials

        if(remove){ // cannot remove materials
            return;
        }

        VoxelObject voxel_object;
        TileObject tile_object;

        int width_thresh = 0;
        int height_thresh = 0;

        if(m_material_select_type == VOXEL_OBJECTS){
            voxel_object = AssetManager::GetVoxelObject(selected_material);
            width_thresh = voxel_object.tile_width;
            height_thresh = voxel_object.tile_height;
        }
        else if(m_material_select_type == TILE_OBJECTS){
            tile_object = AssetManager::GetTileObject(selected_material);
            width_thresh = tile_object.tile_width;
            height_thresh = tile_object.tile_height;
        }

        int width_increment = 0;
        int height_increment = 0;


        for(int x = start.x; x < end.x; x++){
            height_increment = 0;
            for(int y = start.y; y < end.y; y++){

                if(m_material_select_type == TILE_MATERIALS){

                    SetTileMaterial(x, y);
                    continue;
                }

                if(width_increment >= width_thresh){
                    width_increment = 0;
                }
                if(height_increment >= height_thresh){
                    height_increment = 0;
                }

                if(height_increment == 0 && width_increment == 0){

                    if(m_material_select_type == VOXEL_OBJECTS && 
                        Util::ValidateVoxelObjectFitsTileGrid(m_screen_data, x, y, m_current_tile_layer, width_thresh, height_thresh, !m_material_creates_geometry)){
                            SetVoxelObject(x, y);
                    }
                    else if(m_material_select_type == TILE_OBJECTS &&
                        Util::ValidateVoxelObjectFitsTileGrid(m_screen_data, x, y, m_current_tile_layer, width_thresh, height_thresh, !m_material_creates_geometry)){
                            SetTileObject(x, y);
                    }

                }
                
            

                height_increment++;
                // only allow rectangle tool on 1x1 voxel materials
                //else if(m_material_select_type == VOXEL_OBJECTS && voxel_objects[selected_material]->tile_height == 1 && voxel_objects[selected_material]->tile_width == 1){
                //    SetVoxelObject(x, y);
                //}
                //else if(m_material_select_type == TILE_OBJECTS && tile_objects[selected_material]->tile_height == 1 && tile_objects[selected_material]->tile_width == 1){
                //    SetTileObject(x, y);
                //}
            }
            width_increment++;
        }
    }
    else{ // we are drawing tiles
        for(int x = start.x; x < end.x; x++){
            for(int y = start.y; y < end.y; y++){

                SetTileOccupied(x, y, !remove);
            }
        }
    }
    
}

void Editor::DrawEditor(){
    m_screen_data.m_general_surface.clear(sf::Color::Transparent);

    switch(m_view_mode){

        case TileMode::Tiles: {
            DrawTileGuides(m_screen_data.m_general_surface);
            break;
        }
        case TileMode::Material: {
            DrawMaterialGuides(m_screen_data.m_general_surface);
            break;
        }
        case TileMode::Rope: {
            DrawRopeGuides(m_screen_data.m_general_surface);
            break;
        }
        case TileMode::Generation: {
            DrawGenerationGuides(m_screen_data.m_general_surface);
            break;
        }
        case TileMode::Lightmap: {
            DrawLightmapGuides(m_screen_data.m_general_surface);
        }
    }

    if(m_tile_tool == TileTool::BRUSH || (m_tile_tool == TileTool::RECTANGLE && !m_drawing_place_rect && !m_drawing_remove_rect) ){
        cursor_outline.setSize(sf::Vector2f(m_screen_data.m_tile_size, m_screen_data.m_tile_size));
        cursor_outline.setPosition(m_mouse_position);
    
    }
    else if(m_drawing_place_rect || m_drawing_remove_rect){

        sf::Vector2f start = m_mouse_position_inital;
        sf::Vector2f end = m_mouse_position;

        if(start.x > end.x){
            int temp = start.x;
            start.x = end.x;
            end.x = temp;
        }
        if(start.y > end.y){
            int temp = start.y;
            start.y = end.y;
            end.y = temp;
        }
        
        cursor_outline.setSize(sf::Vector2f(end.x - start.x, end.y - start.y));
        cursor_outline.setPosition(start);
    }



    // draw canvas

    sf::Sprite canvas_sprite(m_screen_data.m_canvas.getTexture());

    canvas_sprite.setOrigin(sf::Vector2f(floor(m_screen_data.m_canvas_width / 2.0f), floor(m_screen_data.m_canvas_height / 2.0f)));
    canvas_sprite.setPosition(floor(m_window.getSize().x / 2.0f), floor(m_window.getSize().y / 2.0f));
    canvas_outline.setPosition(canvas_sprite.getPosition());


    if(m_view_mode == TileMode::Voxel){
        //Renderer::Update(m_screen_data, m_window);
        m_window.draw(canvas_sprite);
    }

    // draw general surface
    m_screen_data.m_general_surface.display();
    canvas_sprite.setTexture(m_screen_data.m_general_surface.getTexture());
    m_window.draw(canvas_outline);
    
    m_window.draw(canvas_sprite);

    if(m_view_mode == TileMode::Rope){
        cursor_brush_circle.setRadius(2);
        cursor_brush_circle.setOrigin(sf::Vector2f(2, 2));

        cursor_brush_circle.setOutlineColor(sf::Color::White);

        m_window.draw(cursor_brush_circle);
    }
    if(m_view_mode == TileMode::Material){

        // draw object ui
        if(m_material_select_type == VOXEL_OBJECTS){
            auto voxel_obj = AssetManager::GetVoxelObject(selected_material);
            voxel_obj.sprite_texture.setPosition(m_mouse_position);
            m_window.draw(voxel_obj.sprite_texture);
            
            if(m_tile_tool == TileTool::RECTANGLE && m_drawing_place_rect){
                m_window.draw(cursor_outline);
            }
        }
        if(m_material_select_type == TILE_OBJECTS){
            auto tile_obj = AssetManager::GetTileObject(selected_material);
            tile_obj.sprite_texture.setPosition(m_mouse_position);
            m_window.draw(tile_obj.sprite_texture);

            if(m_tile_tool == TileTool::RECTANGLE && m_drawing_place_rect){
                m_window.draw(cursor_outline);
            }
        }
        if(m_material_select_type == TILE_MATERIALS){
            m_window.draw(cursor_outline);
        }
    }
    else if(m_view_mode == TileMode::Generation){
        cursor_brush_circle.setRadius(m_brush_size * m_screen_data.m_tile_size);
        cursor_brush_circle.setOrigin(sf::Vector2f(m_brush_size * m_screen_data.m_tile_size, m_brush_size * m_screen_data.m_tile_size));

        cursor_brush_circle.setOutlineColor(sf::Color::White);

        m_window.draw(cursor_brush_circle);
    }
    else if(m_view_mode != TileMode::Lightmap){
        m_window.draw(cursor_outline);
    }
    
}

void Editor::CatchEvent(const sf::Event& event){

    if(event.type == sf::Event::KeyPressed){
        if(event.key.scancode == sf::Keyboard::Scancode::Escape){
            palette_preview = !palette_preview;

            if(palette_preview){
                palette_preview_texture.loadFromFile("LevelExports/export_main.png");
                palette_preview_sprite.setTexture(palette_preview_texture);
            }
        }
    }

    // palette preview cycle
    if(palette_preview){

        if(event.type == sf::Event::KeyPressed){
            if(event.key.scancode == sf::Keyboard::Scancode::E){

                selected_palette--;

                if(selected_palette < 0){
                    selected_palette = AssetManager::GetPalettes().size() - 1;
                }
            }
            if(event.key.scancode == sf::Keyboard::Scancode::D){

                selected_palette++;

                if(selected_palette >= AssetManager::GetPalettes().size()){
                    selected_palette = 0;
                }
            }
        }
        // return !!!!!!!!!!!!!!!
        return;
    }


    // adjusting generation brush size
    if(m_view_mode == TileMode::Generation){
        if(event.type == sf::Event::MouseWheelScrolled){
            
            if(event.mouseWheelScroll.delta > 0 && m_brush_size < 20){
                m_brush_size += 1;
            }
            else if(event.mouseWheelScroll.delta < 0 && m_brush_size > 1){
                m_brush_size -= 1;
            }
        }
    }

    if(m_view_mode == TileMode::Lightmap){
        if(event.type == sf::Event::MouseWheelScrolled){
            
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode(sf::Keyboard::Scancode::LShift))){
                if(event.mouseWheelScroll.delta > 0){
                    lightmap_sprite_scale_x += 0.35;
                }
                else if(event.mouseWheelScroll.delta < 0){
                    lightmap_sprite_scale_x -= 0.35;
                }
            }
            else{
                if(event.mouseWheelScroll.delta > 0){
                    lightmap_sprite_scale_y += 0.35;
                }
                else if(event.mouseWheelScroll.delta < 0){
                    lightmap_sprite_scale_y -= 0.35;
                }
            }
        }
    }

    if(m_view_mode == TileMode::Rope){
        if(event.type == sf::Event::MouseButtonPressed){

            if(event.mouseButton.button == sf::Mouse::Button::Left){

                if(rope_being_created == nullptr){
                    m_screen_data.m_ropes[m_current_tile_layer].push_back({m_canvas_coordinate, sf::Vector2f(0,0)});
                    rope_being_created = &m_screen_data.m_ropes[m_current_tile_layer][m_screen_data.m_ropes[m_current_tile_layer].size() - 1];
                }
            }
            if(event.mouseButton.button == sf::Mouse::Button::Middle){

                for(auto& rope : m_screen_data.m_ropes[m_current_tile_layer]){

                    if(Calc::Distance(rope.start, m_canvas_coordinate) < 8){
                        rope_being_moved = &rope;
                        rope_position_being_moved = &rope.start; 
                        break;
                    }

                    else if(Calc::Distance(rope.end, m_canvas_coordinate) < 8){
                        rope_being_moved = &rope;
                        rope_position_being_moved = &rope.end; 
                        break;
                    }

                }
            }
            if(event.mouseButton.button == sf::Mouse::Button::Right){

                int i = 0; 
                for(auto& rope : m_screen_data.m_ropes[m_current_tile_layer]){

                    if(Calc::Distance(rope.start, m_canvas_coordinate) < 8){
                        m_screen_data.m_ropes[m_current_tile_layer].erase(m_screen_data.m_ropes[m_current_tile_layer].begin() + i);
                        break;
                    }

                    else if(Calc::Distance(rope.end, m_canvas_coordinate) < 8){
                        m_screen_data.m_ropes[m_current_tile_layer].erase(m_screen_data.m_ropes[m_current_tile_layer].begin() + i);
                        break;
                    }
                    i++;
                }
            }
        }
        if(event.type == sf::Event::MouseButtonReleased){

            if(event.mouseButton.button == sf::Mouse::Button::Left){
                
                rope_being_created = nullptr;
            }
            if(event.mouseButton.button == sf::Mouse::Button::Middle){

                rope_position_being_moved = nullptr;
                rope_being_moved = nullptr;
            }
        }

    }

    else if(m_tile_tool == TileTool::RECTANGLE && m_view_mode == TileMode::Tiles || m_view_mode == TileMode::Material){
        if(event.type == sf::Event::MouseButtonPressed){

            if(event.mouseButton.button == sf::Mouse::Button::Left){
                m_drawing_place_rect = true;
                m_drawing_remove_rect = false;
                m_tile_coord_inital = m_tile_coord;
                m_mouse_position_inital = m_mouse_position;
            }
            else if(event.mouseButton.button == sf::Mouse::Button::Right){
                m_drawing_remove_rect = true;
                m_drawing_place_rect = false;
                m_tile_coord_inital = m_tile_coord;
                m_mouse_position_inital = m_mouse_position;
            }
        }
        if(event.type == sf::Event::MouseButtonReleased){

            if(event.mouseButton.button == sf::Mouse::Button::Left){
                
                if(m_drawing_place_rect){
                    CreateRect(false);
                }
                m_drawing_place_rect = false;
                m_drawing_remove_rect = false;
            }
            else if(event.mouseButton.button == sf::Mouse::Button::Right){
                if(m_drawing_remove_rect){
                    CreateRect(true);
                }
                m_drawing_place_rect = false;
                m_drawing_remove_rect = false;
            }
        }
    }

    if(event.type == sf::Event::KeyPressed){


        // switching view modes
        if(event.key.scancode == sf::Keyboard::Scancode::Up){
            m_view_mode--;
            if(m_view_mode < 0){
                m_view_mode = 0;
                // dont allow vertical wrap around (from tile->renderer )
            }
            if(m_view_mode == TileMode::Voxel){
                Renderer::StartRender();
                Renderer::DrawScreenData(m_screen_data, m_window);
            }
            else{
                Renderer::StopRender();
            }
            
        }
        if(event.key.scancode == sf::Keyboard::Scancode::Down){
            m_view_mode++;
            // dont allow vertical wrap around (from renderer->tile )
            if(m_view_mode >= NUMBER_OF_VIEWMODES){
                m_view_mode = NUMBER_OF_VIEWMODES - 1;
            }

            if(m_view_mode == TileMode::Voxel){
                Renderer::StartRender();
                Renderer::DrawScreenData(m_screen_data, m_window);
            }
            else{
                Renderer::StopRender();
            }
            
        }
        else if(event.key.scancode == sf::Keyboard::Scancode::Num1){
            m_symmetric_x = !m_symmetric_x;
        }
        else if(event.key.scancode == sf::Keyboard::Scancode::Num2){
            m_symmetric_y = !m_symmetric_y;
        }
        else if(event.key.scancode == sf::Keyboard::Scancode::Num3){
            m_material_creates_geometry = !m_material_creates_geometry;
        }
        else if(event.key.scancode == sf::Keyboard::Scancode::B){
            m_tile_tool = TileTool::BRUSH;
        }
        else if(event.key.scancode == sf::Keyboard::Scancode::R){
            m_tile_tool = TileTool::RECTANGLE;
        }

        else if(event.key.scancode == sf::Keyboard::Scancode::Tab){
            m_material_select_type++;
            selected_material = 0;

            if(m_material_select_type >= NUMBER_OF_MATERIAL_SELECT_TYPES){
                m_material_select_type = 0;
            }
        }


        if(m_view_mode == TileMode::Lightmap){

            if(event.key.scancode == sf::Keyboard::Scancode::E){
                selected_lightmap--;
                lightmap_sprite_scale_y = 1.0;
                lightmap_sprite_scale_x = 1.0;
                lightmap_sprite_rotation = 0;
            }
            if(event.key.scancode == sf::Keyboard::Scancode::D){
                selected_lightmap++;
                lightmap_sprite_scale_y = 1.0;
                lightmap_sprite_scale_x = 1.0;
                lightmap_sprite_rotation = 0;
            }

            if(event.key.scancode == sf::Keyboard::Scancode::C){
                m_screen_data.m_light_map.clear(sf::Color::White);
            }
            if(event.key.scancode == sf::Keyboard::Scancode::F){
                m_screen_data.m_light_map.clear(sf::Color(80,80,80));
            }

            if(selected_lightmap >= AssetManager::GetLightShapes().size()){
                selected_lightmap = 0;
            }
            else if(selected_lightmap < 0){
                selected_lightmap = AssetManager::GetLightShapes().size() - 1;
            }

            
        }


        // cycling through material items
        if(m_view_mode == TileMode::Material){
            // direction of shift
            int shift_material_select_type = 0;
            if(event.key.scancode == sf::Keyboard::Scancode::E){
                shift_material_select_type = -1;
            }
            if(event.key.scancode == sf::Keyboard::Scancode::D){
                shift_material_select_type = 1;
            }
            // marked as not 0 be "D" or "E"
            if(shift_material_select_type != 0){

                std::vector<std::string> names = {};

                switch(m_material_select_type){

                    case TILE_MATERIALS: {
                        names = AssetManager::GetAllTileMaterialNames();
                        break;
                    }
                    case TILE_OBJECTS: {
                        names = AssetManager::GetAllTileObjectNames();
                        break;
                    }
                    case VOXEL_OBJECTS: {
                        names = AssetManager::GetAllVoxelObjectNames();
                        break;
                    }
                }

                selected_material += shift_material_select_type;

                if(selected_material >= names.size()){
                    selected_material = 0;
                }
                else if(selected_material < 0){
                    selected_material = names.size() - 1;
                }
            }
        }

        bool switch_layer = false;
        // up layer
        if(event.key.scancode == sf::Keyboard::Scancode::W){
            m_current_tile_layer--;
            switch_layer = true;
        }
        // down layer
        else if(event.key.scancode == sf::Keyboard::Scancode::S){
            m_current_tile_layer++;
            switch_layer = true;
        }

        // modulo new layer value
        if(switch_layer){
            if(m_current_tile_layer < 0){
                m_current_tile_layer = m_screen_data.m_tile_layers.size() - 1;
            }
            else if(m_current_tile_layer >= m_screen_data.m_tile_layers.size()){
                m_current_tile_layer = 0;
            }
        }


    }

}