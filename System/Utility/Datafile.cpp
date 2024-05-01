#include "Datafile.h"

void Datafile::SetString(const std::string& string, const size_t item_index){

    // we must increase the size of the property array
    if(item_index >= content.size()){
        content.resize(item_index + 1);
    }
    content[item_index] = string;
}

const std::string Datafile::GetString(const size_t item_index) const{
    // index out of bounds, fail silenty
    if(item_index >= content.size()){
        return "";
    }
    else{
        return content[item_index];
    }
} 

void Datafile::SetReal(const double d, const size_t item_index){
    SetString(std::to_string(d), item_index);
}
const double Datafile::GetReal(const size_t item_index) const{
    return std::atof(GetString(item_index).c_str());
}

void Datafile::SetInt(const int32_t i, const size_t item_index){
    SetString(std::to_string(i), item_index);
}
const int32_t Datafile::GetInt(const size_t item_index) const{
    return std::atoi(GetString(item_index).c_str());
}; 
