#ifndef TIMING_CONTROLLER_H
#define TIMING_CONTROLLER_H

#include <Arduino.h>

// --- 全局状态标志位 ---
// volatile关键字确保编译器不会优化掉对该变量的读写，并且保证其在多线程环境（主循环与ISR）中的可见性。
extern volatile bool g_measurementCycleStart; // 测量周期开始标志

/**
 * @brief 定时与控制模块类
 * 
 * 封装了ESP32-C3的GPTimer硬件定时器，用于生成精确的160Hz周期性中断，
 * 并通过ISR设置标志位来驱动主循环中的测量序列。
 */
class TimingController {
public:
    /**
     * @brief 构造函数
     */
    TimingController();

    /**
     * @brief 初始化定时器
     * 
     * 配置GPTimer以1MHz的分辨率运行，并设置一个6250us (160Hz)的周期性警报。
     * 注册ISR回调函数。
     * @return 如果初始化成功，则返回true；否则返回false。
     */
    bool begin();
};

#endif // TIMING_CONTROLLER_H