#pragma once

#include <sstream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <functional>
#include <stack>
#include <iostream>
/*
    a custom format for saving and reading from files
    (similar to json and YAML)

    based off javidx9s approach
    https://www.youtube.com/watch?v=jlS1Y2-yKV0&t=506s
*/
class Datafile{

    public:

        void SetString(const std::string& string, const size_t item_index = 0);
        const std::string GetString(const size_t item_index = 0) const; 

        void SetReal(const double d, const size_t item_index = 0);
        const double GetReal(const size_t item_index = 0) const; 

        void SetInt(const int32_t d, const size_t item_index = 0);
        const int32_t GetInt(const size_t item_index = 0) const; 

        size_t GetValueCount() const{ return content.size();}

        inline Datafile& operator [](const std::string& name){
            
            // check if nodes map already contains this name
            if(map_objects.count(name) == 0){

                // name does not exist, create it
                map_objects[name] = vec_objects.size();
                vec_objects.push_back({name, Datafile()});
            }

            // it exists so return the object,
            // we essentially use the unordered maps constant retrevial to get the vectors index
            return vec_objects[map_objects[name]].second;
        }


        /*
            @param n the Datafile we are saving to 
            @param file_name the file path we are reading from
            @param node_to_stop_at will terminate all reading when this node is reached, allows us to not read the whole file
            
            if say our node structure is

            - header {}
            - world {}
        
            and we dont want to read anything beyond and including world we can specify the node_to_stop_at as "world"

            making the Read only return

            - header {}
        */
        inline static bool Read(
            Datafile& n,
            const std::string& file_name,
            const std::string& node_to_stop_at = "",
            const char list_seperator = ','
            )
        {

            // open the file
            std::ifstream file(file_name);
            if(file.is_open()){

                // the data of the current property
                std::string prop_name;
                std::string prop_value;

                std::stack<std::reference_wrapper<Datafile>> stack_path;
                stack_path.push(n);

                // read file line by line
                while(!file.eof()){
                    
                    std::string line;
                    std::getline(file, line);

                    // removes the white space from the start and end of string
                    auto trim_whitespace = [](std::string& s){
                        s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
                        s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
                    };

                    trim_whitespace(line);

                    // the line has content
                    if(!line.empty()){

                        size_t x = line.find_first_of('=');
                        // we have an assignment
                        if(x != std::string::npos){

                            // trim possible white space
                            prop_name = line.substr(0, x);
                            trim_whitespace(prop_name);


                            prop_value = line.substr(x + 1, line.size());
                            trim_whitespace(prop_value);


                                                //   _____
                            // e.g. list = a, b, c, "d, e", f
                            bool in_quotes = false;
                            std::string token;
                            size_t token_count = 0;

                            for(const auto c : prop_value){

                                if(c == '\"'){
                                    // in quotes, toggle quote state
                                    in_quotes = !in_quotes;
                                }
                                else{
                                    // no, proceed creating token
                                    if(in_quotes){
                                        token.append(1, c);
                                    }
                                    else{

                                        if(c == list_seperator){
                                            trim_whitespace(token);
                                            stack_path.top().get()[prop_name].SetString(token, token_count);
                                            token.clear();
                                            token_count++;
                                        }
                                        else{
                                            token.append(1, c);
                                        }
                                    }
                                }
                            }

                            // add any end of line tokens
                            if(!token.empty()){
                                trim_whitespace(token);
                                stack_path.top().get()[prop_name].SetString(token, token_count);
                            }

                        }
                        else{ // no =

                            if(line[0] == '{'){
                                stack_path.push(stack_path.top().get()[prop_name]);
                            }
                            else if(line[0] == '}'){
                                stack_path.pop();
                            }
                            else{
                                // name floating in space COULD be followed by { or }
                                /*
                                    e,g,

                                    pc
                                    {

                                    }
                                */
                                prop_name = line;

                                if(prop_name == node_to_stop_at){
                                    break;
                                }
                            }

                        }
                    }

                
                }

                file.close();
                return true;
            }
            else{
                return false;
            }
        }

        // writes a "Datafile" node and all of its child nodes recursivley into a file
        inline static bool Write(
            const Datafile& n,
            const std::string& file_name,
            const std::string& indent_char = "\t",
            const char list_seperator = ',')
        {

            std::string string_seperator = std::string(1, list_seperator) + " ";
            size_t indent_count = 0;

            // local function to recursivley call
            std::function<void(const Datafile&, std::ofstream&)> write = [&](const Datafile& n, std::ofstream& file){

                auto indent = [&](const std::string& string, const size_t count){

                    std::string string_out;
                    for(size_t n = 0; n < count; n++){
                        string_out += string;
                    }
                    return string_out;
                };


                for(auto const& property : n.vec_objects){

                    if(property.second.vec_objects.empty()){
                        

                        file << indent(indent_char, indent_count) << property.first << " = ";

                        size_t number_of_items = property.second.GetValueCount();

                        for(size_t i = 0; i < property.second.GetValueCount(); i++){


                            /*
                                if the value being written is in string form we must write in quotations,
                                this is because in the case a string contains ',' seperating characters our program will confuse it
                                for the start of the next element
                            */

                           size_t x = property.second.GetString(i).find_first_of(list_seperator);

                            if(x != std::string::npos){
                                // value contains seperator, wrap in quotes
                                file << "\"" << property.second.GetString(i) << "\"" << ((number_of_items > 1) ? string_seperator : "");
                            }
                            else{
                                // value has no seperator, just write it out normally
                                file << property.second.GetString(i) << ((number_of_items > 1) ? string_seperator : "");
                            }

                            number_of_items--;

                        }
                        // done with this line
                        file << "\n";
                    }
                    else{
                        // node has child node
                        file << "\n" << indent(indent_char, indent_count) << property.first << "\n";
                        file << indent(indent_char, indent_count) << "{\n";
                        indent_count++;

                        write(property.second, file);
                        file << indent(indent_char, indent_count) << "}\n\n";
                    }
                }
                if(indent_count > 0){
                    indent_count--;
                }
            };


            // open file
            std::ofstream file(file_name);
            if(file.is_open()){

                write(n, file);
                file.close();
                return true;
            }
            return false;
        }


    private:

        // a datafile (or node) may have mixed types
        // node = 1, 3, "test", 2.4

        // the list of strings which make up a property value
        // every data type is stored as a string
        std::vector<std::string> content;

        std::vector<std::pair<std::string, Datafile>> vec_objects;
        std::unordered_map<std::string, size_t> map_objects;
};