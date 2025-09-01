// Lord of Robots - LoR Core V3 - AUG 3 2025
// Sample MiniBot Control Program with Bluetooth GamePad Interface

///////////////////////////////////////////////////////////////////////////////////////////
//            --- Environment Setup Prerequisites and Important Notes: ---               //
///////////////////////////////////////////////////////////////////////////////////////////
// 1. Add the following custom ULRs to the arduino IDE - File / Preferences / Additional Boards Manager URLs
//  - https://dl.espressif.com/dl/package_esp32_index.json
//  - https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json
// 2. Install the following boards in Board Manager:
//  - "esp32" by Espressif Systems
//  - "esp32_bluepad32" by Ricardo Quesada
// 3. Install the following libraries using the Library Manager:
//  - "FastLED" (Latest version) by Danial Garcia
//  - "ESP32Servo"(VERSION 3.0.7) by Kevin Harrington, John K. Bennett (Version 3.0.8 does not work due to standard ESP32 board update, ledc handling)
// 4. If needed, Install USB Driver: CH340. Check Device manager for connection in COM ports
//  - https://www.wch-ic.com/download/file?id=65
// 5. Select Target Board:
//  - USE "esp32_bluepad32 / ESP32 Dev Module"
//  - DO NOT use "esp32 / ESP32 Dev Module"
// 6. Select COM Port and Upload
//  - Process is automatic, Boot button is not reqired
//  - If auto load fail, Manually enter Boot mode: Press and hold Boot Button, press and release the RST button, then release Boot Button
// 7. Pair GamePad with LoR Core V3 (FIRST TIME ONLY)
//  - Turn on LoR Core V3
//  - Press and Hold User Button A + D. Press and release RST Button.
//  - When LEDs Flash BLUE and WHITE, Release User Button A + D.
//  - Set GamePad to Bluetooth Pair Mode.
//  - Wait for Pairing to Complete. Indicated with GREEN Flash and automatically returns to Normal mode.
//  - Will automatically connect on next power up or LoR Core V3 and GamePad.
//  - will remeber bluetooth keys between power cycles and uploads.
// 8. LoR Core V3 LED States
//  - ICE BLUE = Waiting for Bluetooth connection to GamePad / Disconnected / Standing by
//  - RED = Error / Disconnection event
//  - GREEN = System ok / Acknowledge
//  - RAINBOW = Recieving GamePad data
//  - Flash BLUE and WHITE = Pair mode

////////////////////////////////////////////////////////////////////////////////////////////
//                            Libraries                                                   //
////////////////////////////////////////////////////////////////////////////////////////////
#include <Bluepad32.h>     // Gamepad Core
#include "esp_task_wdt.h"  // System stability
#include <ESP32Servo.h>    // Servo PWM Core
#include <FastLED.h>       // Addressable LED Core

////////////////////////////////////////////////////////////////////////////////////////////
//                            AUX Port Config                                             //
////////////////////////////////////////////////////////////////////////////////////////////
// --- AUX_Port Pins ---
const uint8_t AUX_PINS[9] = { 0, 5, 18, 23, 19, 22, 21, 1, 3 };  // AUX_PIN[slot_number]note: slot 0 is imaginary

////////////////////////////////////////////////////////////////////////////////////////////
//                            IO Port Config                                              //
////////////////////////////////////////////////////////////////////////////////////////////
// --- IO_Port Pins ---
const uint8_t IO_PINS[13] = { 0, 32, 25, 26, 27, 14, 12, 13, 15, 2, 4, 22, 21 };  // IO_PIN[slot_number]   note: slot 0 is imaginary

////////////////////////////////////////////////////////////////////////////////////////////
//                         User Button and Switch Config                                  //
////////////////////////////////////////////////////////////////////////////////////////////
// --- User Inputs ---
#define User_BTN_A 35
#define User_BTN_B 39
#define User_BTN_C 38
#define User_BTN_D 37
#define User_SW 36

////////////////////////////////////////////////////////////////////////////////////////////
//                      Input Voltage Monitor Config                                      //
////////////////////////////////////////////////////////////////////////////////////////////
// --- Input Voltage Sensor Input ---
#define VIN_SENSE 34

// --- Voltage scaling (6V = 775, 12V = 1720) ---
#define VOLT_SLOPE 0.0063492
#define VOLT_OFFSET 1.079

////////////////////////////////////////////////////////////////////////////////////////////
//                            LED Config                                                  //
////////////////////////////////////////////////////////////////////////////////////////////
// --- Addressable LED Data Output ---
#define LED_PIN 33

#define LED_COUNT 4
#define BRIGHTNESS 255
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
CRGB leds[LED_COUNT];
uint8_t rainbowHue = 0;

////////////////////////////////////////////////////////////////////////////////////////////
//                            Initalize Interal features                                  //
////////////////////////////////////////////////////////////////////////////////////////////
#define WDT_TIMEOUT 3   // Watchdog Timeout: 3 seconds
Servo MotorOutput[13];  // example: servos[slot_number].write(90);

void INIT_InternalFeatures() {
  // Serial communication
  Serial.begin(115200);

  // Setup the watchdog timer
  esp_task_wdt_init(WDT_TIMEOUT, true);  // Enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);

  // --- Inputs ---
  pinMode(User_BTN_A, INPUT);
  pinMode(User_BTN_B, INPUT);
  pinMode(User_BTN_C, INPUT);
  pinMode(User_BTN_D, INPUT);
  pinMode(User_SW, INPUT);
  pinMode(VIN_SENSE, INPUT);

  // --- FastLED Setup ---
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, LED_COUNT);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
  delay(100);
}




////////////////////////////////////////////////////////////////////////////////////////////
//                            bluepad 32 Config                                           //
////////////////////////////////////////////////////////////////////////////////////////////
ControllerPtr myController = nullptr;  // Define a single controller pointer

// --- Gamepad Connected ---
void onConnectedController(ControllerPtr ctl) {
  if (myController == nullptr) {  // check if not already connected to a GamePad
    Serial.println("! GamePad connected !");
    myController = ctl;  //establish pointer

    ctl->playDualRumble(0x00, 0xc0, 0xc0, 0xc0);  // give a lil shake
    ctl->setColorLED(0, 255, 0);                  //set GamePad LED colour

    BP32.enableNewBluetoothConnections(false);  // Disable Pairing

    fill_solid(leds, LED_COUNT, CRGB(0, 255, 0));  // Green LED Status
    FastLED.show();
    delay(500);

  } else {
    Serial.println("Another controller tried to connect but is rejected");
  }
}

// --- Gamepad Disconnected ---
void onDisconnectedController(ControllerPtr ctl) {
  if (myController == ctl) {
    Serial.println("! GamePad disconnected !");
    myController = nullptr;  // Reset the controller pointer when disconnected

    fill_solid(leds, LED_COUNT, CRGB(255, 0, 0));  // RED LED Status
    FastLED.show();
    delay(1000);
  }
}

// --- Gamepad battery monitor ---
static unsigned long lastBatteryUpdate = 0;
void GamePad_BatteryMonitor() {
  if (millis() - lastBatteryUpdate > 1000) {
    int battery = myController->battery();
    if (battery == 0) {
      myController->setColorLED(255, 0, 0);  // Unknown Status

    } else if (battery <= 64) {              // ~25% of 255
      myController->setColorLED(255, 0, 0);  // set Gamepad LED Red
      Serial.println("! GamePad Low Battery !");

      myController->playDualRumble(0x00, 0xc0, 0xc0, 0xc0);  //give a lil shake

      fill_solid(leds, LED_COUNT, CRGB(255, 0, 0));  // RED LED Status
      FastLED.show();
      delay(100);

    } else if (battery <= 128) {               // ~50%
      myController->setColorLED(255, 255, 0);  // Yellow

    } else {
      myController->setColorLED(0, 255, 0);  // Green
    }
    lastBatteryUpdate = millis();
  }
}

// --- Initalize and Pair mode --- //
void INIT_BluetoothGamepad_PairMode() {  // Setup the Bluepad32 callbacks

  //Pair Mode Handling
  if (!digitalRead(User_BTN_A) && !digitalRead(User_BTN_D)) {
    BP32.forgetBluetoothKeys();  // Forgetting Bluetooth keys resets to "factory" and prevents "paired" gamepads to reconnect automatically.
    Serial.println("Gamepad Unpaird!");
    BP32.enableNewBluetoothConnections(true);                       // Allow Game pads to pair
    BP32.setup(&onConnectedController, &onDisconnectedController);  // start bluetooth Gamepage functions

    while (!(myController && myController->isConnected())) {  // WAIT for new GamePad to Pair
      esp_task_wdt_reset();                                   // Feed the watchdog

      fill_solid(leds, LED_COUNT, CRGB(0, 0, 255));  // BLUE LED Status
      FastLED.show();
      delay(100);
      fill_solid(leds, LED_COUNT, CRGB(255, 255, 255));  // WHITE LED Status
      FastLED.show();
      delay(100);

      BP32.update();
    }
    BP32.enableNewBluetoothConnections(false);                           // Stop Gamepads from pairing
  } else BP32.setup(&onConnectedController, &onDisconnectedController);  // start bluetooth Gamepage functions

  BP32.enableVirtualDevice(false);  // Stop Virtual devices from pairing
}


////////////////////////////////////////////////////////////////////////////////////////////
//                            Input voltage / Battery monitor                             //
////////////////////////////////////////////////////////////////////////////////////////////
float Low_Batt_Scaler = 0.25;
unsigned long TriggerTime = 0;
bool Scaler_StepState = 0;
unsigned long Check_Period_TriggerTime = 0;

float LoRcore_BatteryMonitor(uint8_t cellCount, float perCellLowV = 3.0, bool DEBUG = true) {

  int vin_raw = analogRead(VIN_SENSE);
  float vin_voltage = (vin_raw * VOLT_SLOPE) + VOLT_OFFSET;
  float lowVoltageThreshold = cellCount * perCellLowV;

  if (millis() > Check_Period_TriggerTime && DEBUG) {
    Check_Period_TriggerTime = millis() + 500;
    Serial.printf("VIN: %.2f V, Threshold: %.2f V\n", vin_voltage, lowVoltageThreshold);
  }

  if (vin_voltage < lowVoltageThreshold) {
    Serial.printf("LOW Battery: %.2f V\n", vin_voltage);

    if (millis() > TriggerTime) {

      if (Scaler_StepState) {
        Low_Batt_Scaler = 0;
      } else {
        Low_Batt_Scaler = 0.25;
        fill_solid(leds, LED_COUNT, CRGB(255, 0, 0));
        FastLED.show();
        delay(100);
      }

      Scaler_StepState = !Scaler_StepState;
      TriggerTime = millis() + 100;
    }
  } else {
    Low_Batt_Scaler = 1.0;
  }
  return vin_voltage;
}

////////////////////////////////////////////////////////////////////////////////////////////
//                            Power up Diagnostics                                        //
////////////////////////////////////////////////////////////////////////////////////////////
void Powerup_Diagnostics_LED() {  // System check on boot cause and report

  Serial.print("BOOT Condition: ");
  if (esp_reset_reason() == ESP_RST_TASK_WDT) {
    Serial.println("Watchdog Reset Detected");
    fill_solid(leds, LED_COUNT, CRGB(255, 255, 255));  //Watch Dog = WHITE
  } else if (esp_reset_reason() == ESP_RST_BROWNOUT) {
    Serial.println("Brownout Reset Detected");
    fill_solid(leds, LED_COUNT, CRGB(255, 255, 0));  //Brown out = YELLOW
    delay(3000);
  } else if (esp_reset_reason() == ESP_RST_POWERON) {
    Serial.println("Power-on Reset Detected");
    fill_solid(leds, LED_COUNT, CRGB(0, 255, 0));  // Normal Start = GREEN
  } else if (esp_reset_reason() == ESP_RST_SW) {
    Serial.println("Software Reset Detected");
    fill_solid(leds, LED_COUNT, CRGB(0, 0, 255));  // Button / Program Reset = BLUE
  } else if (esp_reset_reason() == ESP_RST_PANIC) {
    Serial.println("Panic Reset Detected");
    fill_solid(leds, LED_COUNT, CRGB(255, 0, 0));  // PANIC = RED
  } else {
    Serial.println("UNKOWN Reset Detected");
    fill_solid(leds, LED_COUNT, CRGB(255, 0, 255));  // UNKOWN = PURPLE;
  }

  FastLED.show();
  delay(500);
  fill_solid(leds, LED_COUNT, CRGB(0, 0, 0));  // LED OFF;
  FastLED.show();
  delay(100);
}

////////////////////////////////////////////////////////////////////////////////////////////
//                               Servo/ Motor Config                                      //
////////////////////////////////////////////////////////////////////////////////////////////

//Names of Predefined Motor types
enum MotorType {
  MG90_CR,
  MG90_Degree,
  N20Plus,
  STD_SERVO,
  Victor_SPX,
  Talon_SRX,
  CUSTOM
};

struct MotorTypeConfig {  // define the config structure
  MotorType type;
  float pwmFreq;
  int minPulseUs;
  int maxPulseUs;
  float inputMin;
  float inputMax;

};

MotorTypeConfig motorTypeConfigs[] = {
  // define specific motor config parameters {NAME, FREQ, MINms, MAXms}
  { MG90_CR, 50, 500, 2500, -1, 1},
  { MG90_Degree, 50, 500, 2500, 1, 180},
  { N20Plus, 50, 1000, 2000, -1, 1 },
  { Victor_SPX, 50, 1000, 2000, -1, 1 },
  { Talon_SRX, 50, 1000, 2000, -1, 1 },
  { STD_SERVO, 50, 1000, 2000 },
  // add more types as needed
};

//--- Configure motors function --- //
void ConfigureMotorOutput(uint8_t slot, MotorType motorType, int startupPositionDeg = 90) {
  // Lookup the config
  float pwmFreq = 50;
  int minPulseUs = 1000;
  int maxPulseUs = 2000;

  for (auto &cfg : motorTypeConfigs) {
    if (cfg.type == motorType) {
      pwmFreq = cfg.pwmFreq;
      minPulseUs = cfg.minPulseUs;
      maxPulseUs = cfg.maxPulseUs;
      break;
    }
  }

  uint8_t pin = IO_PINS[slot];
  pinMode(pin, OUTPUT);

  MotorOutput[slot].setPeriodHertz(pwmFreq);
  MotorOutput[slot].attach(pin, minPulseUs, maxPulseUs);
  MotorOutput[slot].write(startupPositionDeg);

  Serial.printf(
    "Motor slot %d configured on pin %d as type %d: freq=%.1f Hz, pulse=%d-%d us, start=%d deg\n",
    slot, pin, motorType, pwmFreq, minPulseUs, maxPulseUs, startupPositionDeg);
}


////////////////////////////////////////////////////////////////////////////////////////////
//                            Initialize LoR Core                                         //
////////////////////////////////////////////////////////////////////////////////////////////

// Initalizes core features of the LoRcore
void INIT_LoRcore() {
  INIT_InternalFeatures();
  Powerup_Diagnostics_LED();
  INIT_BluetoothGamepad_PairMode();
}



////////////////////////////////////////////////////////////////////////////////////////////
//                            Setup LOOP                                                  //
////////////////////////////////////////////////////////////////////////////////////////////
// Set up pins, LED PWM functionalities and begin GamePad, Serial and Serial2 communication


void setup() {
  INIT_LoRcore();

  // --- IO outputs ---
  Serial.println("Motors Startup");

  ConfigureMotorOutput(1, N20Plus, 90);
  ConfigureMotorOutput(2, N20Plus, 90);
  ConfigureMotorOutput(3, N20Plus, 90);
  ConfigureMotorOutput(4, N20Plus, 90);
  ConfigureMotorOutput(5, N20Plus, 90);
  ConfigureMotorOutput(6, N20Plus, 90);
  ConfigureMotorOutput(7, N20Plus, 90);
  ConfigureMotorOutput(8, N20Plus, 90);
  ConfigureMotorOutput(9, N20Plus, 90);
  ConfigureMotorOutput(10, N20Plus, 90);
  ConfigureMotorOutput(11, N20Plus, 90);
  ConfigureMotorOutput(12, N20Plus, 90);

  // --- System Start Complete ---
  Serial.println("LoRcore V3 System Ready! ");
}

////////////////////////////////////////////////////////////////////////////////////////////
//                            Main LOOP                                                   //
////////////////////////////////////////////////////////////////////////////////////////////

void loop() {

  // --- Watch Dog ---
  esp_task_wdt_reset();  // Feed the watchdog

  // --- Check and Collect Gamepad Data ---
  BP32.update();

  // --- Check LoRcore Battery status ---
  LoRcore_BatteryMonitor(2, 3.0);

  // --- Gamepad Connected ---
  if (myController && myController->isConnected()) {

    // --- Gamepad Battery Monitor ---
    GamePad_BatteryMonitor();

    // --- GamePad Input update ---
    //checks user switch to invert directions or not.
    int currentLeft = (digitalRead(User_SW) == HIGH) ? myController->axisRY() * Low_Batt_Scaler : -myController->axisRY() * Low_Batt_Scaler;  
    int currentRight = (digitalRead(User_SW) == HIGH) ? -myController->axisY() * Low_Batt_Scaler : myController->axisY() * Low_Batt_Scaler;   // INVERTED 

    // handles GamePad stick drift
    if (-50 < currentLeft && currentLeft < 50) currentLeft = 0;  // Deadband handling
    if (-50 < currentRight && currentRight < 50) currentRight = 0;

    // converts from Gamepad joysitck values to motor values
    int MappedLeft = map(currentLeft, -512, 512, 0, 180);  // Map Gamepad data to Servo range
    int MappedRight = map(currentRight, -512, 512, 0, 180);

    MappedLeft = constrain(MappedLeft, 0, 180);
    MappedRight = constrain(MappedRight, 0, 180);

    // --- Servo motion update ---
    // example: MotorOutput[1].write(value); value = a servo position or motor speed; 0-180 where 90 is center position or stop
    MotorOutput[1].write(MappedLeft);
    MotorOutput[2].write(MappedLeft);
    MotorOutput[3].write(MappedLeft);
    MotorOutput[4].write(MappedLeft);
    MotorOutput[5].write(MappedLeft);
    MotorOutput[6].write(MappedLeft);
    MotorOutput[7].write(MappedRight);
    MotorOutput[8].write(MappedRight);
    MotorOutput[9].write(MappedRight);
    MotorOutput[10].write(MappedRight);
    MotorOutput[11].write(MappedRight);
    MotorOutput[12].write(MappedRight);

    // --- Rainbow LED animation  ---
    fill_rainbow(leds, LED_COUNT, rainbowHue++, 20);
    FastLED.show();

    // --- System stability delay ~20 hz (50 loops per second) ---
    delay(50);
  }

  // --- Gamepad Disconnected ---
  else {
    for (int i = 1; i <= 12; i++) {
      MotorOutput[i].write(90);  //stop motors
    }
    fill_solid(leds, LED_COUNT, CRGB(0, 80, 255));  // ICY BLUE Led = Waiting for gamepad bluetooth connection
    FastLED.show();
  }
}