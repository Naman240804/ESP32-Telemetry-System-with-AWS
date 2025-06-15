#include <esp_now.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Wi-Fi credentials (for internet use or server connection)
const char* ssid = "BJV 2G";
const char* password = "bjv@1234";

// AWS IoT Details
const char* mqtt_server = "a15vyn7fqmj1ao-ats.iot.us-east-1.amazonaws.com";
const int mqtt_port = 8883;
const char* shadow_topic = "$aws/things/Telemetry/shadow/name/ESP32/update";

// Certificates (Replace with your actual certificate strings)
const char* root_ca = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----

)EOF";

const char* device_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUNVNQGy10y0QU+yC5xnmAUNLRjeowDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MDMxOTA3NTMw
NloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPUyOa8olqOi6KqQk4Nh
bz7ltICHJsaN2E1g2PB/NMozsFvxZ63C8ATkWQw1CGJvXuN3Y/VAZ7tSF1fgU5Ht
ZzfDfy2+efQ4YFlT1NzSxc3PRMEd7pjYZLcKl1dbTqoOHeoR7A2uAP87dKGIngn4
A2/J9csuRBFAD6+8MEcm2NFwz1oYRspVGGxSd+aphkfwcx5PQFKLfuPawzwa8rqH
gYktIeGk1MpuYPHEmWNfO/yyB8FytilKwjbORpR1CVdUDvgG2iUhxqvmvLqXIOWJ
LvRM+uhVFlt8xKO+wdKxNCnn50sTpLjoT4dcBnUJr2oe4Wgpx56P3VlPi2VlO4BM
53sCAwEAAaNgMF4wHwYDVR0jBBgwFoAU40Zz+GRkXwZHy82VyDFm9FA3QzQwHQYD
VR0OBBYEFGhqvzqquHF+MtOP//rXMgx/OEKrMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCpwjP74/D+6Kq19XyxNnk/LxqW
0ScWYPkQ/yFJ//pggOZvv7fz/RE6u78Qd+tdzca7lBEHm8RuLAj2NrVqjlTSDkbm
d/KMlgZ7QJj1WRxXSpyEacajp/Rlpb5R7flgSnMBLALbOPEC/snOtkMiwyONiNdI
YP744fQ2UJV/T7KuOg0V9STnssxYDf3ff3eCdnV1s+NnohZGaw0U8Ls09JX2xc7Q
yU1ey+TZUwxLd+cZSo4eSc7cFeJ4Fo0cJtELbtqDKBkyeFTTE9VDf9SVH70Aakdv
siQy1DvU0riJi+xT2Cq8hjpBE/vA56NNZifKdF3Hd1rI6XcVrF5joteSjua/
-----END CERTIFICATE-----

)EOF";

const char* device_private_key = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEA9TI5ryiWo6LoqpCTg2FvPuW0gIcmxo3YTWDY8H80yjOwW/Fn
rcLwBORZDDUIYm9e43dj9UBnu1IXV+BTke1nN8N/Lb559DhgWVPU3NLFzc9EwR3u
mNhktwqXV1tOqg4d6hHsDa4A/zt0oYieCfgDb8n1yy5EEUAPr7wwRybY0XDPWhhG
ylUYbFJ35qmGR/BzHk9AUot+49rDPBryuoeBiS0h4aTUym5g8cSZY187/LIHwXK2
KUrCNs5GlHUJV1QO+AbaJSHGq+a8upcg5Yku9Ez66FUWW3zEo77B0rE0KefnSxOk
uOhPh1wGdQmvah7haCnHno/dWU+LZWU7gEznewIDAQABAoIBAQCLChcRz1dUpW2A
QwmTJ644oXMwqvL/BrJQdomuHsQloRfxmzuqme8XJxGjzXLatsK9BBennAT2gP+B
slBRYKj2QyKcIvXu2gUwwkvjiJXoNg+L3thMzfCWpyfYvosl/WlTMSSFQVWJZgKm
Et1jU8NHIeQCelLY4VXD1lgeENZENLMJxmbHzhKeJmwn3aTRg293niBrA6HwM5Hr
bJ50lSEbdGay/Yg+R3XtSGvVS6c0q/ctzl4rQC+F8fVxpNFgzzw7J/lv20PlYG8K
bzz3i5Hl3n8dkTtyNJuu8c+xxvJUTEqMfhgu0NgqEDMX8qj+rNHUTiVzKVlmvkKH
HA/MXRHZAoGBAPw8m5eKJMUZuxgFOIoXImm1YvmVD59XYnaFezhVZZxxYAdNKbAh
ZtlA6lkCgC5FOnrOm77U+xsZASF6F0qLYNoeqZlNoym9bBRAcLdwKMy4jFPtr59d
KlJTV8Zp02KV0kIYsuCT56ZvaK8bbLsSjnpIPWA4jZWl41XJUZmbwzEHAoGBAPja
uhQeYXOtPN0NBtdewbodeag57PMQr4gJrP1LpAIlGz1mJ16iw0pZkKc2hFrtrcrP
PiEqkF9bdSeO3Sm+jCq6IUxHKQ8EArKY7R+yeXR/tmUxWl2SVFzBhB+LA/6Q55bg
fvmNYMbjI+eRdBzNnyeY8SpkIvgoa6gNygdUtlztAoGBAPDLWy3bryaKXdWchlEt
rKiFhKsG5hcTcOf8nZk1zP8ALNBLlZSTDoAwPIgknwGNDTrXqk8PO3iu8ilnSHzd
eeRAiV/UggMC1f6vVeuIoypss2ob6iCU7CYTciP0OtxOcp6uvXUvPWHh0Wn5IF+m
Y2XROOZ2lpNZ/iCTQ9cEn95ZAoGBAJ6rB13wiP4yhM4Ekkob1rWOOZVzaYsJJoI8
eRn/cpETi6y4R33llVMAjGSxM+6uANOBFAIdm3PRYn9YbddB8SkFCFoUPS+fxQCO
1RISv7HyBB+9OfrTZArIMC/UeTWEusTOWudOp1foV2o8qaX7zPB4abw3oLxNOWcr
c5b9AlF5AoGAQM/37mrQ2eTR9jsDF9EOj9luXHkKHgfrJNkJIfg6r6iEJRl/R23Z
2jlQaEsMaRC3++9XV6IQCjVEiBeQAXI9M1KPndfcxihBYAztmG1DXeYf+U99bQTu
W9w/uMelXoK+DSpTAxIGKRH0sMBVDxrUijbemhETrfBKaeHs/c6A718=
-----END RSA PRIVATE KEY-----

)EOF";

// WiFi and MQTT Setup
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Replace with MAC address of ESP32-2 (receiver)
uint8_t broadcastAddress[] = {0x08, 0xD1, 0xF9, 0x34, 0x66, 0x10};

// Define the structure to send (must match receiver)
typedef struct struct_message {
  int rpm;
  uint8_t voltage;
  uint8_t current;
  uint8_t motorTemp;
  uint8_t controllerTemp;
} struct_message;

struct_message motorData;
esp_now_peer_info_t peerInfo;

// Callback for data sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // Set AWS IoT Certificates
  espClient.setCACert(root_ca);
  espClient.setCertificate(device_cert);
  espClient.setPrivateKey(device_private_key);

  // Connect to AWS IoT
  client.setServer(mqtt_server, mqtt_port);
  
  Serial.print("🔗 Connecting to AWS IoT...");
  if (client.connect("ESP32Client")) {
    Serial.println("✅ Connected!");
  } else {
    Serial.print("❌ Failed to connect. State: ");
    Serial.println(client.state());  // Shows error code
  }

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  motorData.rpm = random(1000, 5000);
  motorData.voltage = random(120, 150) / 10.0;
  motorData.current = random(10, 50) / 10.0;
  motorData.motorTemp = random(30, 90);
  motorData.controllerTemp = random(25, 80);
    if (!client.connected()) {
    Serial.println("⚠️ MQTT Disconnected! Reconnecting...");
    client.connect("ESP32Client");
  }

  // Create a JSON payload for Named Shadow update (without timestamp)
  StaticJsonDocument<256> doc;
  doc["state"]["reported"]["device_id"] = "ESP32";
  doc["state"]["reported"]["motor_rpm"] = motorData.rpm;
  doc["state"]["reported"]["temperature"] = motorData.motorTemp;
  doc["state"]["reported"]["voltage"] = motorData.voltage;

  
  // Convert JSON object to string
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  // Debug: Print JSON to Serial Monitor
  Serial.println("Sending JSON to AWS IoT:");
  Serial.println(jsonBuffer);

  // Publish to AWS IoT Named Shadow (ESP32)
  if (client.publish(shadow_topic, jsonBuffer)) {
    Serial.println("✅ Named Shadow `ESP32` Updated!");
  } else {
    Serial.println("❌ Failed to Publish to AWS IoT");
  }

  client.loop();

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&motorData, sizeof(motorData));

  if (result == ESP_OK) {
    Serial.println("Data sent successfully:");
    Serial.printf("RPM: %d, Voltage: %u, Current: %u, MotorTemp: %u, ControllerTemp: %u\n",
      motorData.rpm, motorData.voltage, motorData.current, motorData.motorTemp, motorData.controllerTemp);
  } else {
    Serial.println("Error sending the data");
  }

  delay(2000);
}
