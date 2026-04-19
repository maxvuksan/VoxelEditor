#include <SFML/Graphics.hpp>
#include "ScreenData.h"
#include "../System/Utility/Datafile.h"

class Serilizer {

    public: 

        // level editor file
        static void SaveScreenData(std::string saveFile, ScreenData& screen_data);
        static void OpenSaveFile();

        // final render file
        static void SaveRenderData(std::string saveFile, ScreenData& screen_data);

        static std::string GetCurrentWorkingFile(){return currentWorkingFile;}


    private:

        // @returns false if user cancels save
        static bool NewSaveFile();


    static std::string currentWorkingFile;
};