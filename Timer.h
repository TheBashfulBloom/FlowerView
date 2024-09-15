#pragma once
#ifndef TIMER_H
#define TIMER_H

#include <windows.h>
#define WARNING_SECONDS_UPLIMIT 20.0
class Timer
{
public:
    Timer();

    // 启动定时器，输入游戏起始秒数
    void start(double gameStartSeconds);

    // 检查游戏时间，输入游戏当前秒数
    void checkGameTime(double gameCurrentSeconds);

    // 重置定时器
    void reset();

    // 判断是否需要发出警告
    bool isWarning() const;

private:
    // 获取当前时间（秒）
    double getCurrentTimeInSeconds();

    double startTime;          // 游戏起始秒数
    double currentTime;        // 游戏当前秒数
    double lastCheckTime;      // 上次检查时间
    double accumulatedError;  // 累积误差
    double warningThreshold;   // 超限预警秒数阈值
    bool isRunning;            // 定时器是否正在运行

public:
    bool get_isRunning();
};

#endif // TIMER_H

