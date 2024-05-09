#include "AssetManager.h"
#include <iostream>
#include "Util.h"
#include <filesystem>
#include "ScreenData.h"

std::map<std::string, TileMaterial> AssetManager::m_tile_materials;
std::map<std::string, VoxelMaterial> AssetManager::m_voxel_materials;

int AssetManager::m_voxel_material_increment = 0;

// reads and interprets the Asset folder
void AssetManager::Init(ScreenData& screen_data){

    // tile materials ______________________________________________________________________________________
    
    std::string tile_directory = "Assets/tile_materials";

    std::vector<std::string> tile_material_filenames = {};

    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(tile_directory)){

        tile_material_filenames.push_back(FilepathFromDirectoryEntry(dir_entry));
    }

    for(int i = 0; i < tile_material_filenames.size(); i++){
        CreateTileMaterial(RemoveDirectoryFromFilepath(tile_directory, tile_material_filenames[i]), tile_material_filenames[i], false);
    }

    // voxel materials ______________________________________________________________________________________

    std::string voxel_directory = "Assets/voxel_materials";
    std::vector<std::string> voxel_material_filenames = {};
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(voxel_directory)){

        voxel_material_filenames.push_back(FilepathFromDirectoryEntry(dir_entry));
    }

    for(int i = 0; i < voxel_material_filenames.size(); i++){
        CreateVoxelMaterial(RemoveDirectoryFromFilepath(voxel_directory, voxel_material_filenames[i]), voxel_material_filenames[i], screen_data, true);
    }

    // flat voxel materials (drawn with no sides)
    std::string voxel_directory_2D = "Assets/voxel_materials_2D";
    std::vector<std::string> voxel_material_2D_filenames = {};
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(voxel_directory_2D)){

        voxel_material_2D_filenames.push_back(FilepathFromDirectoryEntry(dir_entry));
    }

    for(int i = 0; i < voxel_material_2D_filenames.size(); i++){
        CreateVoxelMaterial(RemoveDirectoryFromFilepath(voxel_directory_2D, voxel_material_2D_filenames[i]), voxel_material_2D_filenames[i], screen_data, false);
    }

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

void AssetManager::CreateTileMaterial(std::string material_name, std::string file_location, bool bevel_edges){

    m_tile_materials.insert(std::pair<std::string, TileMaterial>(material_name, TileMaterial{bevel_edges}));
    
    if(!m_tile_materials[material_name].texture.loadFromFile(file_location)){
        std::cout << "ERROR: Failed to load Tile Material (texture) at " << file_location << "\n";
    }
    if(!m_tile_materials[material_name].image_texture.loadFromFile(file_location)){
        std::cout << "ERROR: Failed to load Tile Material (image_texture) at " << file_location << "\n";
    }

}

void AssetManager::CreateVoxelMaterial(std::string material_name, std::string file_location, ScreenData& screen_data, bool draw_sides){

    m_voxel_materials.insert(std::pair<std::string, VoxelMaterial>(material_name, VoxelMaterial{}));
    
    if(!m_voxel_materials[material_name].image_texture.loadFromFile(file_location)){
        std::cout << "ERROR: Failed to load Voxel Material at " << file_location << "\n";
    }


    int y_pos = m_voxel_materials[material_name].image_texture.getSize().y - 1;
    // loop from the bottom to find where the schematic ends
    while(y_pos >= 0){

        if(m_voxel_materials[material_name].image_texture.getPixel(0, y_pos) != sf::Color::White){
            break;
        }
        y_pos--;
    }

    m_voxel_materials[material_name].texture.loadFromImage(m_voxel_materials[material_name].image_texture);

    m_voxel_materials[material_name].layer_width = m_voxel_materials[material_name].image_texture.getSize().x;
    m_voxel_materials[material_name].layer_height = m_voxel_materials[material_name].image_texture.getSize().y - y_pos - 1;
    m_voxel_materials[material_name].layer_count = m_voxel_materials[material_name].image_texture.getSize().y / m_voxel_materials[material_name].layer_height - 1;

    // only show bottom white UI hint in sprite
    m_voxel_materials[material_name].sprite_texture.setTexture(m_voxel_materials[material_name].texture);
    m_voxel_materials[material_name].sprite_texture.setTextureRect(
        sf::IntRect(0, 
                    m_voxel_materials[material_name].texture.getSize().y - m_voxel_materials[material_name].layer_height,
                    m_voxel_materials[material_name].layer_width, 
                    m_voxel_materials[material_name].layer_height));

    m_voxel_materials[material_name].sprite_texture.setColor(Util::GetColourFromColourLoop(m_voxel_material_increment));

    m_voxel_materials[material_name].tile_width = floor(m_voxel_materials[material_name].layer_width / screen_data.m_tile_size);
    m_voxel_materials[material_name].tile_height = floor(m_voxel_materials[material_name].layer_height / screen_data.m_tile_size);
     
    m_voxel_materials[material_name].draw_sides = draw_sides;

    m_voxel_material_increment++;
}


const TileMaterial& AssetManager::GetTileMaterial(std::string material_name){

    if(m_tile_materials.find(material_name) == m_tile_materials.end()){
        std::cout << "ERROR: Could not find tile material of name " << material_name << ", AssetManager::GetTileMaterial()\n";
    }

    return m_tile_materials[material_name];
}

const VoxelMaterial& AssetManager::GetVoxelMaterial(std::string material_name){

    if(m_voxel_materials.find(material_name) == m_voxel_materials.end()){
        std::cout << "ERROR: Could not find voxel material of name " << material_name << ", AssetManager::GetVoxelMaterial()\n";
    }

    return m_voxel_materials[material_name];
}

std::vector<std::string> AssetManager::GetAllTileMaterialNames(){

    std::vector<std::string> names = {};

    for(auto mat : m_tile_materials){
        names.push_back(mat.first);
    }
    return names;
}
std::vector<TileMaterial*> AssetManager::GetAllTileMaterials(){

    std::vector<TileMaterial*> mats = {};

    if(m_tile_materials.size() == 0){
        std::cout << "ERROR: No tile materials created, AssetManager::GetAllTileMaterials()\n";
    }

    for(auto& mat : m_tile_materials){
        mats.push_back(&mat.second);
    }


    return mats;
}

std::vector<std::string> AssetManager::GetAllVoxelMaterialNames(){

    std::vector<std::string> names = {};

    for(auto mat : m_voxel_materials){
        names.push_back(mat.first);
    }
    return names;
}
std::vector<VoxelMaterial*> AssetManager::GetAllVoxelMaterials(){

    std::vector<VoxelMaterial*> mats = {};

    if(m_voxel_materials.size() == 0){
        std::cout << "ERROR: No voxel materials created, AssetManager::GetAllTileMaterials()\n";
    }

    for(auto& mat : m_voxel_materials){
        mats.push_back(&mat.second);
    }


    return mats;
}