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
#define PUMP_PIN 26
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

void moisture() {
  // Read and map moisture level
  moistureValue = analogRead(MOISTURE_SENSOR);
  moistureValue = map(moistureValue, 0, 1023, 0, 100);

  // Send moisture to Blynk (Virtual Pin V0)
  Blynk.virtualWrite(V4, moistureValue);

  // Control Pump Automatically
  // Change 255 to the real threshold value after simulating in Lab
  if (moistureValue < 255) {
    digitalWrite(PUMP_PIN, HIGH); // Activate pump
    pumpStatus = "ON";
  }
  else {
    digitalWrite(PUMP_PIN, LOW); // Deactivate pump
    pumpStatus = "OFF";
  }
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
    Serial.print(F(" °C "));
    Blynk.virtualWrite(VPIN_HUM, hum);
    Blynk.virtualWrite(VPIN_TEMP, temp);
  }
}

void setup()
{
  Serial.begin(115200);
  
  sht31.begin(0x44);

  servo.attach(SERVO_PIN);

  pinMode(LED_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, HIGH); // Pump OFF initially

  Blynk.begin(auth, ssid, pass);
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