#include "Editor.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "Util.h"

void Editor::Start(){

    if(!m_font.loadFromFile("Assets/Roboto-Medium.ttf")){
        std::cout << "[ERROR]: Failed to load font, Editor::Start()\n";
    }
    m_text.setFont(m_font);
    m_text.setCharacterSize(24);

    m_camera.m_perspective_factor = 0.1;


    m_window.setMouseCursorVisible(false);

    m_screen_data.Create(40, 26, 16, 10);

    canvas_outline.setOutlineColor(sf::Color::White);
    canvas_outline.setOutlineThickness(1);
    canvas_outline.setFillColor(sf::Color::Transparent);
    canvas_outline.setSize(sf::Vector2f(m_screen_data.m_canvas_width, m_screen_data.m_canvas_height));
    canvas_outline.setOrigin(sf::Vector2f(m_screen_data.m_canvas_width / 2.0f, m_screen_data.m_canvas_height / 2.0f));

    cursor_outline.setOutlineColor(sf::Color::White);
    cursor_outline.setOutlineThickness(2);
    cursor_outline.setSize(sf::Vector2f(m_screen_data.m_tile_size, m_screen_data.m_tile_size));
    cursor_outline.setFillColor(sf::Color(0,0,15, 50));

    m_current_tile_layer = 0;
    m_view_mode = TileMode::Tiles;
    m_tile_tool = TileTool::BRUSH;



    Renderer::Init();
    AssetManager::Init(m_screen_data);

}


void Editor::Update(){

    Drawing();
    MouseHandling();
    DrawText();
}

void Editor::DrawText(){
    
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

    if(Renderer::m_demo_fog){
        m_text.setColor(sf::Color::Green);
    }
    else{
        m_text.setColor(sf::Color::White);
    }
    m_text.setString("(F) Fog");
    m_text.setPosition(sf::Vector2f(32, 152));
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
        auto materials = AssetManager::GetAllTileMaterialNames();
        
        
        if(looking_at_voxel_materials){
            materials = AssetManager::GetAllVoxelMaterialNames();
            m_text.setString("(E/D) Voxel Material");
        }
        else{
            m_text.setString("(E/D) Tile Material");
        }
        m_text.setPosition(sf::Vector2f(260, 32 ));
        m_window.draw(m_text);

        m_text.setString("(Tab) Switch Material Mode");
        m_text.setPosition(sf::Vector2f(260, 64 ));
        m_window.draw(m_text);



        for(int i = 0; i < materials.size(); i++){

            m_text.setString(materials[i]);

            if((i == selected_voxel_material && looking_at_voxel_materials) || (i == selected_tile_material && !looking_at_voxel_materials)){
                m_text.setColor(sf::Color::Green);

            }
            else{
                m_text.setColor(sf::Color::White);

            }

            m_text.setPosition(sf::Vector2f(260, 32 * (i + 4)));

            m_window.draw(m_text);
        }
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

    auto voxel_materials = AssetManager::GetAllVoxelMaterials();

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


        for(int x = 0; x < m_screen_data.m_tile_layers[i].size(); x++){         
            for(int y = 0; y < m_screen_data.m_tile_layers[i].size(); y++){
                
                // is there a tile
                if(!m_screen_data.m_tile_layers[i][x][y].occupied){
                    continue;
                }
                // are we on the selected layer?
                if(i != m_current_tile_layer){
                    continue;
                }
                // is part of a voxel material
                if(m_screen_data.m_tile_layers[i][x][y].voxel_material_index != -1 && !m_screen_data.m_tile_layers[i][x][y].is_topleft_of_voxel_material){
                    continue;
                }

                float _x = x * m_screen_data.m_tile_size;
                float _y = y * m_screen_data.m_tile_size;


                // no voxel
                if(m_screen_data.m_tile_layers[i][x][y].voxel_material_index == -1){

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
                // draw voxel sprite
                else if(m_screen_data.m_tile_layers[i][x][y].is_topleft_of_voxel_material){
                    voxel_materials[m_screen_data.m_tile_layers[i][x][y].voxel_material_index]->sprite_texture.setPosition(sf::Vector2f(_x, _y));
                    surface.draw(voxel_materials[m_screen_data.m_tile_layers[i][x][y].voxel_material_index]->sprite_texture);
                }
            }
        }
        surface.draw(tile_guides);
    }

}

void Editor::DrawRopeGuides(sf::RenderTarget& surface){

    // move rope being created to mouse pos
    if(rope_being_created != nullptr){
        
        rope_being_created->end = m_canvas_coordinate;
        
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::E)){
            rope_being_created->slack -= 1.3f;
        }    
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)){
            rope_being_created->slack += 1.3f;
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

        for(auto& rope : m_screen_data.m_ropes[i]){

            auto positions = rope.SamplePositions();

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

void Editor::SetTileMaterial(int tile_x, int tile_y){

    // remove exisiting voxel material if present
    if(m_screen_data.m_tile_layers[m_current_tile_layer][tile_x][tile_y].is_topleft_of_voxel_material){
        
        auto voxel_materials = AssetManager::GetAllVoxelMaterials();
        int voxel_material_index = m_screen_data.m_tile_layers[m_current_tile_layer][tile_x][tile_y].voxel_material_index;

        for(int x = 0; x < voxel_materials[voxel_material_index]->tile_width; x++){
            for(int y = 0; y < voxel_materials[voxel_material_index]->tile_height; y++){

                m_screen_data.m_tile_layers[m_current_tile_layer][tile_x + x][tile_y + x].voxel_material_index = -1;
                m_screen_data.m_tile_layers[m_current_tile_layer][tile_x + y][tile_y + y].is_topleft_of_voxel_material = false;
            }
        }
    }
    // set tile
    m_screen_data.m_tile_layers[m_current_tile_layer][tile_x][tile_y].tile_material_index = selected_tile_material;
}
void Editor::SetVoxelMaterial(int tile_x, int tile_y){
    
    auto voxel_materials = AssetManager::GetAllVoxelMaterials();

    // mark each tile with appropriate voxel index
    for(int x = 0; x < voxel_materials[selected_voxel_material]->tile_width; x++){
        for(int y = 0; y < voxel_materials[selected_voxel_material]->tile_height; y++){
            m_screen_data.m_tile_layers[m_current_tile_layer][tile_x + x][tile_y + y].voxel_material_index = selected_voxel_material;
            m_screen_data.m_tile_layers[m_current_tile_layer][tile_x + x][tile_y + y].is_topleft_of_voxel_material = false;
        }
    }

    // mark top corner as origin 
    m_screen_data.m_tile_layers[m_current_tile_layer][tile_x][tile_y].is_topleft_of_voxel_material = true;
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



    m_canvas_coordinate = m_mouse_position - sf::Vector2f(m_window.getSize().x / 2.0f - m_screen_data.m_half_canvas_width,
                                                                                m_window.getSize().y / 2.0f - m_screen_data.m_half_canvas_height);

    m_tile_coord = sf::Vector2i(round(m_canvas_coordinate.x / (float)m_screen_data.m_tile_size),
                                                  round(m_canvas_coordinate.y / (float)m_screen_data.m_tile_size));



    if(m_view_mode == TileMode::Material){
        // draw voxel material UI visual at mouse
        if(looking_at_voxel_materials){
            auto voxel_materials = AssetManager::GetAllVoxelMaterials();
            voxel_materials[selected_voxel_material]->sprite_texture.setPosition(m_mouse_position);

            m_window.draw(voxel_materials[selected_voxel_material]->sprite_texture);
        }
    }





    if(m_tile_tool == TileTool::BRUSH){
        // coordinate is within bounds
        if(m_tile_coord.x >= 0 && m_tile_coord.x < m_screen_data.m_tile_layers[m_current_tile_layer].size()){
            if(m_tile_coord.y >= 0 && m_tile_coord.y < m_screen_data.m_tile_layers[m_current_tile_layer][0].size()){
                
                // we are editing materials
                if(m_view_mode == TileMode::Material){

                    if(looking_at_voxel_materials){
                        auto voxel_materials = AssetManager::GetAllVoxelMaterials();
                        // drawing voxel
                        if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && Util::ValidateVoxelMaterialFitsTileGrid(m_screen_data, m_tile_coord.x, m_tile_coord.y, m_current_tile_layer, *voxel_materials[selected_voxel_material])){
                            SetVoxelMaterial(m_tile_coord.x, m_tile_coord.y);
                        }
                    }
                
                    else{
                        if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)){
                            SetTileMaterial(m_tile_coord.x, m_tile_coord.y);
                        }
                    }

                }
                else if(m_view_mode == TileMode::Tiles){ // we are editing tiles
                    if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)){
                        // set tile
                        m_screen_data.m_tile_layers[m_current_tile_layer][m_tile_coord.x][m_tile_coord.y].occupied = true;
                    }
                    else if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)){
                        // remove tile
                        m_screen_data.m_tile_layers[m_current_tile_layer][m_tile_coord.x][m_tile_coord.y].occupied = false;
                    }
                }
            }
        }
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

        auto voxel_materials = AssetManager::GetAllVoxelMaterials();

        for(int x = start.x; x < end.x; x++){
            for(int y = start.y; y < end.y; y++){

                if(!looking_at_voxel_materials){

                    SetTileMaterial(x, y);
                }
                // only allow rectangle tool on 1x1 voxel materials
                else if(voxel_materials[selected_voxel_material]->tile_height == 1 && voxel_materials[selected_voxel_material]->tile_width == 1){
                    SetVoxelMaterial(x, y);
                }

            }
        }
    }
    else{ // we are drawing tiles
        for(int x = start.x; x < end.x; x++){
            for(int y = start.y; y < end.y; y++){

                m_screen_data.m_tile_layers[m_current_tile_layer][x][y].occupied = !remove;
            }
        }
    }
    
}

void Editor::Drawing(){
    m_window.clear(sf::Color(161, 163, 173));
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
    canvas_sprite.setOrigin(sf::Vector2f(m_screen_data.m_canvas_width / 2.0f, m_screen_data.m_canvas_height / 2.0f));

    canvas_sprite.setPosition(m_window.getSize().x / 2.0f, m_window.getSize().y / 2.0f);

    canvas_outline.setPosition(canvas_sprite.getPosition());

    if(m_view_mode == TileMode::Voxel){
        canvas_sprite.setScale(sf::Vector2f(2.0f, 2.0f));
        m_window.draw(canvas_sprite);
    }
    else{
        canvas_sprite.setScale(sf::Vector2f(1.0f, 1.0f));
    }

    // draw general surface
    m_screen_data.m_general_surface.display();
    canvas_sprite.setTexture(m_screen_data.m_general_surface.getTexture());
    m_window.draw(canvas_outline);
    
    m_window.draw(canvas_sprite);

    m_window.draw(cursor_outline);
    
}



void Editor::CatchEvent(const sf::Event& event){

    if(m_view_mode == TileMode::Rope){
        if(event.type == sf::Event::MouseButtonPressed){

            if(event.mouseButton.button == sf::Mouse::Button::Left){

                if(rope_being_created == nullptr){
                    m_screen_data.m_ropes[m_current_tile_layer].push_back({m_canvas_coordinate, sf::Vector2f(0,0)});
                    rope_being_created = &m_screen_data.m_ropes[m_current_tile_layer][m_screen_data.m_ropes[m_current_tile_layer].size() - 1];
                }

            }
        }
        if(event.type == sf::Event::MouseButtonReleased){

            if(event.mouseButton.button == sf::Mouse::Button::Left){
                
                rope_being_created = nullptr;
            }
        }

    }

    else if(m_tile_tool == TileTool::RECTANGLE){
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
                m_view_mode = NUMBER_OF_VIEWMODES - 1;
            }
            if(m_view_mode == TileMode::Voxel){
                Renderer::DrawScreenData(m_screen_data, m_window);
            }
            
        }
        if(event.key.scancode == sf::Keyboard::Scancode::Down){
            m_view_mode++;
            if(m_view_mode >= NUMBER_OF_VIEWMODES){
                m_view_mode = 0;
            }

            if(m_view_mode == TileMode::Voxel){
                Renderer::DrawScreenData(m_screen_data, m_window);
            }
            
        }
        if(event.key.scancode == sf::Keyboard::Scancode::F){
            Renderer::m_demo_fog = !Renderer::m_demo_fog;
        }

        else if(event.key.scancode == sf::Keyboard::Scancode::B){
            m_tile_tool = TileTool::BRUSH;
        }
        else if(event.key.scancode == sf::Keyboard::Scancode::R){
            m_tile_tool = TileTool::RECTANGLE;
        }

        else if(event.key.scancode == sf::Keyboard::Scancode::Tab){
            looking_at_voxel_materials = !looking_at_voxel_materials;
        }

        if(event.key.scancode == sf::Keyboard::Scancode::E){

            if(looking_at_voxel_materials){
                selected_voxel_material--;
                if(selected_voxel_material < 0){
                    selected_voxel_material = AssetManager::GetAllVoxelMaterialNames().size() - 1;
                }
            }
            else{
                selected_tile_material--;
                if(selected_tile_material < 0){
                    selected_tile_material = AssetManager::GetAllTileMaterialNames().size() - 1;
                }
            }
        }
        if(event.key.scancode == sf::Keyboard::Scancode::D){
            if(looking_at_voxel_materials){
                selected_voxel_material++;
                if(selected_voxel_material >= AssetManager::GetAllVoxelMaterialNames().size()){
                    selected_voxel_material = 0;
                }
            }
            else{
                selected_tile_material++;
                if(selected_tile_material >= AssetManager::GetAllTileMaterialNames().size()){
                    selected_tile_material = 0;
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