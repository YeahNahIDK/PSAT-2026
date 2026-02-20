#include <WiFi.h>
#include <Adafruit_GPS.h>

// -------- GPS PIN DEFINITIONS --------
#define GPS_TX 4   // ESP32 TX -> GPS RX
#define GPS_RX 5   // ESP32 RX -> GPS TX

// -------- WiFi CREDENTIALS --------
const char* ssid = "SSID";
const char* password = "password";

WiFiServer server(80);
HardwareSerial GPSSerial(2);
Adafruit_GPS GPS(&GPSSerial);

// ================= GPS DATA ============================
float latitude  = 0.0;
float longitude = 0.0;
bool  hasFix    = false;

void setup() {
  Serial.begin(115200);

  // ---------- GPS SERIAL ----------
  GPSSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); // Enable location sentences
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1Hz update
  GPS.sendCommand(PGCMD_ANTENNA);

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
  // ================= READ GPS =================
  while (GPSSerial.available()) {
    GPS.read();
  }

  if (GPS.newNMEAreceived()) {
    Serial.println(GPS.lastNMEA());   // DEBUG: raw GPS data

    if (!GPS.parse(GPS.lastNMEA())) {
      Serial.println("GPS parse failed");
    }
  }

  // ================= UPDATE LOCATION =================
  if (GPS.fix) {
    latitude  = GPS.latitudeDegrees;
    longitude = GPS.longitudeDegrees;
    hasFix = true;
  } else {
    hasFix = false;
  }


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
  <title>ESP32 GPS Tracker</title>
</head>
<body>
  <h1>ESP32 GPS Tracker</h1>

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
            document.getElementById('status').innerHTML = "GPS Fix Acquired";
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