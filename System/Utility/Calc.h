#pragma once
#include <math.h>
#include <SFML/Graphics.hpp>

class Calc{

    public:

        static float EaseOutBack(float x) {

            float c1 = 1.70158;
            float c3 = c1 + 1;

            return 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);
        }

        static float EaseInCubic(float x){
            return x * x * x;
        }
        static float EaseOutCubic(float x){
            return 1 - pow(1 - x, 3);
        }

        // @returns true if @param val is within the min and max value (inclusive)
        static bool InRange(float val, float min, float max){
            if(val > max){
                return false;
            }
            else if(val < min){
                return false;
            }
            return true;        
        }

        // @returns the hypotenuse of a right angle triangle (calculated with pythag)
        static float Hypotenuse(float width, float height){
            return sqrt(pow(width, 2) + pow(height, 2));
        }

        static float Clamp(float val, float min, float max){
            if(val > max){
                val = max;
            }
            else if(val < min){
                val = min;
            }
            return val;
        }
        static float ClampLower(float val, float min){
            if(val < min){
                val = min;
            }
            return val;
        }
        static float ClampUpper(float val, float max){
            if(val > max){
                val = max;
            }
            return val;
        }
        static int ClampLower(int val, int min){
            if(val < min){
                val = min;
            }
            return val;
        }
        static int ClampUpper(int val, int max){
            if(val > max){
                val = max;
            }
            return val;
        }

        // @returns the distance between two points without traversing diagonally 
        static int ManhattenDistance(sf::Vector2i start, sf::Vector2i end){
            return abs(start.x - end.x) + abs(start.y - end.y);
        }
        // @returns the distance between two points without traversing diagonally 
        static float ManhattenDistance(sf::Vector2f start, sf::Vector2f end){
            return abs(start.x - end.x) + abs(start.y - end.y);
        }
        static float Distance(sf::Vector2i start, sf::Vector2i end){
            return sqrt(pow(abs(start.x - end.x), 2) + pow(abs(start.y - end.y), 2));
        }
        static float Distance(sf::Vector2f start, sf::Vector2f end){
            return sqrt(pow(abs(start.x - end.x), 2) + pow(abs(start.y - end.y), 2));
        }
        // @returns the distance squared between 2 points. Though inaccurate to actual distance, is more efficent than Distance() by removing the final sqrt() step.
        static float DistanceSimple(sf::Vector2i start, sf::Vector2i end){
            return pow(abs(start.x - end.x), 2) + pow(abs(start.y - end.y), 2);
        }

        static float Radians(float degrees){
            // using the approximation 1Deg × π/180 = 0.01745Rad
            return degrees * 0.01745;
        }

        static float Degrees(float radians){
            // using the approximation 1Rad × 180/π = 57.296Deg
            return radians * 57.296;
        }

        // @returns the interporlated value between a and b at position t
        static float Lerp(float a, float b, float t){
            return a * (1.0 - t) + b * t;
        }

        // @returns the interporlated vector between a and b by position t
        static sf::Vector2f Lerp(sf::Vector2f a, sf::Vector2f b, float t){
            return sf::Vector2f(Lerp(a.x, b.x, t), Lerp(a.y, b.y, t));
        }

        static sf::Vector3f Lerp(sf::Vector3f a, sf::Vector3f b, float t){
            return sf::Vector3f(Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t));
        }

        // @returns the interpolated colour between a and by position t
        static sf::Color Lerp(sf::Color a, sf::Color b, float t){
            return sf::Color(Lerp(a.r, b.r, t), Lerp(a.g, b.g, t), Lerp(a.b, b.b, t), Lerp(a.a, b.a, t));
        }

        // @returns a vector of magnitude 1 from the provided vector
        static sf::Vector2f Normalize(sf::Vector2f vector){

            float magnitude = Magnitude(vector);
            return sf::Vector2f(vector.x / magnitude, vector.y / magnitude);
        }
        static sf::Vector3f Normalize(sf::Vector3f vector){

            float magnitude = Magnitude(vector);
            return sf::Vector3f(vector.x / magnitude, vector.y / magnitude, vector.z / magnitude);
        }
        // @returns a vector of magnitude 1 from the provided vector
        static sf::Vector2f Normalize(sf::Vector2i vector){
            return Normalize(sf::Vector2f(vector.x, vector.y));
        }

        // @returns the magnitude (hypotenuse) of a vector
        static float Magnitude(sf::Vector2f vector){
            return sqrt(pow(vector.x, 2) + pow(vector.y, 2));
        }

        static float Magnitude(sf::Vector3f vector){
            return sqrt(pow(vector.x, 2) + pow(vector.y, 2) + pow(vector.z, 2));

        }

        // @returns the dot product of two vectors
        static float DotProduct(sf::Vector2f a, sf::Vector2f b){
            return a.x * b.x + a.y * b.y;
        }
        // @returns the dot product of two vectors
        static float DotProduct(sf::Vector3f a, sf::Vector3f b){
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        // @returns the angle between two points in degreess
        static float AngleBetween(sf::Vector2f a, sf::Vector2f b){
            return Degrees(RadiansBetween(a, b));
        }
        // @returns the angle between two points in radians
        static float RadiansBetween(sf::Vector2f a, sf::Vector2f b){
            sf::Vector2f dif = b - a;
            return atan2(dif.y, dif.x);
        }

        // @returns the a normalized vector pointing from a to b
        static sf::Vector2f VectorBetween(sf::Vector2f a, sf::Vector2f b){
            float rad = RadiansBetween(a, b);
            return RadiansToVector(rad);
        }



        static float VectorToRadians(sf::Vector2f vector){
            return atan2(vector.y, vector.x);
        }

        static sf::Vector2f RadiansToVector(float radians){
            return sf::Vector2f(cos(radians), sin(radians));
        }
};