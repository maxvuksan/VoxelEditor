#include "Time.h"

std::chrono::time_point<std::chrono::system_clock> Time::recording_start;

float Time::deltaTime = 0;
float Time::time = 0;