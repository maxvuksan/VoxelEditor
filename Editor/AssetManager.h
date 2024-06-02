#pragma once
#include <SFML/Graphics.hpp>
#include "TileMaterial.h"
#include "VoxelObject.h"
#include "TileObject.h"
#include <map>
#include <filesystem>

/*
    holds references to all our materials
*/
class ScreenData;
class AssetManager {

    public:

        // reads and interprets the Asset folder
        static void Init(ScreenData& screen_data);
        static void Destruct();
        static void SaveFinalRender(ScreenData& screen_data);

        // converts a directory entry object to a string
        static std::string FilepathFromDirectoryEntry(std::filesystem::__cxx11::directory_entry entry);
        static std::string RemoveDirectoryFromFilepath(std::string directory, std::string filepath);

        static void ConstructTextureAtlas();
        static sf::RenderTexture& GetTextureAtlas(){m_texture_atlas.display(); return m_texture_atlas;}

        static void SaveTextureAtlas();

        /*
            @param material_name the name that shows up in the editor
            @param file_location path to the texture image
            @param bevel_edges do edge tiles have fake bevels drawn?
        */
        static void CreateTileMaterial(std::string material_name, std::string file_location, bool bevel_edges);
        static void CreateVoxelObject(std::string material_name, std::string file_location, ScreenData& screen_data, bool draw_sides = false);
        static void CreateTileObject(std::string material_name, std::string file_location, ScreenData& screen_data);

        static const TileMaterial& GetTileMaterial(int material_index){return m_tile_materials[material_index];}
        static const TileObject& GetTileObject(int material_index){return m_tile_objects[material_index];}
        static const VoxelObject& GetVoxelObject(int material_index){return m_voxel_objects[material_index];}

        static const std::vector<std::string>& GetAllTileMaterialNames(){return m_tile_materials_names;}
        static const std::vector<TileMaterial>& GetAllTileMaterials(){return m_tile_materials;}

        static const std::vector<std::string>& GetAllVoxelObjectNames(){return m_voxel_objects_names;}
        static const std::vector<VoxelObject>& GetAllVoxelObjects(){return m_voxel_objects;}

        static std::vector<std::string> GetAllTileObjectNames(){return m_tile_objects_names;}
        static const std::vector<TileObject>& GetAllTileObjects(){return m_tile_objects;}

        static const std::vector<sf::Texture*>& GetPalettes(){return m_palettes;}
        static const std::vector<sf::Texture*>& GetLightShapes(){return m_light_shapes;}

    private:

        static int m_colour_increment; // only used to determine colour
        
        static sf::RenderTexture m_texture_atlas;

        static std::vector<TileObject> m_tile_objects;
        static std::vector<VoxelObject> m_voxel_objects;
        static std::vector<TileMaterial> m_tile_materials;

        static std::vector<std::string> m_tile_objects_names;
        static std::vector<std::string> m_voxel_objects_names;
        static std::vector<std::string> m_tile_materials_names;

        static std::vector<sf::Texture*> m_palettes;
        static std::vector<sf::Texture*> m_light_shapes;
};