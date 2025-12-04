#define BLYNK_TEMPLATE_ID "TMPL4vC03FgXG"
#define BLYNK_TEMPLATE_NAME "IndustrialProject"
#define BLYNK_AUTH_TOKEN "I_MHZ2hUc8477q_VxdLw7oxEm8obORIE"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_SHT31.h>
#include <ESP32Servo.h>
#include "esp_task_wdt.h"
#include "esp_system.h"

// ====== WiFi and Blynk credentials ======
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "";     // Enter your WIFI name
char pass[] = "";     // Enter your WIFI password

// ====== Hardware ======
#define LDR_PIN 32
#define SERVO_PIN 25
#define MOISTURE_SENSOR 34
#define RELAY_PIN 14

// SHT31 is connected to pins 21 (SDA) and 22 (SCL)
// No need to define pins in the code for this sensor only
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Servo servo;

// ====== Flags ======
bool manual_auto_switch = false; // Flag for manual (true) vs automatic (false) control
bool manualPumpControl = false;  // Flag for manual on vs off

// ====== Virtual Pins in Blynk app ======
#define VPIN_LDR   V0
#define VPIN_HUM   V1
#define VPIN_TEMP  V2
#define VPIN_SERVO V3
#define VPIN_MOISTURE V4
#define VPIN_PUMP_MANUAL_AUTO_SWITCH V5
#define VPIN_PUMP_MANUAL_CONTROL V6


BLYNK_WRITE(VPIN_SERVO) {
  int value = param.asInt();
  // value is one or zero, as it is a digital ouput from blynk
  servo.write(value * 180);
}

BLYNK_WRITE(VPIN_PUMP_MANUAL_AUTO_SWITCH) {
  int value = param.asInt();
  manual_auto_switch = (value == 1);
}

BLYNK_WRITE(VPIN_PUMP_MANUAL_CONTROL) {
  int value = param.asInt();
  manualPumpControl = (value == 1);
}

void moisture() {
  int moistureValue = analogRead(MOISTURE_SENSOR);

  Blynk.virtualWrite(VPIN_MOISTURE, moistureValue);

  if (!manual_auto_switch) {
    // Control Pump Automatically based on moisture
    if (moistureValue < 400) {  // Soil is dry
      digitalWrite(RELAY_PIN, HIGH);  // Activate relay (pump ON)
    }
    else {
      digitalWrite(RELAY_PIN, LOW);  // Deactivate relay (pump OFF)
    }
  }
  else {
    // Control Pump Manual
    if (manualPumpControl) {
      digitalWrite(RELAY_PIN, HIGH);
    }
    else {
      digitalWrite(RELAY_PIN, LOW);
    }
  }
}


void LDR_send()
{
  int ldrValue = analogRead(LDR_PIN);
  Blynk.virtualWrite(VPIN_LDR, ldrValue);
}

void SHT31_send()
{
  float hum = sht31.readHumidity();
  float temp = sht31.readTemperature();

  if (isnan(hum) || isnan(temp)) {
    Blynk.virtualWrite(VPIN_HUM, 0);
    Blynk.virtualWrite(VPIN_TEMP, 0);
  }
  else {
    Blynk.virtualWrite(VPIN_HUM, hum);
    Blynk.virtualWrite(VPIN_TEMP, temp);
  }
}

void setup()
{
  sht31.begin(0x44);
  servo.attach(SERVO_PIN);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  Blynk.begin(auth, ssid, pass);

  // ===== Watchdog configuration =====
  int WDT_TIMEOUT = 10;
  const esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT * 1000,
    .idle_core_mask = (1<<0),
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);

  // ===== Detect if last reset was caused by WDT =====
  esp_reset_reason_t reason = esp_reset_reason();

  if (reason == ESP_RST_TASK_WDT || reason == ESP_RST_INT_WDT  || reason == ESP_RST_WDT) {
    // Send zeros to your virtual pins so we can see it on the app
    Blynk.virtualWrite(VPIN_LDR,   0);
    Blynk.virtualWrite(VPIN_HUM,   0);
    Blynk.virtualWrite(VPIN_TEMP,  0);
    Blynk.virtualWrite(VPIN_MOISTURE, 0);
  }
}

void loop()
{
  // Start counting the timeout
  esp_task_wdt_reset();

  // To test hanging
  while(true);

  Blynk.run();
  
  LDR_send();
  delay(500);
  SHT31_send();
  delay(500);
  moisture();
}