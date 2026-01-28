#include "camera.h"
#include "esp_camera.h"

#include "pins.h"

// Must be cpp because of conflict on sensor_t!

void camera::begin(Scheduler&)
{
  // Camera pin mapping per Seeed XIAO ESP32S3 Sense docs :contentReference[oaicite:4]{index=4}
  camera_config_t cfg = {};
  cfg.ledc_channel = LEDC_CHANNEL_0;
  cfg.ledc_timer   = LEDC_TIMER_0;

  cfg.pin_d0       = pins::CamY2.pin;
  cfg.pin_d1       = pins::CamY3.pin;
  cfg.pin_d2       = pins::CamY4.pin;
  cfg.pin_d3       = pins::CamY5.pin;
  cfg.pin_d4       = pins::CamY6.pin;
  cfg.pin_d5       = pins::CamY7.pin;
  cfg.pin_d6       = pins::CamY8.pin;
  cfg.pin_d7       = pins::CamY9.pin;

  cfg.pin_xclk     = pins::CamXclk.pin;
  cfg.pin_pclk     = pins::CamPclk.pin;
  cfg.pin_vsync    = pins::CamVsync.pin;
  cfg.pin_href     = pins::CamHref.pin;

  cfg.pin_sccb_sda = pins::CamSda.pin;
  cfg.pin_sccb_scl = pins::CamScl.pin;

  // Typically not used / not connected on this module
  cfg.pin_pwdn     = -1;
  cfg.pin_reset    = -1;

  cfg.xclk_freq_hz = 20000000;

  // Setup-only defaults (tune later in “business logic”)
  cfg.pixel_format = PIXFORMAT_JPEG;
  cfg.frame_size   = FRAMESIZE_QVGA;
  cfg.jpeg_quality = 12;
  cfg.fb_count     = 1;

  const esp_err_t err = esp_camera_init(&cfg);
  if (err != ESP_OK)
  {
    Serial.printf("[camera] esp_camera_init failed: 0x%x\n", static_cast<unsigned>(err));
    return;
  }
}
