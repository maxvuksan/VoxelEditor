#pragma once
#include <iostream>

enum GenerationType{

    Shadow, // darkens by 1 step
    Overshadow, // sets all voxels to the darkest colour
    Melt, 
    Roots, // placed on ceilings
    RootsChaotic,
    ThickRoots,
    ThickRootsChaotic,
    NUMBER_OF_GENERATION_TYPES,
};


struct GenerativeEffect {

    std::string name;
    GenerationType type;    
};
