/*
 * Project: Smart Glasses ESP32-CAM (with microSD Support)
 * Target: AI Thinker ESP32-CAM Module
 * Description: Captures JPEG frames, saves them to SD card, and sends them to a smartphone via HTTP POST.
 */

#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "FS.h"                // SD Card ESP32 File System
#include "SD_MMC.h"            // SD Card ESP32-CAM Library
#include "soc/soc.h"           // Brownout problems
#include "soc/rtc_cntl_reg.h"  // Brownout problems

// ===========================
// CONFIGURATION
// ===========================
const char* ssid = "YOUR_PHONE_HOTSPOT_NAME";
const char* password = "YOUR_HOTSPOT_PASSWORD";
const char* serverUrl = "http://192.168.x.x:5000/upload"; // Target Phone Server URL

// Transmission Interval (ms)
const int captureInterval = 3000; 

// AI Thinker Camera Pin Mapping
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

int pictureCount = 0;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // 1. Camera Configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 

  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA; 
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // 2. Initialize Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // 3. Initialize SD Card
  Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    // We continue anyway, so the project still works via WiFi
  } else {
    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE){
      Serial.println("No SD Card attached");
    }
  }

  // 4. Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    delay(500);
    Serial.print(".");
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
  } else {
    Serial.println("\nWiFi Failed (Will retry in loop)");
  }
}

void loop() {
  captureProcess();
  delay(captureInterval);
}

void captureProcess() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // --- STEP 1: Save to SD Card ---
  String path = "/pic_" + String(pictureCount) + ".jpg";
  fs::FS &fs = SD_MMC;
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } else {
    file.write(fb->buf, fb->len); 
    Serial.printf("Saved file to path: %s\n", path.c_str());
    pictureCount++;
  }
  file.close();

  // --- STEP 2: Send to Smartphone Server ---
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    if (http.begin(serverUrl)) {
      http.addHeader("Content-Type", "image/jpeg");
      int httpResponseCode = http.POST(fb->buf, fb->len);
      if (httpResponseCode > 0) {
        Serial.printf("HTTP Sent! Response: %d\n", httpResponseCode);
      } else {
        Serial.printf("HTTP POST Error: %s\n", http.errorToString(httpResponseCode).c_str());
      }
      http.end();
    }
  } else {
    Serial.println("WiFi not connected. Skipping upload...");
    // Try to reconnect in background
    if (WiFi.status() != WL_CONNECTED) WiFi.begin(ssid, password);
  }

  esp_camera_fb_return(fb); 
}
