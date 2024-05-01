#include "Core.h"

/////////////////////////////////////////////////////////

sf::RenderWindow Core::m_window;
sf::Clock Core::m_delta_clock;

/////////////////////////////////////////////////////////


Core::Core(int width, int height, std::string title){

    m_window.create(sf::VideoMode(width, height), title);
    m_window.setFramerateLimit(140);
}


void Core::Run(){

    this->Start();

    while (m_window.isOpen())
    {
        m_window.clear();

        // catch events
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed){
                m_window.close();
            }
            if(event.type == sf::Event::Resized){
                
                m_window.setSize(sf::Vector2u(event.size.width, event.size.height));
                
                sf::View new_view = sf::View(sf::FloatRect(0.f, 0.f, event.size.width, event.size.height));
                m_window.setView(new_view);
            }

            this->CatchEvent(event);
        }

        // calculating delta time...
        sf::Time dt = m_delta_clock.restart();
        Time::SetDeltaTime(dt.asSeconds());

        // performing updates
        this->Update();

        m_window.display();
        Time::Increment();
    }
    
}