#define BLYNK_TEMPLATE_ID "TMPL6oo5-PMp6"
#define BLYNK_TEMPLATE_NAME "Prototype Tubes PRD"
#define BLYNK_AUTH_TOKEN "ucR03fLtebZ987cGVDgn2GcQewWZSm4b"

#define WIFI_SSID "Edw"
#define WIFI_PASS "11223344"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTTYPE DHT22

// Pin config
#define PIN_RELAY D1
#define PIN_DHT   D4
#define PIN_SDA   D6
#define PIN_SCL   D5

#define BLYNK_TEMP_PIN    V0
#define BLYNK_HUMID_PIN   V1
#define BLYNK_FAN_PIN     V2
#define BLYNK_DHT_PIN     V3

DHT dht(PIN_DHT, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

#define FAN_ON  1
#define FAN_OFF 0

#define DHT_ON  1
#define DHT_OFF 0

#define HUMID_ON_THRESHOLD  85.0
#define HUMID_OFF_THRESHOLD 75.0

#define TEMP_ON_THRESHOLD  28.0
#define TEMP_OFF_THRESHOLD 27.0

float lastTemp = NAN;
float lastHumid = NAN;
bool fanState = false;

void setFan(bool on) {
  if (fanState == on) return;
  fanState = on;

  digitalWrite(PIN_RELAY, on ? LOW : HIGH);

  if (Blynk.connected())
    Blynk.virtualWrite(BLYNK_FAN_PIN, on ? FAN_ON : FAN_OFF);

  Serial.print("Kipas: ");
  Serial.println(on ? "Nyala" : "Mati");
}

BLYNK_WRITE(BLYNK_FAN_PIN) {
  int val = param.asInt();
  setFan(val == FAN_ON);
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(BLYNK_FAN_PIN);
}

void sendSensorData() {
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();

  if (isnan(temp) || isnan(humid)) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor failed!      ");
    Serial.println("Failed to read from DHT22!");
    lcd.setCursor(0, 1);
    lcd.print("Cek kabel.      ");

    digitalWrite(PIN_RELAY, HIGH);
    fanState = false;
    
    if (Blynk.connected()) {
      Blynk.virtualWrite(BLYNK_DHT_PIN, DHT_OFF);
      Blynk.virtualWrite(BLYNK_FAN_PIN, FAN_OFF);
    }

    return;
  }

  if (Blynk.connected()) {
    Blynk.virtualWrite(BLYNK_DHT_PIN, DHT_ON);
    Blynk.virtualWrite(BLYNK_TEMP_PIN, temp);
    Blynk.virtualWrite(BLYNK_HUMID_PIN, humid);
  }

  if (temp != lastTemp || humid != lastHumid) {
    lastTemp = temp;
    lastHumid = humid;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Humid: ");
    lcd.print(humid);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcd.print(temp);
    lcd.print("C");
  }

  if (humid >= HUMID_ON_THRESHOLD || temp >= TEMP_ON_THRESHOLD) setFan(true);
  else if (humid < HUMID_OFF_THRESHOLD && temp < TEMP_OFF_THRESHOLD) setFan(false);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("System offline");

  delay(2000);

  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, HIGH);

  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);

  timer.setInterval(2000L, sendSensorData);
  
  lcd.clear();
  lcd.print("System online");
}

void loop() {
  Blynk.run();
  timer.run();
}