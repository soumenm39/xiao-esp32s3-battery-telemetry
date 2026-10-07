

#include <WiFi.h>
#include <ThingSpeak.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

WiFiClient client;

// =====================================================
// THINGSPEAK
// =====================================================

unsigned long myChannelNumber = YOUR_CHANNEL_NUMBER;
const char *myWriteAPIKey = "YOUR_THINGSPEAK_WRITE_API_KEY";

// =====================================================
// INA219
// =====================================================

Adafruit_INA219 ina219;

// XIAO ESP32-S3 Sense
// D4 = GPIO5 = SDA
// D5 = GPIO6 = SCL

#define SDA_PIN 5
#define SCL_PIN 6

// =====================================================
// Timing
// =====================================================

// Upload to ThingSpeak every 20 seconds
const unsigned long THINGSPEAK_INTERVAL = 20000;

// If Wi-Fi is lost, retry every 10 seconds
const unsigned long WIFI_RETRY_INTERVAL = 10000;

unsigned long lastThingSpeakUpdate = 0;
unsigned long lastWiFiAttempt = 0;

// =====================================================
// Wi-Fi connection function
// =====================================================

void connectWiFi()
{
  Serial.println();
  Serial.println("Trying to connect to Wi-Fi...");

  // Disconnect any stale connection
  WiFi.disconnect();

  delay(100);

  WiFi.begin(ssid, pass);

  lastWiFiAttempt = millis();
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  // ---------------------------------------------------
  // Serial Monitor is OPTIONAL
  // Wait max 15 seconds, then continue automatically.
  // ---------------------------------------------------

  unsigned long serialStart = millis();

  while (!Serial && (millis() - serialStart < 15000))
  {
    delay(100);
  }

  Serial.println();
  Serial.println("=================================");
  Serial.println(" XIAO ESP32-S3 Battery Monitor");
  Serial.println(" INA219 + ThingSpeak");
  Serial.println("=================================");

  // ---------------------------------------------------
  // I2C
  // ---------------------------------------------------

  Wire.begin(SDA_PIN, SCL_PIN);

  // ---------------------------------------------------
  // INA219
  // ---------------------------------------------------

  if (!ina219.begin())
  {
    Serial.println("ERROR: INA219 not found!");

    while (1)
    {
      delay(1000);
    }
  }

  Serial.println("INA219 detected at 0x40");

  // Temporary calibration
  ina219.setCalibration_32V_2A();

  // ---------------------------------------------------
  // Wi-Fi configuration
  // ---------------------------------------------------

  WiFi.mode(WIFI_STA);

  // ESP32 should automatically reconnect where possible
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  connectWiFi();

  // ---------------------------------------------------
  // ThingSpeak
  // ---------------------------------------------------

  ThingSpeak.begin(client);

  Serial.println("Battery monitor started.");
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
  unsigned long now = millis();

  // ===================================================
  // READ BATTERY
  // This continues even when internet is OFF.
  // ===================================================

  float voltage = ina219.getBusVoltage_V();

  float shuntVoltage = ina219.getShuntVoltage_mV();

  float current = ina219.getCurrent_mA();

  float power = ina219.getPower_mW();

  // ===================================================
  // Determine battery state
  // ===================================================

  int batteryStatus = 0;

  if (current > 20)
  {
    batteryStatus = 1;      // CHARGING
  }
  else if (current < -20)
  {
    batteryStatus = 2;      // DISCHARGING
  }
  else
  {
    batteryStatus = 0;      // IDLE
  }

  // ===================================================
  // SERIAL OUTPUT
  // ===================================================

  Serial.println("---------------------------------");

  Serial.print("Voltage: ");
  Serial.print(voltage, 3);
  Serial.println(" V");

  Serial.print("Current: ");
  Serial.print(current, 3);
  Serial.println(" mA");

  Serial.print("Power: ");
  Serial.print(power, 3);
  Serial.println(" mW");

  Serial.print("Shunt: ");
  Serial.print(shuntVoltage, 3);
  Serial.println(" mV");

  Serial.print("Battery: ");

  if (batteryStatus == 1)
  {
    Serial.println("CHARGING");
  }
  else if (batteryStatus == 2)
  {
    Serial.println("DISCHARGING");
  }
  else
  {
    Serial.println("IDLE");
  }

  // ===================================================
  // CHECK Wi-Fi
  // ===================================================

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Wi-Fi: DISCONNECTED");

    // Retry only every 10 seconds.
    // The rest of the program keeps running.
    if (now - lastWiFiAttempt >= WIFI_RETRY_INTERVAL)
    {
      Serial.println("Retrying Wi-Fi connection...");

      connectWiFi();
    }
  }
  else
  {
    Serial.print("Wi-Fi: CONNECTED  RSSI = ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  }

  // ===================================================
  // THINGSPEAK
  // ===================================================

  if (now - lastThingSpeakUpdate >= THINGSPEAK_INTERVAL)
  {
    lastThingSpeakUpdate = now;

    // Upload ONLY if internet/Wi-Fi is available
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println();
      Serial.println("Uploading to ThingSpeak...");

      // Field 1 = Battery voltage
      ThingSpeak.setField(1, voltage);

      // Field 2 = Current
      ThingSpeak.setField(2, current);

      // Field 3 = Power
      ThingSpeak.setField(3, power);

      // Field 4 = Shunt voltage
      ThingSpeak.setField(4, shuntVoltage);

      // Field 5:
      // 0 = IDLE
      // 1 = CHARGING
      // 2 = DISCHARGING
      ThingSpeak.setField(5, batteryStatus);

      int response =
        ThingSpeak.writeFields(
          myChannelNumber,
          myWriteAPIKey
        );

      if (response == 200)
      {
        Serial.println("ThingSpeak upload SUCCESS.");
      }
      else
      {
        Serial.print("ThingSpeak upload FAILED. Code: ");
        Serial.println(response);
      }
    }
    else
    {
      Serial.println(
        "No internet - ThingSpeak upload skipped."
      );
    }
  }

  delay(1000);
}
