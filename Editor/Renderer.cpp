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




void Renderer::CreateVoxelsFromScreenData(ScreenData& screen_data){

    /*
        iterates over each tile layer, converting the large tiles into voxels 
    */

    CreateVoxelsFromRopes(screen_data);

    for(int i = 0; i < screen_data.m_tile_layers.size(); i++){

        for(int x = 0; x < screen_data.m_tile_layers[i].size(); x++){
            for(int y = 0; y < screen_data.m_tile_layers[i][x].size(); y++){

                TileData& tile = screen_data.m_tile_layers[i][x][y];

                if(!tile.occupied){ // || (tile.voxel_material_index != -1 && !tile.is_topleft_of_voxel_material)){
                    continue;
                }
                if(tile.voxel_material_index != -1){
                    
                    if(tile.is_topleft_of_voxel_material){
                        CreateVoxelFromVoxelMaterial(screen_data, x, y, i);
                    }
                }
                else{
                    // draw the tile as a voxel
                    CreateVoxelFromTile(screen_data, x, y, i);
                }

            

            }
        }
    }



}

void Renderer::CreateVoxelsFromRopes(ScreenData& screen_data){

    for(int i = 0; i < screen_data.m_tile_layers.size(); i++){

        int z = i * screen_data.m_tile_size;

        for(auto& rope : screen_data.m_ropes[i]){

            std::vector<sf::Vector2f> curve_points = rope.SamplePositions(0.0012);
            
            for(auto& point : curve_points){

                int x = floor(point.x);
                int y = floor(point.y);

                if(x < 0 || x >= screen_data.m_canvas_width){
                    continue;
                }
                if(y < 0 || y >= screen_data.m_canvas_height){
                    continue;
                }

                screen_data.m_voxel_space[z][x][y].draw_sides = false;
                screen_data.m_voxel_space[z][x][y].occupied = true;
                screen_data.m_voxel_space[z][x][y].normal_colour = m_normal_dictionary[FACE_FRONT].normal_colour;   

            }

        }
    }


}

void Renderer::CreateVoxelFromTile(ScreenData& screen_data, int tile_x, int tile_y, int tile_layer){

    auto material_names = AssetManager::GetAllTileMaterialNames();

    int tile_material_index = screen_data.m_tile_layers[tile_layer][tile_x][tile_y].tile_material_index;
    const TileMaterial& tile_material = AssetManager::GetTileMaterial(material_names[tile_material_index]);


    sf::Vector2i texture_position_to_sample;

    // create the voxels which would make up this tile
    for(int x = 0; x < screen_data.m_tile_size; x++){
        for(int y = 0; y < screen_data.m_tile_size; y++){
            for(int depth = 0; depth < screen_data.m_tile_depth; depth++){

                int z = depth + (tile_layer * screen_data.m_tile_depth);

                int _x_canvas = x + tile_x * screen_data.m_tile_size;
                int _y_canvas = y + tile_y * screen_data.m_tile_size;
                
                texture_position_to_sample = Util::PositionToTexturePosition(sf::Vector2i(_x_canvas, _y_canvas), tile_material.image_texture, true, true);

                screen_data.m_voxel_space[z][_x_canvas][_y_canvas].occupied = true;
                screen_data.m_voxel_space[z][_x_canvas][_y_canvas].normal_colour = tile_material.image_texture.getPixel(texture_position_to_sample.x, texture_position_to_sample.y);
                screen_data.m_voxel_space[z][_x_canvas][_y_canvas].draw_sides = true;
            }
        }
    }
}

void Renderer::CreateVoxelFromVoxelMaterial(ScreenData& screen_data, int tile_x, int tile_y, int tile_layer){

    auto material_names = AssetManager::GetAllVoxelMaterialNames();

    int voxel_material_index = screen_data.m_tile_layers[tile_layer][tile_x][tile_y].voxel_material_index;
    const VoxelMaterial& voxel_material = AssetManager::GetVoxelMaterial(material_names[voxel_material_index]);

    sf::Color sampled_pixel;

    int z = tile_layer * screen_data.m_tile_depth;
    int canvas_x = tile_x * screen_data.m_tile_size;
    int canvas_y = tile_y * screen_data.m_tile_size; 

    for(int layer = 0; layer < voxel_material.layer_count; layer++){
        for(int x = 0; x < voxel_material.layer_width; x++){
            for(int y = 0; y < voxel_material.layer_height; y++){
                
                // move down with each layer
                int texture_y = y + voxel_material.layer_height * (voxel_material.layer_count - layer - 1);

                sampled_pixel = voxel_material.image_texture.getPixel(x, texture_y);

                // no voxel at position
                if(sampled_pixel == sf::Color::Transparent){
                    continue;
                }
                //otherwise add..

                screen_data.m_voxel_space[z + layer][canvas_x + x][canvas_y + y].normal_colour = sampled_pixel;
                screen_data.m_voxel_space[z + layer][canvas_x + x][canvas_y + y].draw_sides = voxel_material.draw_sides;
                screen_data.m_voxel_space[z + layer][canvas_x + x][canvas_y + y].occupied = true;
            }
        }

    }

}



void Renderer::DrawVoxelLayer(ScreenData& screen_data, int z_position, sf::RenderTarget& surface, DrawMode draw_mode){

    std::cout << "[DRAWING LAYER]: " << z_position << "\n";

    float perspective_constant = 0.003f;
    int depth_constant = z_position;
    int depth_further_constant = (z_position + 1);
    /*
        iterates over an image within the voxel space (image specified by z_post)
    */

    sf::VertexArray vertex_array;
    sf::Vertex vertex;

    // draw sides 
    for(int x = 0; x < screen_data.m_canvas_width; x++){
        for(int y = 0; y < screen_data.m_canvas_height; y++){

            if(!screen_data.m_voxel_space[z_position][x][y].occupied || !screen_data.m_voxel_space[z_position][x][y].draw_sides){
                continue;
            }

            // convert each voxel to a rect
            Rect rect({x, y, 1, 1});

            // shift back by z position
            sf::Vector2f new_rect_pos = Util::ShiftVertexOnPerspectiveAxis(sf::Vector2f(rect.m_position_x, rect.m_position_y), screen_data, perspective_constant * z_position, true);
            rect.m_position_x = new_rect_pos.x;
            rect.m_position_y = new_rect_pos.y;



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


            for(int i = 0; i < positions_pairs.size() - 1; i++){

                // is there a tile nearby that should block the face?

                bool draw = !Util::VoxelSurrounding(x, y, z_position, face_directions[i], screen_data);

                if(draw_mode == DrawMode::NORMALS){
                    vertex.color = m_normal_dictionary[i + 1].normal_colour;
                }

                // close vertex
                vertex.position = positions_pairs[i];
                if(draw_mode == DrawMode::DEPTH){
                    vertex.color = Util::ColourFromDepth(depth_constant);
                }
                if(draw){
                    vertex_array.append(vertex);
                }

                // far vertex
                vertex.position = Util::ShiftVertexOnPerspectiveAxis(positions_pairs[i], screen_data, perspective_constant, true);
                positions_pairs[i] = vertex.position;
                if(draw_mode == DrawMode::DEPTH){
                    vertex.color = Util::ColourFromDepth(depth_further_constant);
                }
                if(draw){
                    vertex_array.append(vertex);
                }

                // far vertex
                vertex.position = Util::ShiftVertexOnPerspectiveAxis(positions_pairs[i + 1], screen_data, perspective_constant, true);
                if(draw_mode == DrawMode::DEPTH){
                    vertex.color = Util::ColourFromDepth(depth_further_constant);
                }
                if(draw){
                    vertex_array.append(vertex);
                }

                // close vertex
                vertex.position = positions_pairs[i + 1]; 
                if(draw_mode == DrawMode::DEPTH){
                    vertex.color = Util::ColourFromDepth(depth_constant);
                }
                if(draw){
                    vertex_array.append(vertex);
                }
            }

            // setting the end position to the translated end
            positions_pairs[4] = positions_pairs[0];
        }
    }


    // draw front 
    for(int x = 0; x < screen_data.m_canvas_width; x++){
        for(int y = 0; y < screen_data.m_canvas_height; y++){


            // black represents empty
            if(!screen_data.m_voxel_space[z_position][x][y].occupied){
                continue;
            }
            // block in front
            if(z_position > 0 && screen_data.m_voxel_space[z_position - 1][x][y].occupied){
                continue;
            }

            vertex.color = screen_data.m_voxel_space[z_position][x][y].normal_colour;

            sf::Vector2f pos = Util::ShiftVertexOnPerspectiveAxis(sf::Vector2f(x, y), screen_data, perspective_constant * z_position, true);

            if(draw_mode == DrawMode::DEPTH){
                vertex.color = Util::ColourFromDepth(depth_constant);
            }


            vertex.position.x = pos.x;
            vertex.position.y = pos.y;    
            vertex_array.append(vertex);

            vertex.position.x = pos.x + 1;    
            vertex.position.y = pos.y;    
            vertex_array.append(vertex);

            vertex.position.x = pos.x + 1;    
            vertex.position.y = pos.y + 1;     
            vertex_array.append(vertex);

            vertex.position.x = pos.x;    
            vertex.position.y = pos.y + 1;    
            vertex_array.append(vertex);

        }
    }
    
    surface.draw(vertex_array);




}

void Renderer::ClearVoxels(ScreenData& screen_data){

    // clear voxel space
    for(int z = 0; z < screen_data.m_voxel_space.size(); z++){
        for(int x = 0; x < screen_data.m_canvas_width; x++){
            for(int y = 0; y < screen_data.m_canvas_height; y++){
                screen_data.m_voxel_space[z][x][y].occupied = false; 
                screen_data.m_voxel_space[z][x][y].normal_colour = sf::Color::Black; 
            }
        }
    }
}

/*
    drawing our scene
*/
void Renderer::DrawScreenData(ScreenData& screen_data, sf::RenderTarget& surface){

    screen_data.m_canvas.clear(sf::Color::Transparent);
    screen_data.m_canvas_normals.clear(sf::Color::Black);
    screen_data.m_canvas_depth.clear(sf::Color::White);

    ClearVoxels(screen_data);
    CreateVoxelsFromScreenData(screen_data);

    // draw normal map
    for(int z = screen_data.m_voxel_space.size() - 1; z >= 0; z--){
        DrawVoxelLayer(screen_data, z, screen_data.m_canvas_normals, DrawMode::NORMALS);
    }
    // draw depth map
    for(int z = screen_data.m_voxel_space.size() - 1; z >= 0; z--){
        DrawVoxelLayer(screen_data, z, screen_data.m_canvas_depth, DrawMode::DEPTH);
    }

    screen_data.m_canvas_normals.display();
    screen_data.m_canvas_depth.display();

    ComputeNormalCanvas(screen_data);

    ComputeRaycastedLighting(screen_data);

    screen_data.m_canvas.display();
    if(m_demo_fog){
        screen_data.m_canvas.draw(sf::Sprite(screen_data.m_canvas_depth.getTexture()), sf::BlendAdd);
    }
}

/*
    draws the geometry for a tile layer,
    is used to draw normal texture and depth texture
*/
void Renderer::DrawTileLayer(int tile_layer_index, ScreenData& screen_data, sf::RenderTarget& surface, DrawMode draw_mode){

    sf::Vector2f half_screen_size(screen_data.m_canvas_width / 2.0f, screen_data.m_canvas_height / 2.0f);

    sf::Vertex vertex;
    vertex.color = sf::Color::Red;

    sf::VertexArray vertex_array;
    vertex_array.setPrimitiveType(sf::PrimitiveType::Quads);

    std::vector<Rect> total_rects = {};

    std::vector<std::vector<TileData>> tiles = screen_data.m_tile_layers[tile_layer_index];
    
    float perspective_constant = screen_data.m_tile_size * 0.003;
    
    for(int x = 0; x < tiles.size(); x++){
        for(int y = 0; y < tiles[x].size(); y++){
            
            if(!tiles[x][y].occupied || (tiles[x][y].voxel_material_index != -1 && !tiles[x][y].is_topleft_of_voxel_material)){
                continue;
            }

            total_rects.clear();

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
    */      
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

    sf::Vector3f ray_step_direction = -screen_data.m_flat_light_direction * 0.1f; // * 0.0f;
    ray_step_direction.z * 0.25f; // scale z direction so shadows become longer

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


