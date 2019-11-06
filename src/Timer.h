#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h>
#include <stdint.h>
#include <time.h>

class Timer
{
    private:
        struct timespec start, end;
        
    public:
        Timer();
        void Restart();

        uint32_t Micros();
        uint32_t Millis();
        float Seconds();
        float SecondsMax() ;
        float SecondsMin() ;

        static void Wait(int ms = 100);
};

#endif