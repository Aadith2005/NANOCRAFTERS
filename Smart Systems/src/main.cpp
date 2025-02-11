#include <Arduino.h>

#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ------------------- ThingSpeak Credentials -------------------
const char* ssid = "Mukesh";               // WiFi SSID
const char* password = "mukesh04120";      // WiFi Password
const char* server = "http://api.thingspeak.com/update";
const char* apiKey = "J9QAULPPD90PF1AI";   // Your API Key
const char* channelID = "2835908";         // Your Channel ID

// ------------------- Sensor & OLED Definitions -------------------
#define DHTPIN         4
#define DHTTYPE        DHT11
#define GAS_SENSOR_PIN 33
#define RELAY_PIN      13

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DHT dht(DHTPIN, DHTTYPE);

// ------------------- Function to Send Data to ThingSpeak -------------------
void sendToThingSpeak(float temperature, float humidity, int gasValue, bool relayStatus) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String url = String(server) + "?api_key=" + apiKey +
                 "&field1=" + String(temperature) +
                 "&field2=" + String(humidity) +
                 "&field3=" + String(gasValue) +
                 "&field4=" + String(relayStatus);
    
    Serial.println("Sending data to ThingSpeak: " + url);
    
    http.begin(url);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      Serial.println("Data sent successfully to ThingSpeak!");
    } else {
      Serial.println("Error sending data: " + String(httpResponseCode));
    }
    
    http.end();
  } else {
    Serial.println("WiFi Disconnected! Cannot send data.");
  }
}

// ------------------- Function to Update OLED Display -------------------
void updateOLED(float temperature, float humidity, int gasValue, bool relayStatus) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Temp: " + String(temperature, 1) + " C");
  display.println("Humidity: " + String(humidity, 1) + " %");
  display.println("Gas: " + String(gasValue));
  display.println("Relay: " + String(relayStatus ? "ON" : "OFF"));
  display.display();
}

// ------------------- Setup Function -------------------
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("OLED initialization failed");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED Initialized!");
  display.display();
  delay(2000);

  dht.begin();
  pinMode(GAS_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

// ------------------- Loop Function -------------------
void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int gasValue = analogRead(GAS_SENSOR_PIN);
  
  // Relay ON if threshold is exceeded
  bool relayStatus = (temperature > 24 || humidity < 40 || humidity > 60 || gasValue > 500);
  digitalWrite(RELAY_PIN, relayStatus ? HIGH : LOW);

  // Update OLED Display
  updateOLED(temperature, humidity, gasValue, relayStatus);

  // Send Data to ThingSpeak
  sendToThingSpeak(temperature, humidity, gasValue, relayStatus);

  delay(15000);  // ThingSpeak allows updates every 15 seconds
}
