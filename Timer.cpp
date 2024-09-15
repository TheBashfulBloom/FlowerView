#include "Timer.h"
#include <cmath> // For std::abs
#include <iostream>

Timer::Timer()
    : startTime(0), currentTime(0), lastCheckTime(0), accumulatedError(0), warningThreshold(WARNING_SECONDS_UPLIMIT ), isRunning(false)
{}

void Timer::start(double gameStartSeconds)
{
    startTime = getCurrentTimeInSeconds();
    currentTime = gameStartSeconds;
    accumulatedError = 0;
    isRunning = true;
    lastCheckTime = startTime;
}

bool Timer::get_isRunning(){ return isRunning; }

void Timer::checkGameTime(double gameCurrentSeconds)
{
    if (!isRunning)
        return;

    double now = getCurrentTimeInSeconds();
    double elapsedGameTime = gameCurrentSeconds - currentTime;
    double elapsedTimerTime = now - lastCheckTime;
    double error = std::abs(elapsedGameTime - elapsedTimerTime);

    accumulatedError += error;

    if (error / elapsedGameTime < 0.05) // 如果误差比例小于5%
    {
        reset();
    }
    else if (isWarning())
    {
        // 处理警告情况
        std::cout << "Warning: Accumulated error exceeded threshold." << std::endl;
    }

    lastCheckTime = now;
    currentTime = gameCurrentSeconds;
}

void Timer::reset()
{
    accumulatedError = 0;
    lastCheckTime = getCurrentTimeInSeconds();
}

bool Timer::isWarning() const
{
    return accumulatedError > warningThreshold;
}

double Timer::getCurrentTimeInSeconds()
{
    FILETIME fileTime;
    GetSystemTimeAsFileTime(&fileTime);
    ULARGE_INTEGER ul;
    ul.LowPart = fileTime.dwLowDateTime;
    ul.HighPart = fileTime.dwHighDateTime;
    return (ul.QuadPart / 10000000.0) - 11644473600.0; // Convert to seconds since epoch
}


