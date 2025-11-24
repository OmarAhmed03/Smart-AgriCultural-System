#define BLYNK_TEMPLATE_ID "TMPL4vC03FgXG"
#define BLYNK_TEMPLATE_NAME "IndustrialProject"
#define BLYNK_AUTH_TOKEN "I_MHZ2hUc8477q_VxdLw7oxEm8obORIE"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
//#include <Adafruit_SHT31.h>
#include <DHT.h>
#include <ESP32Servo.h>

// ====== WiFi and Blynk credentials ======
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";  // For Wokwi simulation
char pass[] = "";

// ====== Hardware ======
#define LDR_PIN 36
#define SERVO_PIN 2
#define DHT_PIN 21

DHT dht22(DHT_PIN, DHT22);
//Adafruit_SHT31 sht31 = Adafruit_SHT31();
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

void LDR_send()
{
  int ldrValue = analogRead(LDR_PIN);
  Serial.print("LDR = ");
  Serial.println(ldrValue);
  Blynk.virtualWrite(VPIN_LDR, ldrValue);
}

void SHT31_send()
{
  float hum = dht22.readHumidity();
  float temp = dht22.readTemperature();
  //float hum = sht31.readHumidity();
  //float temp = sht31.readTemperature();

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
  
  dht22.begin();
  //sht31.begin(0x44);
  servo.attach(SERVO_PIN);

  Blynk.begin(auth, ssid, pass);
}

void loop()
{
  Blynk.run();
  
  LDR_send();
  delay(500);
  SHT31_send();
}
