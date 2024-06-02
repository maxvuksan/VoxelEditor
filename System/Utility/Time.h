#pragma once
#include <chrono>
#include <iostream>

/*
    Keeping track of everything time related
*/
class Time {



	public:

		/*
			total elapsed time in seconds
		*/
		static float GetSeconds() { return time; }
		/*
			total elapsed time in milliseconds
		*/
		static float GetMilliseconds() { return time * 1000.0f; }

		static float FPS() { return 1.0f / deltaTime;}


		/*
			returns the time elapsed since the last frame, in seconds
		*/
		static float DeltaTimeRaw() { return deltaTime; }
        /*
            returns the time elapsed since the last frame, in milliseconds
        */
		static float DeltaTime() { return deltaTime * 1000.0f; }
		// short hand for Time::DeltaTime()
		static float Dt() { return deltaTime * 1000.0f; }

        /* sets the delta time value */
		static void SetDeltaTime(float dt) { deltaTime = dt; }

        /* increments the total elapsed time by delta time*/
		static void Increment() { time += deltaTime; }
		
        /* begins a performance recording, can be stopped and printed with Time::EndRecord() */
        static void StartRecord(){
            recording_start = std::chrono::system_clock::now();
        }
        /* ends a recording printing the elapsed time to std::out, should be started with Time::StartRecord() */
        static void EndRecord(){
            //std::cout << "Time::EndRecord(): " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - recording_start).count() << "ms\n";
        }
        static void EndRecordNano(){
            //std::cout << "Time::EndRecord(): " << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - recording_start).count() << "ns\n";
        }

	private:
        static std::chrono::time_point<std::chrono::system_clock> recording_start;

		static float deltaTime;
		static float time;
};