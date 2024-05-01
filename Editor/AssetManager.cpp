#include "AssetManager.h"
#include <iostream>
#include <filesystem>

std::map<std::string, TileMaterial> AssetManager::m_tile_materials;


// reads and interprets the Asset folder
void AssetManager::Init(){

    std::string tile_directory = "Assets/tile_materials";

    std::vector<std::string> material_filenames = {};

    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(tile_directory)){

        material_filenames.push_back(FilepathFromDirectoryEntry(dir_entry));
    }

    for(int i = 0; i < material_filenames.size(); i++){
        CreateTileMaterial(RemoveDirectoryFromFilepath(tile_directory, material_filenames[i]), material_filenames[i], false);
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
        std::cout << "ERROR: Failed to load Tile Material at " << file_location << "\n";
    }

}

const TileMaterial& AssetManager::GetTileMaterial(std::string material_name){

    if(m_tile_materials.find(material_name) == m_tile_materials.end()){
        std::cout << "ERROR: Could not find material of name " << material_name << ", AssetManager::GetTileMaterial()\n";
    }

    return m_tile_materials[material_name];
}

std::vector<std::string> AssetManager::GetAllTileMaterialNames(){

    std::vector<std::string> names = {};

    for(auto mat : m_tile_materials){
        names.push_back(mat.first);
    }
    return names;
}
std::vector<TileMaterial*> AssetManager::GetAllTileMaterials(){

    std::vector<TileMaterial*> names = {};

    if(m_tile_materials.size() == 0){
        std::cout << "ERROR: No materials created, AssetManager::GetAllTileMaterials()\n";
    }

    for(auto& mat : m_tile_materials){
        names.push_back(&mat.second);
    }


    return names;
}