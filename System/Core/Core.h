#pragma once

#include <SFML/Graphics.hpp>

///////////////////////////////

#include "../Utility/Time.h"


class Core{

    /*
        the entry point, acts as the brains of the program.
    */

    public:
        Core(int width, int height, std::string title = "Untitled Project");

        // starts the program, i.e begins the game loop
        void Run();
        virtual void Cleanup(){}

        // is called directly after Run()
        virtual void Start(){}
        // is called every frame while the game loop is running
        virtual void Update(){}
        // is called whenever a event occurs
        virtual void CatchEvent(const sf::Event& event){}

        
    protected:

        static sf::RenderWindow m_window;
        static sf::Clock m_delta_clock;
};