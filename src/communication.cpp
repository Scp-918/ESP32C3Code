#include "communication.h"
#include "driver/uart.h"

// 16帧的发送缓冲区
DataFrame commBuffer[COMM_BUFFER_FRAME_COUNT];
int frameCount = 0;

void initCommunication() {
    uart_config_t uart_config = {
        .baud_rate = SERIAL_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    
    // 配置UART0，并安装驱动，启用DMA发送
    // RX/TX buffer size: 256 bytes
    uart_driver_install(UART_NUM_0, 256, 256, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    
    // 设置UART引脚
    // !!!关键改动!!!: 明确指定UART0的TX和RX引脚，确保DMA正确工作。
    // ESP32-C3的UART0默认引脚为TXD(GPIO21)和RXD(GPIO20)
    uart_set_pin(UART_NUM_0, 
                 GPIO_NUM_21, // TXD
                 GPIO_NUM_20, // RXD
                 UART_PIN_NO_CHANGE, // RTS
                 UART_PIN_NO_CHANGE); // CTS

    // 重新配置Arduino的Serial以使用DMA
    Serial.begin(SERIAL_BAUD_RATE);
}

void initCommunication2() {
    Serial.begin(SERIAL_BAUD_RATE);
}

// 新的非阻塞发送函数，使用DMA将数据帧发送到串口
void sendDataFrame(const DataFrame* frame) {
    // uart_write_bytes是非阻塞的，它将数据拷贝到内部DMA缓冲区，并立即返回
    // 如果DMA缓冲区满，它只会写入能容纳的数据量，并返回实际写入的字节数
    uart_write_bytes(UART_NUM_0, (const char*)frame, sizeof(DataFrame));
}

// 原来的函数不再被使用，但为了保留原始代码，此处保留
void addDataToBuffer(uint16_t ad7680_data, uint16_t ads1220_data) {
    if (frameCount < COMM_BUFFER_FRAME_COUNT) {
        // 获取当前帧的引用
        DataFrame &frame = commBuffer[frameCount];

        frame.header1 = FRAME_HEADER1;
        frame.header2 = FRAME_HEADER2;

        // 填充16位ADC数据 (大端模式, MSB first)
        frame.adc_datahigh = (ad7680_data >> 8) & 0xFF;
        frame.adc_datalow = ad7680_data & 0xFF;

        // 填充24位AFE数据 (大端模式, MSB first)
        frame.afe_datahigh = 0x00;
        frame.afe_datamedium = (ads1220_data >> 8) & 0xFF;
        frame.afe_datalow = ads1220_data & 0xFF;

        // 计算校验和：前面数据字节的异或
        frame.checksum = frame.adc_datahigh ^ frame.adc_datalow ^ frame.afe_datahigh ^ frame.afe_datamedium ^ frame.afe_datalow;

        frame.footer1 = FRAME_FOOTER1;
        frame.footer2 = FRAME_FOOTER2;

        frameCount++;
    }
}

void sendBufferIfFull() {
    if (frameCount >= COMM_BUFFER_FRAME_COUNT) {
        // 将整个缓冲区作为字节数组发送
        Serial.write((uint8_t*)commBuffer, sizeof(commBuffer));
        // 重置计数器
        frameCount = 0;
    }
}

void addDataToBufferSingle() {
    // 此函数未使用，保留
}

void sendBufferIfFullSingle() {
    // 此函数未使用，保留
}