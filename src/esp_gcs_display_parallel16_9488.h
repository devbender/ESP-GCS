#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#define LCD_CS 37
#define LCD_BLK 45

class LGFX_Parallel_9488 : public lgfx::LGFX_Device
{
    // lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Panel_ILI9488 _panel_instance;
    lgfx::Bus_Parallel16 _bus_instance; // Instance of an 8-bit parallel bus (ESP32 only)

public:
    // Create a constructor and set various configurations here.
    // If you change the class name, specify the same name for the constructor.
    LGFX_Parallel_9488(void)
    {
        {                                      // Configure bus control.
            auto cfg = _bus_instance.config(); // Retrieve the structure for bus settings.

            // 16-bit settings
            cfg.port = 0;              // Select the I2S port to use (0 or 1) (Uses the ESP32's I2S LCD mode)
            cfg.freq_write = 20000000; // Transmission clock (maximum 20MHz, rounded to an integer value of 80MHz divided)
            cfg.pin_wr = 35;           // Pin number connected to WR
            cfg.pin_rd = 48;           // Pin number connected to RD
            cfg.pin_rs = 36;           // Pin number connected to RS (D/C)

            cfg.pin_d0 = 47;
            cfg.pin_d1 = 21;
            cfg.pin_d2 = 14;
            cfg.pin_d3 = 13;
            cfg.pin_d4 = 12;
            cfg.pin_d5 = 11;
            cfg.pin_d6 = 10;
            cfg.pin_d7 = 9;
            cfg.pin_d8 = 3;
            cfg.pin_d9 = 8;
            cfg.pin_d10 = 16;
            cfg.pin_d11 = 15;
            cfg.pin_d12 = 7;
            cfg.pin_d13 = 6;
            cfg.pin_d14 = 5;
            cfg.pin_d15 = 4;

            _bus_instance.config(cfg);              // Apply the configuration values to the bus.
            _panel_instance.setBus(&_bus_instance); // Set the bus to the panel.
        }

        {                                        // Configure display panel control.
            auto cfg = _panel_instance.config(); // Retrieve the structure for display panel settings.

            cfg.pin_cs = -1;   // CS is pulled low
            cfg.pin_rst = -1;  // RST is connected to the board's RST
            cfg.pin_busy = -1; // Pin number connected to BUSY (-1 = disable)

            // The following settings have common default values for each panel, so try commenting them out if you're unsure.

            cfg.memory_width = 320;   // Maximum width supported by the driver IC
            cfg.memory_height = 480;  // Maximum height supported by the driver IC
            cfg.panel_width = 320;    // Actual displayable width
            cfg.panel_height = 480;   // Actual displayable height
            cfg.offset_x = 0;         // X-direction offset of the panel
            cfg.offset_y = 0;         // Y-direction offset of the panel
            cfg.offset_rotation = 0;  // Offset value for rotation direction 0~7 (4~7 are upside down)
            cfg.dummy_read_pixel = 8; // Number of dummy reads before reading pixels
            cfg.dummy_read_bits = 1;  // Number of dummy reads before reading data other than pixels
            cfg.readable = true;      // Set to true if data reading is possible
            cfg.invert = false;       // Set to true if the panel's brightness is inverted
            cfg.rgb_order = false;    // Set to true if red and blue on the panel are swapped
            cfg.dlen_16bit = true;    // Set to true for panels that transmit data in 16-bit units
            cfg.bus_shared = true;    // Set to true if sharing the bus with the SD card (bus control is performed in functions like drawJpgFile)

            _panel_instance.config(cfg);
        }

        setPanel(&_panel_instance); // Set the panel to be used.
    }
};