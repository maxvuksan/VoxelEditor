#include "AssetManager.h"
#include <iostream>
#include "Util.h"
#include <filesystem>
#include "ScreenData.h"

std::vector<TileObject> AssetManager::m_tile_objects;
std::vector<VoxelObject> AssetManager::m_voxel_objects;
std::vector<TileMaterial> AssetManager::m_tile_materials;

std::vector<std::string> AssetManager::m_tile_objects_names;
std::vector<std::string> AssetManager::m_voxel_objects_names;
std::vector<std::string> AssetManager::m_tile_materials_names;

std::vector<sf::Texture*> AssetManager::m_palettes;
std::vector<sf::Texture*> AssetManager::m_light_shapes;

int AssetManager::m_colour_increment = 0;

sf::RenderTexture AssetManager::m_texture_atlas;

// reads and interprets the Asset folder
void AssetManager::Init(ScreenData& screen_data){
    // tile materials ______________________________________________________________________________________
    
    std::string directory = "Assets/repeating_tile_materials";

    std::vector<std::string> filenames = {};

    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(directory)){

        filenames.push_back(FilepathFromDirectoryEntry(dir_entry));
    }

    for(int i = 0; i < filenames.size(); i++){
        CreateTileMaterial(RemoveDirectoryFromFilepath(directory, filenames[i]), filenames[i], false);
    }

    // tile objects ______________________________________________________________________________________

    directory = "Assets/tile_objects";
    filenames.clear();
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(directory)){

        filenames.push_back(FilepathFromDirectoryEntry(dir_entry));
    }

    for(int i = 0; i < filenames.size(); i++){
        CreateTileObject(RemoveDirectoryFromFilepath(directory, filenames[i]), filenames[i], screen_data);
    }

    // voxel objects ______________________________________________________________________________________

    directory = "Assets/voxel_objects";
    filenames.clear();
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(directory)){

        filenames.push_back(FilepathFromDirectoryEntry(dir_entry));
    }

    for(int i = 0; i < filenames.size(); i++){
        CreateVoxelObject(RemoveDirectoryFromFilepath(directory, filenames[i]), filenames[i], screen_data, true);
    }

    // voxel objects (NO SIDES)

    directory = "Assets/voxel_objects_NOSIDES";
    filenames.clear();
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(directory)){

        filenames.push_back(FilepathFromDirectoryEntry(dir_entry));
    }

    for(int i = 0; i < filenames.size(); i++){
        CreateVoxelObject(RemoveDirectoryFromFilepath(directory, filenames[i]), filenames[i], screen_data, false);
    }

    // light shapes (for lightmap)

    directory = "Assets/Palettes";
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(directory)){

        m_palettes.push_back(new sf::Texture);
        m_palettes[m_palettes.size() - 1]->loadFromFile(dir_entry.path().string());
    }

    directory = "Assets/LightShapes";
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(directory)){

        m_light_shapes.push_back(new sf::Texture);
        m_light_shapes[m_light_shapes.size() - 1]->loadFromFile(dir_entry.path().string());
    }


    ConstructTextureAtlas();

}

void AssetManager::Destruct(){

    // free heap memory
    for(int i = 0; i < m_tile_objects.size(); i++){
        delete m_tile_objects[i].texture;
        delete m_tile_objects[i].image_texture;
    }
    for(int i = 0; i < m_tile_materials.size(); i++){
        delete m_tile_materials[i].texture;
        delete m_tile_materials[i].image_texture;
    }
    for(int i = 0; i < m_voxel_objects.size(); i++){
        delete m_voxel_objects[i].texture;
        delete m_voxel_objects[i].image_texture;
    }

    for(int i = 0; i < m_palettes.size(); i++){
        delete m_palettes[i];
    }
}

void AssetManager::SaveFinalRender(ScreenData& screen_data){

    sf::Image depth_image = screen_data.m_canvas_depth.getTexture().copyToImage();
    sf::Image final_image = screen_data.m_canvas.getTexture().copyToImage();

    screen_data.m_light_map.display();
    sf::Image light_map_image = screen_data.m_light_map.getTexture().copyToImage();

    for(int x = 0; x < final_image.getSize().x; x++){
        for(int y = 0; y < final_image.getSize().y; y++){

            sf::Color current_colour = final_image.getPixel(x, y);


            // in shadow
            current_colour.b = 255;
            // no shadow
            if(light_map_image.getPixel(x, y) != sf::Color::White){
                current_colour.b = 0;
            }



            // depth will be packed within the alpha channel (its value being the z - 255)
            // so alpha of 255 will be z of 0
            final_image.setPixel(x, y, sf::Color(current_colour.r, current_colour.g, current_colour.b, 255 - depth_image.getPixel(x, y).r));
        }
    }

    final_image.saveToFile("LevelExports/export_main.png");
}


std::string AssetManager::FilepathFromDirectoryEntry(std::filesystem::__cxx11::directory_entry entry){
    // route dir_item into string stream
    std::stringstream ss;
    std::string output;

    ss << entry << std::endl;
    ss >> output;

    // substr to remove quotes
    return output.substr(1, output.size() - 2);
}

std::string AssetManager::RemoveDirectoryFromFilepath(std::string directory, std::string filepath){
    return filepath.substr(directory.size(), filepath.size());
}

void AssetManager::ConstructTextureAtlas(){

    // idea from https://x.com/Billy_Basso/status/1579863034486743040

    int step_size = 16;
    m_texture_atlas.create(16 * 100, 16 * 100);
    m_texture_atlas.clear(sf::Color::Transparent);

    // sort objects by width
    std::sort(m_voxel_objects.begin(), m_voxel_objects.end(), [](const VoxelObject& a, const VoxelObject& b) {
        return a.tile_width > a.tile_height;
    });

    std::sort(m_tile_objects.begin(), m_tile_objects.end(), [](const TileObject& a, const TileObject& b) {
        return a.tile_width > a.tile_height;
    });

    auto doesObjectCollide = [&](int x, int y, int width, int height) {

        // check if new placement would overlap with previous
        for(int i = 0; i < m_voxel_objects.size(); i++){
            if(m_voxel_objects[i].texture_atlas_position_x == -1 || m_voxel_objects[i].texture_atlas_position_y == -1){
                continue;
            }

            if(x + width > m_voxel_objects[i].texture_atlas_position_x && m_voxel_objects[i].texture_atlas_position_x + m_voxel_objects[i].layer_width > x){
                if(y + height > m_voxel_objects[i].texture_atlas_position_y && m_voxel_objects[i].texture_atlas_position_y + m_voxel_objects[i].layer_height > y){
                    return true;
                }
            }
        }

        for(int i = 0; i < m_tile_objects.size(); i++){
            if(m_tile_objects[i].texture_atlas_position_x == -1 || m_tile_objects[i].texture_atlas_position_y == -1){
                continue;
            }

            if(x + width > m_tile_objects[i].texture_atlas_position_x && m_tile_objects[i].texture_atlas_position_x + m_tile_objects[i].layer_width > x){
                if(y + height > m_tile_objects[i].texture_atlas_position_y && m_tile_objects[i].texture_atlas_position_y + m_tile_objects[i].layer_height > y){
                    return true;
                }
            }
        }

        return false;
    };

    for(int i = 0; i < m_voxel_objects.size(); i++){

        for(int y = 0; y < m_texture_atlas.getSize().y; y += step_size){
            bool set_sprite = false;
            for(int x = 0; x < m_texture_atlas.getSize().x; x += step_size){

                if(!doesObjectCollide(x, y, m_voxel_objects[i].tile_width, m_voxel_objects[i].tile_height)){

                    m_voxel_objects[i].sprite_texture.setPosition(x, y);
                    m_texture_atlas.draw(m_voxel_objects[i].sprite_texture);

                    m_voxel_objects[i].texture_atlas_position_x = x;
                    m_voxel_objects[i].texture_atlas_position_y = y;
                    set_sprite = true;
                    break;
                }
            }
            if(set_sprite){
                break;
            }
        }
    }

    for(int i = 0; i < m_tile_objects.size(); i++){

        for(int y = 0; y < m_texture_atlas.getSize().y; y += step_size){
            bool set_sprite = false;
            for(int x = 0; x < m_texture_atlas.getSize().x; x += step_size){

                if(!doesObjectCollide(x, y, m_tile_objects[i].tile_width, m_tile_objects[i].tile_height)){

                    m_tile_objects[i].sprite_texture.setPosition(x, y);
                    m_texture_atlas.draw(m_tile_objects[i].sprite_texture);

                    m_tile_objects[i].texture_atlas_position_x = x;
                    m_tile_objects[i].texture_atlas_position_y = y;
                    set_sprite = true;
                    break;
                }
            }
            if(set_sprite){
                break;
            }
        }
    }

    SaveTextureAtlas();

}
void AssetManager::SaveTextureAtlas(){

    m_texture_atlas.display();
    m_texture_atlas.getTexture().copyToImage().saveToFile("LevelExports/texture_atlas_test.png");
}



void AssetManager::CreateTileMaterial(std::string material_name, std::string file_location, bool bevel_edges){

    m_tile_materials.push_back(TileMaterial{bevel_edges});
    m_tile_materials_names.push_back(material_name);
    int material_index = m_tile_materials.size() - 1;

    m_tile_materials[material_index].texture = new sf::Texture();
    m_tile_materials[material_index].image_texture = new sf::Image();

    m_tile_materials[material_index].texture->loadFromFile(file_location);
    m_tile_materials[material_index].image_texture->loadFromFile(file_location);
}

void AssetManager::CreateVoxelObject(std::string material_name, std::string file_location, ScreenData& screen_data, bool draw_sides){

    m_voxel_objects.push_back(VoxelObject{});
    m_voxel_objects_names.push_back(material_name);
    int material_index = m_voxel_objects.size() - 1;

    m_voxel_objects[material_index].image_texture = new sf::Image();
    m_voxel_objects[material_index].image_texture->loadFromFile(file_location);

    int y_pos = m_voxel_objects[material_index].image_texture->getSize().y - 1;
    // loop from the bottom to find where the schematic ends
    while(y_pos >= 0){

        if(m_voxel_objects[material_index].image_texture->getPixel(0, y_pos) != sf::Color::White){
            break;
        }
        y_pos--;
    }

    m_voxel_objects[material_index].texture = new sf::Texture();
    m_voxel_objects[material_index].texture->loadFromFile(file_location);

    m_voxel_objects[material_index].layer_width = m_voxel_objects[material_index].image_texture->getSize().x;
    m_voxel_objects[material_index].layer_height = m_voxel_objects[material_index].image_texture->getSize().y - y_pos - 1;
    m_voxel_objects[material_index].layer_count = m_voxel_objects[material_index].image_texture->getSize().y / m_voxel_objects[material_index].layer_height - 1;

    // only show bottom white UI hint in sprite
    m_voxel_objects[material_index].sprite_texture.setTexture(*m_voxel_objects[material_index].texture);
    m_voxel_objects[material_index].sprite_texture.setTextureRect(
        sf::IntRect(0, 
                    m_voxel_objects[material_index].texture->getSize().y - m_voxel_objects[material_index].layer_height,
                    m_voxel_objects[material_index].layer_width, 
                    m_voxel_objects[material_index].layer_height));
    
    m_voxel_objects[material_index].sprite_texture.setColor(Util::GetColourFromColourLoop(m_colour_increment));

    m_voxel_objects[material_index].tile_width = floor(m_voxel_objects[material_index].layer_width / screen_data.m_tile_size);
    m_voxel_objects[material_index].tile_height = floor(m_voxel_objects[material_index].layer_height / screen_data.m_tile_size);
     
    m_voxel_objects[material_index].draw_sides = draw_sides;

    m_colour_increment++;
}

void AssetManager::CreateTileObject(std::string material_name, std::string file_location, ScreenData& screen_data){

    m_tile_objects.push_back(TileObject{});
    m_tile_objects_names.push_back(material_name);
    int material_index = m_tile_objects.size() - 1;


    m_tile_objects[material_index].image_texture = new sf::Image();

    if(!m_tile_objects[material_index].image_texture->loadFromFile(file_location)){
        std::cout << "ERROR: Failed to load Voxel Material at " << file_location << "\n";
    }

    int y_pos = m_tile_objects[material_index].image_texture->getSize().y - 1;
    // loop from the bottom to find where the schematic ends
    while(y_pos >= 0){

        if(m_tile_objects[material_index].image_texture->getPixel(0, y_pos) != sf::Color::White){
            break;
        }
        y_pos--;
    }

    m_tile_objects[material_index].texture = new sf::Texture();
    m_tile_objects[material_index].texture->loadFromFile(file_location);

    m_tile_objects[material_index].layer_width = m_tile_objects[material_index].image_texture->getSize().x;
    m_tile_objects[material_index].layer_height = m_tile_objects[material_index].image_texture->getSize().y - y_pos - 1;

    // only show bottom white UI hint in sprite
    m_tile_objects[material_index].sprite_texture.setTexture(*m_tile_objects[material_index].texture);
    m_tile_objects[material_index].sprite_texture.setTextureRect(
        sf::IntRect(0, 
                    m_tile_objects[material_index].texture->getSize().y - m_tile_objects[material_index].layer_height,
                    m_tile_objects[material_index].layer_width, 
                    m_tile_objects[material_index].layer_height));

    m_tile_objects[material_index].sprite_texture.setColor(Util::GetColourFromColourLoop(m_colour_increment));

    m_tile_objects[material_index].tile_width = floor(m_tile_objects[material_index].layer_width / screen_data.m_tile_size);
    m_tile_objects[material_index].tile_height = floor(m_tile_objects[material_index].layer_height / screen_data.m_tile_size);
     
    m_colour_increment++;
}
