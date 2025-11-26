// Replace those with your Blynk account Tokens
#define BLYNK_TEMPLATE_ID "TMPL4vC03FgXG"
#define BLYNK_TEMPLATE_NAME "IndustrialProject"
#define BLYNK_AUTH_TOKEN "I_MHZ2hUc8477q_VxdLw7oxEm8obORIE"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_SHT31.h>
#include <ESP32Servo.h>

// ====== WiFi and Blynk credentials ======
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "ZTE_80B3BE";   // Replace with your WiFi name
char pass[] = "2ZL48Y8226";   // Replace with your WiFi password

// ====== Hardware ======
#define LDR_PIN 32
#define SERVO_PIN 25
#define RELAY_PIN 26  // Relay for water pump (9-12V)
#define MOISTURE_SENSOR 34

int moistureValue = 0;
String pumpStatus = "OFF";

// Connect SHT to pins 21 (SDA) and 22 (SCL) No need to define pins in the code for this sensor only
Adafruit_SHT31 sht31 = Adafruit_SHT31();

Servo servo;

// ====== Virtual Pins in Blynk app ======
#define VPIN_LDR   V0
#define VPIN_HUM   V1
#define VPIN_TEMP  V2
#define VPIN_SERVO V3
#define VPIN_MOISTURE V4
#define VPIN_PUMP_MANUAL V5  // Manual pump control from Blynk

bool manualPumpControl = false;  // Flag for manual vs automatic control

BLYNK_WRITE(VPIN_SERVO) {
  int value = param.asInt();
  // value is one or zero, as it is a digital ouput from blynk
  servo.write(value * 180);

  if (value) {
    Serial.println("Window is closed");
  }
  else {
    Serial.println("Window is open");
  }
}

BLYNK_WRITE(VPIN_PUMP_MANUAL) {
  int value = param.asInt();
  manualPumpControl = (value == 1);
  
  if (manualPumpControl) {
    digitalWrite(RELAY_PIN, HIGH);  // Turn pump ON
    pumpStatus = "ON (Manual)";
    Serial.println("Water pump ON (Manual Control)");
  }
  else {
    digitalWrite(RELAY_PIN, LOW);  // Turn pump OFF
    pumpStatus = "OFF (Manual)";
    Serial.println("Water pump OFF (Manual Control)");
  }
}

void moisture() {
  // Read and map moisture level
  moistureValue = analogRead(MOISTURE_SENSOR);
  moistureValue = map(moistureValue, 0, 4095, 0, 100);  // ESP32 ADC is 12-bit (0-4095)

  // Send moisture to Blynk
  Blynk.virtualWrite(VPIN_MOISTURE, moistureValue);

  // Only control pump automatically if manual control is not active
  if (!manualPumpControl) {
    // Control Pump Automatically based on moisture
    // Change 30 to the real threshold value after testing in Lab
    if (moistureValue < 30) {  // Soil is dry
      digitalWrite(RELAY_PIN, HIGH);  // Activate relay (pump ON)
      pumpStatus = "ON (Auto)";
      Serial.println("Water pump ON (Automatic - Low Moisture)");
    }
    else {
      digitalWrite(RELAY_PIN, LOW);  // Deactivate relay (pump OFF)
      pumpStatus = "OFF (Auto)";
      Serial.println("Water pump OFF (Automatic - Moisture OK)");
    }
  }
  
  Serial.print("Moisture: ");
  Serial.print(moistureValue);
  Serial.print("% - Pump Status: ");
  Serial.println(pumpStatus);
}

void LDR_send()
{
  int ldrValue = analogRead(LDR_PIN);
  Serial.print("LDR = ");
  Serial.println(ldrValue);
  Blynk.virtualWrite(VPIN_LDR, ldrValue);
}

void SHT31_send()
{
  float hum = sht31.readHumidity();
  float temp = sht31.readTemperature();

  if (isnan(hum) || isnan(temp)) {
    Serial.println(F("Failed to read from SHT31 sensor!"));
    return;
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(hum);
    Serial.print(" % ");
    Serial.print(F("Temperature: "));
    Serial.print(temp);
    Serial.println(F(" °C"));
    Blynk.virtualWrite(VPIN_HUM, hum);
    Blynk.virtualWrite(VPIN_TEMP, temp);
  }
}

void setup()
{
  Serial.begin(115200);
  
  sht31.begin(0x44);
  servo.attach(SERVO_PIN);

  // Initialize relay pin for water pump
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Pump OFF initially (relay not active)

  Blynk.begin(auth, ssid, pass);
  
  Serial.println("System initialized - Pump control ready");
}

void loop()
{
  Blynk.run();
  
  moisture();
  delay(500);
  LDR_send();
  delay(500);
  SHT31_send();
}