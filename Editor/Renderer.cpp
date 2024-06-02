#include "Renderer.h"
#include "../System/Utility/Calc.h"
#include <iostream>
#include "Util.h"
#include "AssetManager.h"
#include "../System/Utility/PerlinNoise.h"

NormalPreset Renderer::m_normal_dictionary[NUMBER_OF_FACES];
sf::Image Renderer::m_spare_image;

RenderStep Renderer::m_render_progress = RenderStep::Finished;
int Renderer::m_progress_increment_y;
int Renderer::m_progress_goal_y;
int Renderer::m_progress_increment_z;
int Renderer::m_progress_goal_z; 

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
    m_normal_dictionary[FaceDirection::FACE_FRONT].normal_colour = Util::ColourFromNormal(FaceDirection::FACE_FRONT);

    m_normal_dictionary[FaceDirection::FACE_TOP].normal = Util::NormalFromFace(top_face);
    m_normal_dictionary[FaceDirection::FACE_TOP].normal_colour = Util::ColourFromNormal(FaceDirection::FACE_TOP);

    m_normal_dictionary[FaceDirection::FACE_LEFT].normal = Util::NormalFromFace(left_face);
    m_normal_dictionary[FaceDirection::FACE_LEFT].normal_colour = Util::ColourFromNormal(FaceDirection::FACE_LEFT);

    m_normal_dictionary[FaceDirection::FACE_RIGHT].normal = Util::NormalFromFace(right_face);
    m_normal_dictionary[FaceDirection::FACE_RIGHT].normal_colour = Util::ColourFromNormal(FaceDirection::FACE_RIGHT);

    m_normal_dictionary[FaceDirection::FACE_BOTTOM].normal = Util::NormalFromFace(bottom_face);
    m_normal_dictionary[FaceDirection::FACE_BOTTOM].normal_colour = Util::ColourFromNormal(FaceDirection::FACE_BOTTOM);


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

                if(!tile.occupied){ // || (tile.voxel_object_index != -1 && !tile.is_topleft_of_object)){
                    continue;
                }
                if(tile.voxel_object_index != -1){
                    
                    if(tile.is_topleft_of_object){
                        CreateVoxelFromVoxelObject(screen_data, x, y, i);
                    }
                }
                else if(tile.tile_object_index != -1){
                    if(tile.is_topleft_of_object){
                        CreateVoxelFromTileObject(screen_data, x, y, i);
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

        int z = i * screen_data.m_tile_depth;

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

                // skip occupied spaces
                if(screen_data.m_voxel_space[z][x][y].occupied){
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

    const TileMaterial& tile_material = AssetManager::GetTileMaterial(screen_data.m_tile_layers[tile_layer][tile_x][tile_y].tile_material_index);

    sf::Vector2i texture_position_to_sample;

    // create the voxels which would make up this tile
    for(int x = 0; x < screen_data.m_tile_size; x++){
        for(int y = 0; y < screen_data.m_tile_size; y++){
            for(int depth = 0; depth < screen_data.m_tile_depth; depth++){

                int z = depth + (tile_layer * screen_data.m_tile_depth);

                int _x_canvas = x + tile_x * screen_data.m_tile_size;
                int _y_canvas = y + tile_y * screen_data.m_tile_size;
                
                texture_position_to_sample = Util::PositionToTexturePosition(sf::Vector2i(_x_canvas, _y_canvas), *tile_material.image_texture, true, true);

                screen_data.m_voxel_space[z][_x_canvas][_y_canvas].occupied = true;
                screen_data.m_voxel_space[z][_x_canvas][_y_canvas].normal_colour = tile_material.image_texture->getPixel(texture_position_to_sample.x, texture_position_to_sample.y);
                screen_data.m_voxel_space[z][_x_canvas][_y_canvas].draw_sides = true;
            }
        }
    }
}

void Renderer::CreateVoxelFromVoxelObject(ScreenData& screen_data, int tile_x, int tile_y, int tile_layer){


    const VoxelObject& voxel_material = AssetManager::GetVoxelObject(screen_data.m_tile_layers[tile_layer][tile_x][tile_y].voxel_object_index);

    sf::Color sampled_pixel;

    int z = tile_layer * screen_data.m_tile_depth;
    int canvas_x = tile_x * screen_data.m_tile_size;
    int canvas_y = tile_y * screen_data.m_tile_size; 

    for(int layer = 0; layer < voxel_material.layer_count; layer++){
        for(int x = 0; x < voxel_material.layer_width; x++){
            for(int y = 0; y < voxel_material.layer_height; y++){
                
                // move down with each layer
                int texture_y = y + voxel_material.layer_height * (voxel_material.layer_count - layer - 1);

                sampled_pixel = voxel_material.image_texture->getPixel(x, texture_y);

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

void Renderer::CreateVoxelFromTileObject(ScreenData& screen_data, int tile_x, int tile_y, int tile_layer){

    auto object_names = AssetManager::GetAllTileObjectNames();

    int voxel_object_index = screen_data.m_tile_layers[tile_layer][tile_x][tile_y].tile_object_index;
    const TileObject& tile_object = AssetManager::GetTileObject(voxel_object_index);

    sf::Color sampled_pixel;

    int z = tile_layer * screen_data.m_tile_depth;
    int canvas_x = tile_x * screen_data.m_tile_size;
    int canvas_y = tile_y * screen_data.m_tile_size; 

    for(int layer = 0; layer < screen_data.m_tile_depth; layer++){
        for(int x = 0; x < tile_object.layer_width; x++){
            for(int y = 0; y < tile_object.layer_height; y++){
                
                sampled_pixel = tile_object.image_texture->getPixel(x, y);

                // no voxel at position
                if(sampled_pixel == sf::Color::Transparent){
                    continue;
                }
                //otherwise add..

                screen_data.m_voxel_space[z + layer][canvas_x + x][canvas_y + y].normal_colour = sampled_pixel;
                screen_data.m_voxel_space[z + layer][canvas_x + x][canvas_y + y].draw_sides = true;
                screen_data.m_voxel_space[z + layer][canvas_x + x][canvas_y + y].occupied = true;
            }
        }

    }

}

void Renderer::DrawVoxelLayer(ScreenData& screen_data, int y, int z_position, sf::RenderTarget& surface, DrawMode draw_mode){

    //std::cout << "[DRAWING LAYER]: " << z_position << "\n";

    float perspective_constant = 0.001f;
    int depth_constant = z_position;
    int depth_further_constant = (z_position + 1);
    /*
        iterates over an image within the voxel space (image specified by z_post)
    */

    sf::VertexArray vertex_array;
    sf::Vertex vertex;

    // draw sides 
    for(int x = 0; x < screen_data.m_canvas_width; x++){

            if(!screen_data.m_voxel_space[z_position][x][y].occupied){
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
                    if(!screen_data.m_voxel_space[z_position][x][y].draw_sides){
                        vertex.color = screen_data.m_voxel_space[z_position][x][y].normal_colour;
                    }
                    else{
                        vertex.color = m_normal_dictionary[i + 1].normal_colour;
                    }

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


    // draw front 
    for(int x = 0; x < screen_data.m_canvas_width; x++){


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
    
    surface.draw(vertex_array);




}

void Renderer::ManipulateVoxelsThroughGeneration(ScreenData& screen_data){

    PerlinNoise perlin;

    for(int i = 0; i < screen_data.m_tile_layers.size(); i++){

        for(int z = i * screen_data.m_tile_depth; z < (i + 1) * screen_data.m_tile_depth; z++){
            
            for(int x = 0; x < screen_data.m_canvas_width; x++){
                for(int y = 0; y < screen_data.m_canvas_height; y++){
                    
                    sf::Color pixel_colour = screen_data.m_generation_canvas[0][i].getPixel(x, y);
            
                    if(pixel_colour.r <= 1){
                        continue;
                    }


                    float percent = pixel_colour.r / (float)255;

                    double noise_val = perlin.octave2D_01(x * 0.01, y * 0.01, 2, 7);

                    if(percent == 1.0 || noise_val * percent + percent * 0.2 > 1.0 - percent){
                        screen_data.m_voxel_space[z][x][y].normal_colour = m_normal_dictionary[FACE_BOTTOM].normal_colour;
                    }
                    else if(noise_val * percent > 0.6 - percent){

                        if(screen_data.m_voxel_space[z][x][y].normal_colour == m_normal_dictionary[FACE_TOP].normal_colour){
                            screen_data.m_voxel_space[z][x][y].normal_colour = m_normal_dictionary[FACE_FRONT].normal_colour;
                        }
                        else{
                            screen_data.m_voxel_space[z][x][y].normal_colour = m_normal_dictionary[FACE_BOTTOM].normal_colour;
                        }
                    }
                }
            }
        }
    }

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

    


    //std::cout << "[STARTING RENDER...]\n";

    screen_data.m_canvas.clear(sf::Color::Transparent);
    screen_data.m_canvas_normals.clear(sf::Color::Black);
    screen_data.m_canvas_depth.clear(sf::Color::White);

    ClearVoxels(screen_data);

    //std::cout << "[CREATING VOXELS...]\n";

    CreateVoxelsFromScreenData(screen_data);

    ManipulateVoxelsThroughGeneration(screen_data);

    // draw normal map
    for(int z = screen_data.m_voxel_space.size() - 1; z >= 0; z--){
        for(int y = 0; y < screen_data.m_canvas_height; y++){
            DrawVoxelLayer(screen_data, y, z, screen_data.m_canvas_normals, DrawMode::NORMALS);
        }
    }
    // draw depth map
    for(int z = screen_data.m_voxel_space.size() - 1; z >= 0; z--){
        for(int y = 0; y < screen_data.m_canvas_height; y++){
            DrawVoxelLayer(screen_data, y, z, screen_data.m_canvas_depth, DrawMode::DEPTH);
        }
    }

    screen_data.m_canvas_normals.display();
    screen_data.m_canvas_depth.display();
    
    m_spare_image = screen_data.m_canvas_normals.getTexture().copyToImage();
    //ComputeNormalCanvas(screen_data);
    for(int y = 0; y < screen_data.m_canvas_height; y++){
        ComputeNormalCanvas(screen_data, y);
    }
    sf::Texture texture_from_img;
    texture_from_img.loadFromImage(screen_data.m_canvas_image);
    screen_data.m_canvas.draw(sf::Sprite(texture_from_img));

    ComputeRaycastedLighting(screen_data);

    screen_data.m_canvas.display();

    AssetManager::SaveFinalRender(screen_data);

}


void Renderer::Update(ScreenData& screen_data, sf::RenderTarget& surface){

    // CURRENTLY NOT USED
    return;

    if(m_render_progress != RenderStep::Finished){

        switch(m_render_progress){

            case RenderStep::Inital: {
                
                screen_data.m_canvas.clear(sf::Color::Transparent);
                screen_data.m_canvas_normals.clear(sf::Color::Black);
                screen_data.m_canvas_depth.clear(sf::Color::White);

                ClearVoxels(screen_data);
                CreateVoxelsFromScreenData(screen_data);

                m_render_progress = RenderStep::DrawNormals;

                m_progress_goal_y = screen_data.m_voxel_space[0][0].size();
                m_progress_increment_y = 0;
                m_progress_goal_z = screen_data.m_voxel_space.size();
                m_progress_increment_z = 0;
                break;
            }
            case RenderStep::DrawNormals: {

                // draw normal map
                DrawVoxelLayer(screen_data, m_progress_increment_y, screen_data.m_voxel_space.size() - 1 - m_progress_increment_z, screen_data.m_canvas_normals, DrawMode::NORMALS);
                
                // draw texture
                //screen_data.m_canvas_normals.display();
                //screen_data.m_canvas.draw(sf::Sprite(screen_data.m_canvas_normals.getTexture()));

                m_progress_increment_y++;
                m_progress_increment_z++;

                if(m_progress_increment_y >= m_progress_goal_y){
                    m_progress_increment_y = 0;
                    m_progress_increment_z++;
                }
                if(m_progress_increment_z >= m_progress_goal_z){
                    m_progress_increment_z = 0;
                    m_render_progress = RenderStep::DrawDepth;  
                }

                break;
            }
            case RenderStep::DrawDepth: {

                // draw depth map
                DrawVoxelLayer(screen_data, m_progress_increment_y, screen_data.m_voxel_space.size() - 1 - m_progress_increment_z, screen_data.m_canvas_depth, DrawMode::DEPTH);

                // draw texture
                //screen_data.m_canvas_depth.display();
                //screen_data.m_canvas.draw(sf::Sprite(screen_data.m_canvas_depth.getTexture()));

                m_progress_increment_y++;

                if(m_progress_increment_y >= m_progress_goal_y){
                    m_progress_increment_y = 0;
                    m_progress_increment_z++;
                }
                if(m_progress_increment_z >= m_progress_goal_z){
                    m_progress_increment_y = 0;

                    // load normal map as image
                    m_spare_image = screen_data.m_canvas_normals.getTexture().copyToImage();
                    m_render_progress = RenderStep::ComputeLight;  
                }

                break;
            }
            case RenderStep::ComputeLight: {

                ComputeNormalCanvas(screen_data, m_progress_increment_y);

                m_progress_increment_y++;

                sf::Texture texture_from_img;
                texture_from_img.loadFromImage(screen_data.m_canvas_image);

                screen_data.m_canvas.draw(sf::Sprite(texture_from_img));

                if(m_progress_increment_y >= m_progress_goal_y){
                    m_render_progress = RenderStep::Finished;  
                }
                
                break;
            }

        }


    }
    else{
        
        if(m_demo_fog){
            //screen_data.m_canvas.draw(sf::Sprite(screen_data.m_canvas_depth.getTexture()), sf::BlendAdd);
        }
        screen_data.m_canvas.display();
    
    }


}

/*
    converts the screen_data normal texture to flat light values (drawn to m_canvas)
    is called after the normal texture is created
*/
void Renderer::ComputeNormalCanvas(ScreenData& screen_data, int y){

    sf::Color sampled_col;
    for(unsigned int x = 0; x < screen_data.m_canvas_width; x++){
        //std::cout << "[FLAT LIGHT]: X: " << x << "/" << screen_data.m_canvas_width << "\n";
            
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

/*
    is called after ComputeNormalCanvas() and after the depth texture is created 
*/
void Renderer::ComputeRaycastedLighting(ScreenData& screen_data){

    // pixel grid of the depth
    m_spare_image = screen_data.m_canvas_depth.getTexture().copyToImage();
    
    sf::Color sampled_col;
    sf::Int32 sampled_depth;

    if(screen_data.m_flat_light_direction.z <= 0){
        //std::cout << "[ERROR]: Light direction z input cannot be less or equal to zero, Renderer::ComputeRaycastedLighting()\n";
        return;
    }

    sf::Vector3f ray_step_direction = -screen_data.m_flat_light_direction * 0.1f; // * 0.0f;
    ray_step_direction.z * 0.25f; // scale z direction so shadows become longer

    sf::Color shadow_colour;

    for(unsigned int x = 0; x < screen_data.m_canvas_width; x++){
        break;
        //std::cout << "[CASTING SHADOWS]: X: " << x << "/" << screen_data.m_canvas_width << "\n";
        
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


    sf::Texture texture_from_img;
    texture_from_img.loadFromImage(screen_data.m_canvas_image);

    screen_data.m_canvas.draw(sf::Sprite(texture_from_img), sf::BlendMultiply);
    
    screen_data.m_canvas.display();
}


