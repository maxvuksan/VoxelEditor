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


