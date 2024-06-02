#pragma once
#include <iostream>

/*
    A wrapper for new and delete heap interactions to track any possible memory problems.
*/
class Memory {

    public:

        template <typename T>
        static T* New(const char* source){

            T* memory = new T;

            /*
            if(Globals::PROFILE_MEMORY){

                allocations++;
                total_size += sizeof(*memory);

                //std::cout << "New   : " << sizeof(*memory) << " b : " << source << "\n";
                //std::cout << "Size  : " << total_size << " b\n";
                //std::cout << "Count : " << allocations << "\n";
            }
            */

            return memory;
        }

        template <typename T>
        static void Delete(T* memory, const char* source){

            /*
            if(Globals::PROFILE_MEMORY){

                allocations--;
                total_size -= sizeof(*memory);

                //std::cout << "Delete : " << sizeof(*memory) << " b : " << source << "\n";
                //std::cout << "Size  : " << total_size << " b\n";
                //std::cout << "Count : " << allocations << "\n";
            }
            */

            delete memory;
        }

    private:
        static int allocations; // how many allocations on the heap?
        static float total_size; // how many bytes have been allocated?
};