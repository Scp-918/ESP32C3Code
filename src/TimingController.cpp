#include "TimingController.h"
#include "driver/gptimer.h"

// --- 全局状态标志位定义 ---
volatile bool g_measurementCycleStart = false;

// --- 引脚定义 ---
// 将所有硬件相关的引脚定义集中管理，便于修改和维护。
// 根据表1的引脚分配策略定义
const int SW_CTRL_PIN = 6; // 模拟开关控制引脚

// --- 定时器句柄 ---
static gptimer_handle_t gptimer = NULL;

/**
 * @brief 定时器警报中断服务程序 (ISR)
 * 
 * IRAM_ATTR 属性: 将此函数放入指令RAM中，以获得更低的中断延迟。
 * 这个ISR的职责是最小化的：仅执行时间最关键的操作（切换模拟开关）并设置一个标志位。
 * 所有耗时的操作（如SPI通信）都应在主循环中响应此标志位来完成。
 * 
 * @param timer 定时器句柄
 * @param edata 警报事件数据
 * @param user_data 用户数据指针
 * @return bool 返回true表示需要进行上下文切换（通常在FreeRTOS任务中使用），此处返回false。
 */
static bool IRAM_ATTR timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data) {
    // 1. 将模拟开关切换到惠斯通电桥通路（高电平激励）
    // 这是整个周期的起点，时间精度要求最高。
    digitalWrite(SW_CTRL_PIN, HIGH);//高电平IO给开关接通惠斯通电桥

    // 2. 设置全局标志位，通知主循环开始新的测量周期
    g_measurementCycleStart = true;

    return false;
}

TimingController::TimingController() {
    // 构造函数可以为空，主要逻辑在begin()中实现
}

bool TimingController::begin() {
    // 初始化模拟开关控制引脚
    pinMode(SW_CTRL_PIN, OUTPUT);
    digitalWrite(SW_CTRL_PIN, LOW); // 默认设置为低电平激励通路

    // 1. 配置定时器
    gptimer_config_t timer_config = {
       .clk_src = GPTIMER_CLK_SRC_DEFAULT, // 使用默认时钟源 (APB_CLK)
       .direction = GPTIMER_COUNT_UP,      // 向上计数
       .resolution_hz = 1 * 1000 * 1000,   // 分辨率设置为 1MHz, 1 tick = 1us
    };
    esp_err_t ret = gptimer_new_timer(&timer_config, &gptimer);
    if (ret!= ESP_OK) {
        Serial.println("创建GPTimer失败");
        return false;
    }

    // 2. 配置警报动作
    gptimer_alarm_config_t alarm_config = {
       .alarm_count = 6250,                // 警报计数值: 6250us -> 160Hz
       .reload_count = 0,                  // 警报触发后，计数器重载为0
       .flags = {
           .auto_reload_on_alarm = true,   // 启用自动重载
        },
    };
    ret = gptimer_set_alarm_action(gptimer, &alarm_config);
    if (ret!= ESP_OK) {
        Serial.println("设置警报动作失败");
        return false;
    }

    // 3. 注册事件回调函数
    gptimer_event_callbacks_t cbs = {
       .on_alarm = timer_on_alarm_cb, // 注册我们的ISR
    };
    ret = gptimer_register_event_callbacks(gptimer, &cbs, NULL);
    if (ret!= ESP_OK) {
        Serial.println("注册回调函数失败");
        return false;
    }

    // 4. 使能并启动定时器
    ret = gptimer_enable(gptimer);
    if (ret!= ESP_OK) {
        Serial.println("使能定时器失败");
        return false;
    }
    
    ret = gptimer_start(gptimer);
    if (ret!= ESP_OK) {
        Serial.println("启动定时器失败");
        return false;
    }

    Serial.println("定时器初始化并启动成功，频率160Hz。");
    return true;
}