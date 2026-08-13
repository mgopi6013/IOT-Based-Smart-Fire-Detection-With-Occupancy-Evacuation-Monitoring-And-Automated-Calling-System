#define BLYNK_TEMPLATE_ID "TMPL3Ga0gtJkk"
#define BLYNK_TEMPLATE_NAME "project fire"
#define BLYNK_AUTH_TOKEN "uO5pxRHH5v9zeT2R01L_LSDolPht9DGb"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "DHT.h"

// Your WiFi details
char auth[] = "uO5pxRHH5v9zeT2R01L_LSDolPht9DGb";
char ssid[] = "TechKnots";
char pass[] = "TechKnots";

// Pin setup
#define IR1 D1
#define IR2 D2
#define FLAME D8
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

int count = 0;
unsigned long timeout = 2500;      // 2.5 sec between IR sensors
unsigned long debounceDelay = 700; // prevent double counting
bool fireDetected = false;

BlynkTimer timer;

void setup() {
  Serial.begin(9600);
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(FLAME, INPUT);
  dht.begin();

  Blynk.begin(auth, ssid, pass);

  timer.setInterval(500L, checkPeople);
  timer.setInterval(1000L, checkFire);

  Serial.println("🔥 Smart Room Monitoring Started...");
}

// --------------------------------------------------------------------
void loop() {
  Blynk.run();
  timer.run();
}

// --------------------------------------------------------------------
// 👥 People Counting
void checkPeople() {
  int ir1State = digitalRead(IR1);
  int ir2State = digitalRead(IR2);

  // Person ENTER
  if (ir1State == LOW) {
    unsigned long startTime = millis();
    while (millis() - startTime < timeout) {
      if (digitalRead(IR2) == LOW) {
        count++;
        Serial.println("✅ Person ENTERED");
        Serial.print("👥 Current Count: "); Serial.println(count);
        Blynk.virtualWrite(V1, count);
        delay(debounceDelay);
        while (digitalRead(IR1) == LOW || digitalRead(IR2) == LOW);
        break;
      }
    }
  }

  // Person EXIT
  if (ir2State == LOW) {
    unsigned long startTime = millis();
    while (millis() - startTime < timeout) {
      if (digitalRead(IR1) == LOW) {
        if (count > 0) count--;
        Serial.println("🚪 Person EXITED");
        Serial.print("👥 Current Count: "); Serial.println(count);
        Blynk.virtualWrite(V1, count);
        delay(debounceDelay);
        while (digitalRead(IR1) == LOW || digitalRead(IR2) == LOW);
        break;
      }
    }
  }
}

// --------------------------------------------------------------------
// 🔥 Fire Detection + Alert
void checkFire() {
  int fireVal = digitalRead(FLAME);

  if (fireVal == LOW && fireDetected) { // fire detected
    fireDetected = true;

    float temperature = dht.readTemperature();
    if (isnan(temperature)) {
      Serial.println("Temperature Read Error!");
      return;
    }

    Serial.println("🔥 FIRE DETECTED!");
    Serial.print("🌡 Room Temperature: "); Serial.print(temperature); Serial.println("°C");
    Serial.print("👥 People Count: "); Serial.println(count);

    // Build alert message
    String msg = "🔥 FIRE ALERT!\n\n";
    msg += "🌡 Room Temperature: " + String(temperature) + "°C\n";
    msg += "👥 People in Room: " + String(count) + "\n";
    msg += "⚠️ Please rescue immediately!";

    // Send alert to Blynk
    Blynk.logEvent("fire_alert", msg);
    Serial.println("📩 Email Alert Sent!");
  }

  if (fireVal == HIGH && !fireDetected) {
    fireDetected = false;
    Serial.println("✅ Fire Cleared.");
  }
}
