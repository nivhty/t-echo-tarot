#include "power.h"
#include "t_echo_pins.h"
#include "config.h"
#include <nrf_gpio.h>

void disableAllRadios() {
    // LoRa SX1262 pins — tri-state to prevent parasitic current
    pinMode(LoRa_Cs, INPUT);
    pinMode(LoRa_Rst, INPUT);
    pinMode(LoRa_Busy, INPUT);
    pinMode(LoRa_Dio1, INPUT);
    pinMode(LoRa_Mosi, INPUT);
    pinMode(LoRa_Sclk, INPUT);
    pinMode(LoRa_Miso, INPUT);

    // GPS L76K pins — never init UART, leave dormant
    pinMode(Gps_Tx_Pin, INPUT);
    pinMode(Gps_Rx_Pin, INPUT);
    pinMode(Gps_Wakeup_Pin, INPUT);
    pinMode(Gps_Reset_Pin, INPUT);
    pinMode(Gps_pps_Pin, INPUT);

    // I2C pins — not used, tri-state
    pinMode(SDA_Pin, INPUT);
    pinMode(SCL_Pin, INPUT);

    // LEDs (active LOW — write HIGH to turn off)
    pinMode(RedLed_Pin, OUTPUT);
    digitalWrite(RedLed_Pin, HIGH);
    pinMode(GreenLed_Pin, OUTPUT);
    digitalWrite(GreenLed_Pin, HIGH);
    pinMode(BlueLed_Pin, OUTPUT);
    digitalWrite(BlueLed_Pin, HIGH);
}

void enterDeepSleep() {
    // 1. Turn off backlight
    pinMode(ePaper_Backlight, OUTPUT);
    digitalWrite(ePaper_Backlight, LOW);

    // 2. Cut power rail
    pinMode(Power_Enable_Pin, OUTPUT);
    digitalWrite(Power_Enable_Pin, LOW);
    pinMode(Power_Enable1_Pin, OUTPUT);
    digitalWrite(Power_Enable1_Pin, LOW);

    // 3. Tri-state ALL SPI, UART, I2C pins to prevent parasitic current
    pinMode(ePaper_Miso, INPUT);
    pinMode(ePaper_Mosi, INPUT);
    pinMode(ePaper_Sclk, INPUT);
    pinMode(ePaper_Cs, INPUT);
    pinMode(ePaper_Dc, INPUT);
    pinMode(ePaper_Rst, INPUT);
    pinMode(ePaper_Busy, INPUT);

    pinMode(Gps_Tx_Pin, INPUT);
    pinMode(Gps_Rx_Pin, INPUT);
    pinMode(SDA_Pin, INPUT);
    pinMode(SCL_Pin, INPUT);

    // 4. Configure UserButton_Pin as SENSE_LOW wake source
    nrf_gpio_cfg_sense_input(g_ADigitalPinMap[UserButton_Pin], NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);

    // 5. Configure Touch_Pin as SENSE_HIGH wake source
    nrf_gpio_cfg_sense_input(g_ADigitalPinMap[Touch_Pin], NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);

    // 6. Enter System OFF (~0.4µA)
    NRF_POWER->SYSTEMOFF = 1;
    while(1);
}

float readBatteryVoltage() {
    analogReadResolution(12);
    int raw = analogRead(Adc_Pin);
    return (raw / 4096.0f) * 3.6f * 2.0f;
}

uint8_t batteryPercent() {
    float v = readBatteryVoltage();
    if (v >= BATTERY_FULL_V) return 100;
    if (v <= BATTERY_EMPTY_V) return 0;
    return (uint8_t)((v - BATTERY_EMPTY_V) / (BATTERY_FULL_V - BATTERY_EMPTY_V) * 100.0f);
}
