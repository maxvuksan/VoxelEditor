#pragma once
#include <SFML/Graphics.hpp>
#include "TileMaterial.h"
#include <map>
#include <filesystem>

/*
    holds references to all our materials
*/
class AssetManager {

    public:

        // reads and interprets the Asset folder
        static void Init();

        // converts a directory entry object to a string
        static std::string FilepathFromDirectoryEntry(std::filesystem::__cxx11::directory_entry entry);
        static std::string RemoveDirectoryFromFilepath(std::string directory, std::string filepath);

        /*
            @param material_name the name that shows up in the editor
            @param file_location path to the texture image
            @param bevel_edges do edge tiles have fake bevels drawn?
        */
        static void CreateTileMaterial(std::string material_name, std::string file_location, bool bevel_edges);
    
        static const TileMaterial& GetTileMaterial(std::string material_name);

        static std::vector<std::string> GetAllTileMaterialNames();
        static std::vector<TileMaterial*> GetAllTileMaterials();

    private:

        static std::map<std::string, TileMaterial> m_tile_materials;
};