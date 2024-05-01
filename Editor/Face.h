#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

/*
    assumed to make up either 3 or 4 verticies (tri or quad)
*/
struct Face{

    std::vector<sf::Vector3f> m_vert_positions;
};