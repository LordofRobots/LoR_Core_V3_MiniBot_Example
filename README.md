# LoR Core V3 – MiniBot Example
Sample Arduino sketch for the **LoR Core V3** using a Bluetooth gamepad via **Bluepad32**, **ESP32Servo**, and **FastLED**.

- Main sketch: `LoR_Core_V3_MiniBot_Example.ino`
- Target board: **esp32_bluepad32 / ESP32 Dev Module**
- Serial baud: **115200**
- License: **Apache-2.0** (see `LICENSE`)

---

## Requirements
- **Arduino IDE 2.x**
- **LoR Core V3** controller + USB cable
- **Bluetooth gamepad**
- (If needed) **CH340** USB driver

---

## Download the code (ZIP)
1. On the repo page, click **Code ▸ Download ZIP**.  
2. Unzip to a folder.  
3. Open `LoR_Core_V3_MiniBot_Example.ino` in Arduino IDE.  
> Keep the folder name the same as the `.ino` file.

---

## Read the setup notes in the sketch
Open the `.ino` and read the header block at the top. The steps below mirror those instructions.

---

## Environment setup (matches the sketch header)

### 1) Add Boards Manager URLs
**File ▸ Preferences ▸ Additional Boards Manager URLs** → add each on its own line:
```

[https://dl.espressif.com/dl/package\_esp32\_index.json](https://dl.espressif.com/dl/package_esp32_index.json)
[https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32\_files/package\_esp32\_bluepad32\_index.json](https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json)

```

### 2) Install board packages (Boards Manager)
**Tools ▸ Board ▸ Boards Manager…**
- Install **“esp32” by Espressif Systems**
- Install **“esp32_bluepad32” by Ricardo Quesada**

### 3) Install libraries (Library Manager)
**Sketch ▸ Include Library ▸ Manage Libraries…**
- **FastLED** by Daniel Garcia — **Latest**
- **ESP32Servo** by Kevin Harrington, John K. Bennett — **Version 3.0.7**  
  *(Note: 3.0.8 does not work due to ESP32 LEDC handling changes.)*

### 4) (If needed) Install CH340 USB driver
If the board/port doesn’t appear under **Tools ▸ Port**, install:
- CH340: `https://www.wch-ic.com/download/file?id=65`  
Reconnect USB and restart IDE.

### 5) Select the **correct** target board
**Tools ▸ Board** → **USE**: `esp32_bluepad32 / ESP32 Dev Module`  
**DO NOT USE**: `esp32 / ESP32 Dev Module`

### 6) Select COM port and upload
**Tools ▸ Port** → choose the LoR Core V3 port.  
Click **Upload**. The process is automatic (BOOT not required).  
If it fails: hold **BOOT**, tap **RST**, release **BOOT** at “Connecting…”.

### 7) Pair the gamepad (first time only)
1. Power **LoR Core V3**.  
2. **Hold User Buttons A + D**, then press/release **RST**.  
3. When LEDs **flash BLUE + WHITE**, release A + D (pair mode).  
4. Put the **gamepad** into Bluetooth pairing mode.  
5. Wait for **GREEN flash** → pairing done, returns to normal mode.  
Notes: Auto-reconnect on future power-ups; BT keys are remembered across power cycles and uploads.

### 8) LED status reference
- **ICE BLUE** — Waiting / disconnected  
- **RED** — Error / disconnect event  
- **GREEN** — System OK / acknowledge  
- **RAINBOW** — Receiving gamepad data  
- **FLASH BLUE + WHITE** — Pair mode

---

## Build, upload, and run
1. Confirm **Board** = `esp32_bluepad32 / ESP32 Dev Module` and correct **Port**.  
2. **Verify** (✔) then **Upload** (→).  
3. Open **Serial Monitor** at **115200**.  
4. If not yet paired, follow **Step 7**.

---

## What the sketch does
- Initializes watchdog, LEDs (FastLED), servos (ESP32Servo), and Bluepad32.
- Drives **left** (slots 1–6) and **right** (slots 7–12) motor groups from gamepad sticks.
  - Deadband ±50 around center; map to **0–180** with **90 = stop/center**.
- **User_SW** toggles drive inversion.
- Battery monitor on **VIN_SENSE (GPIO 34)** logs VIN and scales output on low voltage.
- LED colors reflect state (see above).

---

## Troubleshooting
- **No COM Port** → different cable/USB port; install **CH340**; restart IDE.  
- **Failed to connect** → manual boot: hold **BOOT**, tap **RST**, release **BOOT** at “Connecting…”.  
- **Compile errors / missing headers** → re-check libraries (use **ESP32Servo 3.0.7**).  
- **Garbled Serial** → set **115200** baud.

---

## Repo structure
```

LoR\_Core\_V3\_MiniBot\_Example/
├─ LoR\_Core\_V3\_MiniBot\_Example.ino
├─ LICENSE            # Apache-2.0
└─ README.md

```

---

## License
**Apache-2.0** — see `LICENSE`.

Copyright 2025 Lord of Robots Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this project except in compliance with the License.
You may obtain a copy of the License at:

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
