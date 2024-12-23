#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <driver/spi_common.h>

class LGFX_M5Core : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9342 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;

public:
    LGFX_M5Core(void) {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST;  // HSPI for M5Core1
            cfg.freq_write = 40000000; // SPI write speed
            cfg.pin_sclk = 18;        // Clock pin
            cfg.pin_mosi = 23;        // MOSI pin
            cfg.pin_miso = 19;        // MISO pin (not used for the display)
            cfg.pin_dc = 27;          // Data/Command pin
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = 14;          // Chip Select pin
            cfg.pin_rst = 33;         // Reset pin
            cfg.memory_width = 320;   // Framebuffer width
            cfg.memory_height = 240;  // Framebuffer height
            cfg.panel_width = 320;    // Display width
            cfg.panel_height = 240;   // Display height
            _panel_instance.config(cfg);
        }

        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = 32;          // Backlight pin
            cfg.freq = 12000;         // PWM frequency
            cfg.pwm_channel = 7;      // PWM channel
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        setPanel(&_panel_instance);
    }
};
