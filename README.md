# ESP-GCS

## Building the Project

### 0. Config

Rename esp_gcs_config.h to my_esp_gcs_config.h and update values

### 1. Fetch Git Submodules

Before building, make sure to fetch all git submodules:

```sh
git submodule update --init --recursive
```

### 2. Install PlatformIO

If you don't have PlatformIO installed, install it using pip:

```sh
pip install platformio
```

### 3. Build the Project

To build the project, run:

```sh
platformio run
```

### 4. Upload to Device (Optional)

To upload the firmware to your device:

```sh
platformio run --target upload
```

Make sure your board is connected and the correct environment is set in `platformio.ini`.