#include "Renderer.h"
#include "../System/Utility/Calc.h"
#include <iostream>
#include "Util.h"
#include "AssetManager.h"

NormalPreset Renderer::m_normal_dictionary[NUMBER_OF_FACES];
sf::Image Renderer::m_spare_image;

bool Renderer::m_demo_fog = false;

void Renderer::Init(){

    // front face
    Face front_face({
        {
            sf::Vector3f(0, 0, 0),
            sf::Vector3f(1, 0, 0),
            sf::Vector3f(1, 1, 0),
            sf::Vector3f(0, 1, 0),
        },
    });

    // top face
    Face top_face = Face({
        {
            sf::Vector3f(0, 0, 0),
            sf::Vector3f(1, 0, 0),
            sf::Vector3f(1, 0, 1),
            sf::Vector3f(0, 0, 1),
        },
    });

    // left face
    Face left_face = Face({
        {
            sf::Vector3f(0, 1, 0),
            sf::Vector3f(0, 0, 0),
            sf::Vector3f(0, 0, 1),
            sf::Vector3f(0, 1, 1),
        },
    });

    // right face
    Face right_face = Face({
        {
            sf::Vector3f(1, 0, 0),
            sf::Vector3f(1, 1, 0),
            sf::Vector3f(1, 1, 1),
            sf::Vector3f(1, 0, 1),
        },
    });

    // bottom face
    Face bottom_face = Face({
        {
            sf::Vector3f(1, 1, 0),
            sf::Vector3f(0, 1, 0),
            sf::Vector3f(0, 1, 1), 
            sf::Vector3f(1, 1, 1),
        },
    });

    m_normal_dictionary[FaceDirection::FACE_FRONT].normal = Util::NormalFromFace(front_face);
    m_normal_dictionary[FaceDirection::FACE_FRONT].normal_colour = Util::ColourFromNormal(m_normal_dictionary[FaceDirection::FACE_FRONT].normal);

    m_normal_dictionary[FaceDirection::FACE_TOP].normal = Util::NormalFromFace(top_face);
    m_normal_dictionary[FaceDirection::FACE_TOP].normal_colour = Util::ColourFromNormal(m_normal_dictionary[FaceDirection::FACE_TOP].normal);

    m_normal_dictionary[FaceDirection::FACE_LEFT].normal = Util::NormalFromFace(left_face);
    m_normal_dictionary[FaceDirection::FACE_LEFT].normal_colour = Util::ColourFromNormal(m_normal_dictionary[FaceDirection::FACE_LEFT].normal);

    m_normal_dictionary[FaceDirection::FACE_RIGHT].normal = Util::NormalFromFace(right_face);
    m_normal_dictionary[FaceDirection::FACE_RIGHT].normal_colour = Util::ColourFromNormal(m_normal_dictionary[FaceDirection::FACE_RIGHT].normal);

    m_normal_dictionary[FaceDirection::FACE_BOTTOM].normal = Util::NormalFromFace(bottom_face);
    m_normal_dictionary[FaceDirection::FACE_BOTTOM].normal_colour = Util::ColourFromNormal(m_normal_dictionary[FaceDirection::FACE_BOTTOM].normal);


    ScreenData screen_data;
}




/*
    drawing our scene
*/
void Renderer::DrawScreenData(ScreenData& screen_data, sf::RenderTarget& surface){

    screen_data.m_canvas.clear(sf::Color::Transparent);

    // draw normal canvas
    screen_data.m_canvas_normals.clear(sf::Color::White);
    for(int i = screen_data.m_tile_layers.size() - 1; i >= 0; i--){
        DrawTileLayer(i, screen_data, screen_data.m_canvas_normals, DrawMode::NORMALS);
    }
    screen_data.m_canvas_normals.display();

    // draw depth canvas
    screen_data.m_canvas_depth.clear(sf::Color::White);
    for(int i = screen_data.m_tile_layers.size() - 1; i >= 0; i--){
        DrawTileLayer(i, screen_data, screen_data.m_canvas_depth, DrawMode::DEPTH);
    }
    screen_data.m_canvas_depth.display();


    ComputeNormalCanvas(screen_data);
    screen_data.m_canvas.display();

    ComputeRaycastedLighting(screen_data);

    if(m_demo_fog){
        screen_data.m_canvas.draw(sf::Sprite(screen_data.m_canvas_depth.getTexture()), sf::BlendAdd);
    }
    screen_data.m_canvas.display();

}

/*
    draws the geometry for a tile layer,
    is used to draw normal texture and depth texture
*/
void Renderer::DrawTileLayer(int tile_layer_index, ScreenData& screen_data, sf::RenderTarget& surface, DrawMode draw_mode){

    sf::Vector2f half_screen_size(screen_data.m_canvas_width / 2.0f, screen_data.m_canvas_height / 2.0f);
    int step_count = 1;

    sf::Vertex vertex;
    vertex.color = sf::Color::Red;

    sf::VertexArray vertex_array;
    vertex_array.setPrimitiveType(sf::PrimitiveType::Quads);

    std::vector<Rect> total_rects = {};

    std::vector<std::vector<TileData>> tiles = screen_data.m_tile_layers[tile_layer_index];
    
    float perspective_constant = screen_data.m_tile_size * 0.001;
    
    for(int x = 0; x < tiles.size(); x++){
        for(int y = 0; y < tiles[x].size(); y++){
            
            if(!tiles[x][y].occupied){
                continue;
            }


            // convert each tile to a rect
            Rect rect({(float)x * screen_data.m_tile_size, (float)y * screen_data.m_tile_size, (float)screen_data.m_tile_size, (float)screen_data.m_tile_size});

            // shift further layers back
            for(int i = 0; i < tile_layer_index; i++){

                sf::Vector2f new_rect_pos = Util::ShiftVertexOnPerspectiveAxis(sf::Vector2f(rect.m_position_x, rect.m_position_y), screen_data, perspective_constant);
                rect.m_position_x = new_rect_pos.x;
                rect.m_position_y = new_rect_pos.y;
            }

            total_rects.push_back(rect);


            std::vector<sf::Vector2f> positions_pairs = 
            { 
                sf::Vector2f(rect.m_position_x, rect.m_position_y), 
                sf::Vector2f(rect.m_position_x + rect.m_width, rect.m_position_y),
                sf::Vector2f(rect.m_position_x + rect.m_width, rect.m_position_y + rect.m_height),
                sf::Vector2f(rect.m_position_x, rect.m_position_y + rect.m_height),
                sf::Vector2f(rect.m_position_x, rect.m_position_y), // to complete loop
            };
            
            std::vector<FaceDirection> face_directions = {
                FACE_TOP,
                FACE_RIGHT,
                FACE_BOTTOM,
                FACE_LEFT,
            };

            for(int step = 0; step < step_count; step++){

                for(int i = 0; i < positions_pairs.size() - 1; i++){

                    // is there a tile nearby that should block the face?

                    bool draw = !Util::TileSurrounding(x, y, face_directions[i], screen_data, tile_layer_index);

                    if(draw_mode == DrawMode::NORMALS){
                        vertex.color = m_normal_dictionary[i + 1].normal_colour;
                    }

                    // close vertex
                    vertex.texCoords.x = 0;
                    vertex.texCoords.y = 0;
                    vertex.position = positions_pairs[i];
                    if(draw_mode == DrawMode::DEPTH){
                        vertex.color = Util::ColourFromDepth(screen_data.m_tile_depth * tile_layer_index);
                    }
                    if(draw){
                        vertex_array.append(vertex);
                    }

                    // far vertex
                    vertex.texCoords.x = 16;
                    vertex.texCoords.y = 0;
                    vertex.position = Util::ShiftVertexOnPerspectiveAxis(positions_pairs[i], screen_data, perspective_constant);
                    positions_pairs[i] = vertex.position;
                    if(draw_mode == DrawMode::DEPTH){
                        vertex.color = Util::ColourFromDepth(screen_data.m_tile_depth * (tile_layer_index + 1));
                    }
                    if(draw){
                        vertex_array.append(vertex);
                    }

                    // far vertex
                    vertex.texCoords.x = 16;
                    vertex.texCoords.y = 16;
                    vertex.position = Util::ShiftVertexOnPerspectiveAxis(positions_pairs[i + 1], screen_data, perspective_constant);
                    if(draw_mode == DrawMode::DEPTH){
                        vertex.color = Util::ColourFromDepth(screen_data.m_tile_depth * (tile_layer_index + 1));
                    }
                    if(draw){
                        vertex_array.append(vertex);
                    }

                    // close vertex
                    vertex.texCoords.x = 0;
                    vertex.texCoords.y = 16;
                    vertex.position = positions_pairs[i + 1]; 
                    if(draw_mode == DrawMode::DEPTH){
                        vertex.color = Util::ColourFromDepth(screen_data.m_tile_depth * tile_layer_index);
                    }
                    if(draw){
                        vertex_array.append(vertex);
                    }
                }

                // setting the end position to the translated end
                positions_pairs[4] = positions_pairs[0];
            }
        }
    }


    // drawing sides
    surface.draw(vertex_array);   

    vertex.color = m_normal_dictionary[FACE_FRONT].normal_colour;
    vertex_array.clear();
    // dont colour front face
    vertex.color = sf::Color::White;

    sf::RenderStates render_states;
    
    if(draw_mode == DrawMode::DEPTH){
        vertex.color = Util::ColourFromDepth(screen_data.m_tile_depth * tile_layer_index);
    }




    
    std::vector<TileMaterial*> tile_materials = AssetManager::GetAllTileMaterials();

    /*
        draw front faces
    */
    for(int x = 0; x < tiles.size(); x++){
        for(int y = 0; y < tiles[x].size(); y++){

            if(!tiles[x][y].occupied){
                continue;
            }


            int position_x = x * screen_data.m_tile_size;
            int position_y = y * screen_data.m_tile_size;

            float pos_float_x = position_x;
            float pos_float_y = position_y;

            // shift further layers back
            for(int i = 0; i < tile_layer_index; i++){

                sf::Vector2f new_rect_pos = Util::ShiftVertexOnPerspectiveAxis(sf::Vector2f(pos_float_x, pos_float_y), screen_data, perspective_constant);
                pos_float_x = new_rect_pos.x;
                pos_float_y = new_rect_pos.y;
            }

            //
            if(draw_mode == DrawMode::DEPTH){
                render_states.texture = nullptr;            
            }
            else{ //otherwise normal

                render_states.texture = &tile_materials[tiles[x][y].tile_material_index]->texture;
            }

            vertex.texCoords = Util::PositionToTexturePosition(sf::Vector2i(position_x, position_y), tile_materials.at(tiles[x][y].tile_material_index)->texture, false, false);
            vertex.position.x = pos_float_x;
            vertex.position.y = pos_float_y;    
            vertex_array.append(vertex);

            vertex.texCoords = Util::PositionToTexturePosition(sf::Vector2i(position_x + screen_data.m_tile_size, position_y), tile_materials.at(tiles[x][y].tile_material_index)->texture, true, false);
            vertex.position.x = pos_float_x + screen_data.m_tile_size;    
            vertex.position.y = pos_float_y;    
            vertex_array.append(vertex);

            vertex.texCoords = Util::PositionToTexturePosition(sf::Vector2i(position_x + screen_data.m_tile_size, position_y + screen_data.m_tile_size), tile_materials.at(tiles[x][y].tile_material_index)->texture, true, true);
            vertex.position.x = pos_float_x + screen_data.m_tile_size;    
            vertex.position.y = pos_float_y + screen_data.m_tile_size;    
            vertex_array.append(vertex);

            vertex.texCoords = Util::PositionToTexturePosition(sf::Vector2i(position_x, position_y + screen_data.m_tile_size), tile_materials.at(tiles[x][y].tile_material_index)->texture, false, true);
            vertex.position.x = pos_float_x;    
            vertex.position.y = pos_float_y + screen_data.m_tile_size;    
            vertex_array.append(vertex);

            surface.draw(vertex_array, render_states);
            
            // draw each face indivudally to allow faces to have different textures
            vertex_array.clear();
        }
    }


    /*
        outline tiles with relative normal, provides more contrast to layers
    */

   if(draw_mode == NORMALS){


    int edge_width = 2;

    sf::Vector2f top_edge[4] = { 
        sf::Vector2f(0, 0), 
        sf::Vector2f(0, edge_width), 
        sf::Vector2f(screen_data.m_tile_size, edge_width), 
        sf::Vector2f(screen_data.m_tile_size, 0)};

    sf::Vector2f bottom_edge[4] = 
    { 
        sf::Vector2f(0, screen_data.m_tile_size), 
        sf::Vector2f(0, screen_data.m_tile_size - edge_width), 
        sf::Vector2f(screen_data.m_tile_size, screen_data.m_tile_size - edge_width), 
        sf::Vector2f(screen_data.m_tile_size, screen_data.m_tile_size)};

    sf::Vector2f left_edge[4] = { 
        sf::Vector2f(0, 0), 
        sf::Vector2f(edge_width, 0), 
        sf::Vector2f(edge_width, screen_data.m_tile_size), 
        sf::Vector2f(0, screen_data.m_tile_size)};

    sf::Vector2f right_edge[4] = { 
        sf::Vector2f(screen_data.m_tile_size, 0), 
        sf::Vector2f(screen_data.m_tile_size - edge_width, 0), 
        sf::Vector2f(screen_data.m_tile_size - edge_width, screen_data.m_tile_size), 
        sf::Vector2f(screen_data.m_tile_size, screen_data.m_tile_size)};

    for(int x = 0; x < tiles.size(); x++){
        for(int y = 0; y < tiles[x].size(); y++){


            std::vector<sf::Vector2f*> quads_to_draw = {};
            std::vector<sf::Vector3f> face_normals = {};

            if(!tiles[x][y].occupied || !tile_materials[tiles[x][y].tile_material_index]->round_edges){
                continue;
            }// only proceeds if round_edges == true

            Rect rect({(float)x * screen_data.m_tile_size, (float)y * screen_data.m_tile_size, (float)screen_data.m_tile_size, (float)screen_data.m_tile_size});

            // shift further layers back
            for(int i = 0; i < tile_layer_index; i++){

                sf::Vector2f new_rect_pos = Util::ShiftVertexOnPerspectiveAxis(sf::Vector2f(rect.m_position_x, rect.m_position_y), screen_data, perspective_constant);
                rect.m_position_x = new_rect_pos.x;
                rect.m_position_y = new_rect_pos.y;
            }

            // determine neighbours

            // bottom
            if(y < tiles[0].size() - 1 && !tiles[x][y + 1].occupied){
                quads_to_draw.push_back(bottom_edge);
                face_normals.push_back(m_normal_dictionary[FaceDirection::FACE_TOP].normal);
            }
            // right
            if(x < tiles.size() - 1 && !tiles[x + 1][y].occupied){
                quads_to_draw.push_back(right_edge);
                face_normals.push_back(m_normal_dictionary[FaceDirection::FACE_LEFT].normal);
            }
            // left
            if(x > 0 && !tiles[x - 1][y].occupied){
                quads_to_draw.push_back(left_edge);
                face_normals.push_back(m_normal_dictionary[FaceDirection::FACE_RIGHT].normal);
            }
            // top
            if(y > 0 && !tiles[x][y - 1].occupied){
                quads_to_draw.push_back(top_edge);
                face_normals.push_back(m_normal_dictionary[FaceDirection::FACE_BOTTOM].normal);
            }


            for(int i = 0; i < face_normals.size(); i++){

                vertex.color = Util::ColourFromNormal(Calc::Lerp(face_normals[i], m_normal_dictionary[FaceDirection::FACE_FRONT].normal, 0.5f));

                vertex.position.x = quads_to_draw[i][0].x + rect.m_position_x;
                vertex.position.y = quads_to_draw[i][0].y + rect.m_position_y;
                vertex_array.append(vertex);

                vertex.position.x = quads_to_draw[i][1].x + rect.m_position_x;
                vertex.position.y = quads_to_draw[i][1].y + rect.m_position_y;
                vertex_array.append(vertex);

                vertex.position.x = quads_to_draw[i][2].x + rect.m_position_x;
                vertex.position.y = quads_to_draw[i][2].y + rect.m_position_y;
                vertex_array.append(vertex);

                vertex.position.x = quads_to_draw[i][3].x + rect.m_position_x;
                vertex.position.y = quads_to_draw[i][3].y + rect.m_position_y;
                vertex_array.append(vertex);
            }



        }
    }
    surface.draw(vertex_array, sf::BlendMax);
   }
            
}

/*
    converts the screen_data normal texture to flat light values (drawn to m_canvas)
    is called after the normal texture is created
*/
void Renderer::ComputeNormalCanvas(ScreenData& screen_data){

    // pixel grid of the normals
    m_spare_image = screen_data.m_canvas_normals.getTexture().copyToImage();
    
    sf::Color sampled_col;
    for(unsigned int x = 0; x < screen_data.m_canvas_width; x++){
        std::cout << "[FLAT LIGHT]: X: " << x << "/" << screen_data.m_canvas_width << "\n";
        for(unsigned int y = 0; y < screen_data.m_canvas_height; y++){
            
            sampled_col = m_spare_image.getPixel(x, y);

            // calculate light from normal
            if(sampled_col != sf::Color::Black){

                sf::Uint32 light_val = Util::ConvertNormalColourToLightValue(screen_data, sampled_col);// + screen_data.m_shadow_lift;
                if(light_val > 255){
                    light_val = 255;
                }

                screen_data.m_canvas_image.setPixel(x, y, sf::Color(light_val, 0, 0));            
            }
            else{
                screen_data.m_canvas_image.setPixel(x, y, sf::Color::Black);            
            }

        }
    }

    sf::Texture image_texture;
    image_texture.loadFromImage(screen_data.m_canvas_image);

    screen_data.m_canvas.draw(sf::Sprite(image_texture));
    
}

/*
    is called after ComputeNormalCanvas() and after the depth texture is created 
*/
void Renderer::ComputeRaycastedLighting(ScreenData& screen_data){


    // pixel grid of the depth
    m_spare_image = screen_data.m_canvas_depth.getTexture().copyToImage();
    
    sf::Color sampled_col;
    sf::Int32 sampled_depth;

    if(screen_data.m_flat_light_direction.z <= 0){
        std::cout << "[ERROR]: Light direction z input cannot be less or equal to zero, Renderer::ComputeRaycastedLighting()\n";
        return;
    }

    sf::Vector3f ray_step_direction = -screen_data.m_flat_light_direction; // * 0.0f;

    sf::Color shadow_colour;

    for(unsigned int x = 0; x < screen_data.m_canvas_width; x++){
        std::cout << "[CASTING SHADOWS]: X: " << x << "/" << screen_data.m_canvas_width << "\n";
        for(unsigned int y = 0; y < screen_data.m_canvas_height; y++){
            
            sampled_col = m_spare_image.getPixel(x, y);

            // ignore any non tile pixels
            if(sampled_col != sf::Color::White){


                sampled_depth = Util::DepthFromColour(sampled_col);
                

                sf::Vector3f position(x, y, sampled_depth);

                shadow_colour = sf::Color::White;
                
                while(position.z > 0){
                    // invert the direction of light (to move towards the light source)
                    position += ray_step_direction;


                    if(Util::PositionIsInsideVoxel(screen_data, position)){
                        shadow_colour = sf::Color(120,120,120); 
                        break;
                    }
                }


                screen_data.m_canvas_image.setPixel(x, y, shadow_colour);            
            }
            else{
                screen_data.m_canvas_image.setPixel(x, y, sf::Color::White);    
            }

        }

    }


    sf::Texture image_texture;
    image_texture.loadFromImage(screen_data.m_canvas_image);

    screen_data.m_canvas.draw(sf::Sprite(image_texture), sf::BlendMultiply);
    
    screen_data.m_canvas.display();
}


