#pragma once
#ifndef TIME_HPP
#define TIME_HPP

#include <iostream>
#include <ostream>
#include <cstring>
#include <ctime>

using namespace std;

#define TIME_SIZE 40

class Time
{
    private:
        Time(const Time& src);
        Time& operator=(Time const& rhs);
        Time();
        ~Time();

    public:
        static void         stamp(char *timeStamp);
};

#endif // TIME_HPP