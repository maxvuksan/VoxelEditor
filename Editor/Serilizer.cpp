#include "Serilizer.h"
#include <windows.h>
#include <shellapi.h>

std::string Serilizer::currentWorkingFile;

bool Serilizer::NewSaveFile(){

    // uses the window API to create a new save file

    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    CHAR currentDirectory[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDirectory);

    // Concatenate the current directory with the "Output" folder name
    char szInitialDir[MAX_PATH];
    sprintf(szInitialDir, "%s\\Output\\", currentDirectory);


    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Voxel Editor Files (*.ve)\0*.ve\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "ve"; // Default extension
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrInitialDir = szInitialDir; // set directory

    if (GetSaveFileName(&ofn)) {
        
        HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        char filePath[MAX_PATH];
        DWORD dwSize = GetFinalPathNameByHandle(hFile, filePath, MAX_PATH, FILE_NAME_NORMALIZED);

        if (dwSize == 0) {
            printf("Error getting file path from handle.\n");
        }

        currentWorkingFile.assign(filePath);

        CloseHandle(hFile);

        return true;
    } else {
        printf("User canceled save operation.\n");
        return false;
    }

}

void Serilizer::SaveScreenData(std::string saveFile, ScreenData& screen_data){
    
    if(!NewSaveFile()){
        return;
    }

    Datafile df;

    df["header"]["tilesize"].SetInt(screen_data.m_tile_size);

    Datafile::Write(df, "../test.txt");

}

void Serilizer::OpenSaveFile(){
    HINSTANCE result = ShellExecuteA(NULL, "open", "Output", NULL, NULL, SW_SHOWNORMAL);
}

void Serilizer::SaveRenderData(std::string saveFile, ScreenData& screen_data){

    Datafile data;
    //data["colliders"][0]

    struct Polygon{
        std::vector<sf::Vector2f> verts;
    };

    std::vector<Polygon> polygons;

    for(int x = 0; x < screen_data.m_tile_layers.at(0).size(); x++){
        for(int y = 0; y < screen_data.m_tile_layers.at(0).at(0).size(); y++){

            if(!screen_data.m_tile_layers[0][x][y].occupied){
                continue;
            }

            Polygon tilePoly;

            switch(screen_data.m_tile_layers[0][x][y].shape){

                case TileShape::SOLID: {
                    // construct square for tile
                    tilePoly.verts.push_back(sf::Vector2f(x, y));
                    tilePoly.verts.push_back(sf::Vector2f(x + 1, y));
                    tilePoly.verts.push_back(sf::Vector2f(x + 1, y + 1));
                    tilePoly.verts.push_back(sf::Vector2f(x, y + 1));
                    break;
                }
                case TileShape::SLOPE_UPRIGHT: {
                    tilePoly.verts.push_back(sf::Vector2f(x, y + 1));
                    tilePoly.verts.push_back(sf::Vector2f(x + 1, y));
                    tilePoly.verts.push_back(sf::Vector2f(x + 1, y + 1));
                    break;
                }
            
                case TileShape::SLOPE_UPLEFT: {
                    tilePoly.verts.push_back(sf::Vector2f(x, y));
                    tilePoly.verts.push_back(sf::Vector2f(x + 1, y + 1));
                    tilePoly.verts.push_back(sf::Vector2f(x, y + 1));
                    break;
                }
                case TileShape::SLOPE_DOWNRIGHT: {
                    tilePoly.verts.push_back(sf::Vector2f(x, y + 1));
                    tilePoly.verts.push_back(sf::Vector2f(x, y));
                    tilePoly.verts.push_back(sf::Vector2f(x + 1, y));
                    break;
                }
                case TileShape::SLOPE_DOWNLEFT: {
                    tilePoly.verts.push_back(sf::Vector2f(x, y));
                    tilePoly.verts.push_back(sf::Vector2f(x + 1, y));
                    tilePoly.verts.push_back(sf::Vector2f(x + 1, y + 1));
                    break;
                }                
            }


            polygons.push_back(tilePoly);

        }
    }


    for(int i = 0; i < polygons.size(); i++){

        int vIndex = 0;
        for(auto& vert : polygons[i].verts){

            data["colliders"][std::to_string(i)].SetInt(vert.x * screen_data.m_tile_size, vIndex);
            vIndex++;
            data["colliders"][std::to_string(i)].SetInt(vert.y * screen_data.m_tile_size, vIndex);
            vIndex++;
        }
    }






    Datafile::Write(data, saveFile);
}

