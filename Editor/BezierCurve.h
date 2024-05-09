#pragma once
#include <SFML/Graphics.hpp>

struct BezierCurve{

    
    sf::Vector2f start;
    sf::Vector2f end;

    float slack = 60; // determines where the anchor is placed, the tightness of the rope

    sf::Vector2f anchor;


    // smaller increment, more points (more perfect curve)
    std::vector<sf::Vector2f> SamplePositions(float increment = 0.025){

        std::vector<sf::Vector2f> positions;

        anchor = Calc::Lerp(start, end, 0.5);
        anchor.y += slack;

        float t = 0;

        while(t <= 1){
        
            positions.push_back(SampleAtT(t));

            t += increment;
        }
        return positions;
    }

    inline sf::Vector2f SampleAtT(float t){
        sf::Vector2f lerp_a = Calc::Lerp(start, anchor, t);
        sf::Vector2f lerp_b = Calc::Lerp(anchor, end, t);

        return Calc::Lerp(lerp_a, lerp_b, t);
        
    }

};