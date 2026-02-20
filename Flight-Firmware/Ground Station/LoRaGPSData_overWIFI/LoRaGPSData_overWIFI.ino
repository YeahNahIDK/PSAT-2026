#include <WiFi.h>
#include <LoRa.h>

// -------- LoRa PIN DEFINITIONS --------
#define LORA_SCK     5    // GPIO5  -- SX1278's SCK
#define LORA_MISO    19   // GPIO19 -- SX1278's MISO
#define LORA_MOSI    27   // GPIO27 -- SX1278's MOSI
#define LORA_SS      18   // GPIO18 -- SX1278's CS
#define LORA_RST     14   // GPIO14 -- SX1278's RESET
#define LORA_DIO0    26   // GPIO26 -- SX1278's IRQ (DIO0)

#define LORA_BAND    915E6  // Frequency (915MHz for US, 868MHz for EU, 433MHz, etc.)

// -------- WiFi CREDENTIALS --------
const char* ssid = "SSID";
const char* password = "password";

WiFiServer server(80);

// ================= GPS DATA ============================
float latitude  = 0.0;
float longitude = 0.0;
bool  hasFix    = false;
unsigned long lastLoRaReceive = 0;

void setup() {
  Serial.begin(115200);

  // ---------- LoRa SETUP ----------
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  
  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("LoRa init failed!");
    while (1);
  }
  
  Serial.println("LoRa init succeeded");
  
  // Optional: Set LoRa parameters for better range/reliability
  // LoRa.setSpreadingFactor(12);      // SF7 to SF12 (higher = longer range, slower)
  // LoRa.setSignalBandwidth(125E3);   // 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3
  // LoRa.setCodingRate4(5);           // 5, 6, 7, 8
  // LoRa.setSyncWord(0x12);           // Sync word (0x12 is default, 0x34 for LoRaWAN)

  delay(1000);

  // ---------- WIFI ----------
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  // ---------- WEB SERVER ----------
  server.begin();
}

void loop() {
  // ================= READ LoRa GPS DATA =================
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    
    // Read the packet
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    
    Serial.print("Received LoRa packet: ");
    Serial.println(received);
    
    // Print RSSI (signal strength)
    Serial.print("RSSI: ");
    Serial.println(LoRa.packetRssi());
    
    // Parse GPS data from LoRa packet
    // Expected format: "FIX,lat,lon" or "NOFIX"
    parseLoRaGPSData(received);
    lastLoRaReceive = millis();
  }

  // Check if data is stale (no update in 10 seconds)
  if (millis() - lastLoRaReceive > 10000 && lastLoRaReceive != 0) {
    hasFix = false;
  }

  // ================= WEB SERVER =================
  WiFiClient client = server.available();
  if (!client) return;

  while (!client.available()) delay(1);
  String request = client.readStringUntil('\r');
  client.flush();

  // ---------- GPS JSON ENDPOINT ----------
  if (request.indexOf("/gps") != -1) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.print("{\"fix\":");
    client.print(hasFix ? "true" : "false");
    client.print(",\"lat\":");
    client.print(latitude, 6);
    client.print(",\"lon\":");
    client.print(longitude, 6);
    client.print("}");
  }

  // ---------- MAIN WEB PAGE ----------
  else {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    client.println(R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 GPS Tracker (LoRa)</title>
</head>
<body>
  <h1>ESP32 GPS Tracker (via LoRa)</h1>

  <p><b>Status:</b> <span id="status">Waiting...</span></p>
  <p><b>Latitude:</b> <span id="lat">--</span></p>
  <p><b>Longitude:</b> <span id="lon">--</span></p>

  <p>
    <a id="map" href="#" target="_blank">Open in Google Maps</a>
  </p>

  <script>
    setInterval(() => {
      fetch('/gps')
        .then(res => res.json())
        .then(data => {
          if (data.fix) {
            document.getElementById('status').innerHTML = "GPS Fix Acquired (LoRa)";
            document.getElementById('lat').innerHTML = data.lat;
            document.getElementById('lon').innerHTML = data.lon;
            document.getElementById('map').href =
              "https://www.google.com/maps?q=" +
              data.lat + "," + data.lon;
          } else {
            document.getElementById('status').innerHTML = "Waiting for GPS fix...";
          }
        });
    }, 1000);
  </script>
</body>
</html>
)rawliteral");
  }
  client.stop();
}

// ================= PARSE LoRa GPS DATA =================
void parseLoRaGPSData(String data) {
  // Expected format: "FIX,lat,lon" or "NOFIX"
  data.trim();
  
  if (data.startsWith("FIX,")) {
    // Remove "FIX," prefix
    data = data.substring(4);
    
    // Find comma separator
    int commaIndex = data.indexOf(',');
    if (commaIndex > 0) {
      String latStr = data.substring(0, commaIndex);
      String lonStr = data.substring(commaIndex + 1);
      
      latitude = latStr.toFloat();
      longitude = lonStr.toFloat();
      hasFix = true;
      
      Serial.print("Parsed GPS: ");
      Serial.print(latitude, 6);
      Serial.print(", ");
      Serial.println(longitude, 6);
    }
  } else if (data == "NOFIX") {
    hasFix = false;
    Serial.println("No GPS fix received");
  }
}