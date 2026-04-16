# Smart Glasses Project (ESP32-CAM)

This folder contains the complete firmware and server code for a smart glasses prototype using the ESP32-CAM AI Thinker module.

## 📦 Folder Contents
1. **`esp32_cam_glasses.ino`**: The main Arduino firmware.
2. **`phone_server_mock.py`**: A Python Flask server to receive images on a smartphone or PC.
3. **`README.md`**: This instruction guide.

---

## 🚀 Getting Started

### 1. Flash the ESP32-CAM
- Open `esp32_cam_glasses.ino` in the Arduino IDE.
- **Board Settings:**
  - Board: `AI Thinker ESP32-CAM`
  - CPU Frequency: `240MHz`
  - Flash Mode: `QIO`
  - Flash Frequency: `40MHz`
  - PSRAM: `Enabled` (Crucial for image quality)
- **Code Changes:** Update the `ssid`, `password`, and `serverUrl` variables at the top of the file to match your phone's hotspot.

### 2. Prepare the Hardware
- **MicroSD Card:** Insert a formatted FAT32 microSD card (up to 32GB). The device will automatically save a backup of every photo taken.
- **Power:** Connect to a stable 5V power source.

### 3. Run the Phone Server
- Install Python on your phone (using an app like Pydroid 3) or on a PC connected to the same hotspot.
- Run `python phone_server_mock.py`.
- It will listen for images and save them in a folder called `received_images`.

---

## 🛠️ Memory & Stability Optimization
- **SD Card Buffering:** Images are saved to the SD card first. If WiFi is disconnected, the image remains on the SD card so no data is lost.
- **Brownout Protection:** The code includes a software fix to prevent the module from resetting during high-power WiFi bursts.
- **PSRAM Handling:** The code dynamically checks for PSRAM. If found, it uses "Double Buffering" to capture the next image while the current one is still being uploaded.

## ⚠️ Troubleshooting
- **No SD Card Found:** Ensure the card is pushed in fully and formatted to FAT32.
- **WiFi Fail:** If the glasses can't connect, check if your phone hotspot is set to "2.4 GHz" (ESP32 does not support 5GHz).
- **Image Quality:** To increase quality, lower the `jpeg_quality` number (e.g., from 12 to 5) in the code.
