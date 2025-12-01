#include <WiFi.h>
#include <ArduinoMqttClient.h>

#include "UiCom.h"
#include "stateHolder.h"

// ESP-IDF 5.x WPA2-Enterprise API
#include "esp_wifi.h"
#include "esp_eap_client.h"

// ===================================================
// Credentials
// ===================================================
const char* ssid = "eduroam";
const char* username = "zcabddw@ucl.ac.uk";
const char* password = "toVmom-gaggyx-0jejgi";

// ThingsBoard
const char* mqttServer = "demo.thingsboard.io";
const int mqttPort = 1883;
const char* token = "exn0x81mpavwoa4oizjb";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);


// ===================================================
// WiFi - WPA2 Enterprise (eduroam)
// ===================================================
void connectWiFi() {
    WiFi.mode(WIFI_STA);

    // Enable WPA2-Enterprise mode
    esp_wifi_sta_enterprise_enable();

    // Identity + credentials
    esp_eap_client_set_identity((const uint8_t*)username, strlen(username));
    esp_eap_client_set_username((const uint8_t*)username, strlen(username));
    esp_eap_client_set_password((const uint8_t*)password, strlen(password));

    WiFi.begin(ssid);

    Serial.print("Connecting to eduroam");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to eduroam!");
}


// ===================================================
// Helper: Extract number from JSON
// ===================================================
float extractNumber(const String& json, const String& key) {
    int idx = json.indexOf(key);
    if (idx == -1) return NAN;

    idx = json.indexOf(":", idx);
    if (idx == -1) return NAN;

    int start = idx + 1;

    int end = start;
    while (end < json.length() &&
          (isdigit(json[end]) || json[end] == '.' || json[end] == '-')) {
        end++;
    }

    String numStr = json.substring(start, end);
    return numStr.toFloat();
}


// ===================================================
// Helper: Print ALL setpoints
// ===================================================
void printSetpoints() {
    Serial.println("========== CURRENT SETPOINTS ==========");
    Serial.println("Target RPM : " + String(reactorState.targetrpM));
    Serial.println("Target pH  : " + String(reactorState.targetpH));
    Serial.println("Target Temp: " + String(reactorState.targetTemp));
    Serial.println("========================================");
}


// ===================================================
// Handle shared attributes manually
// ===================================================
void onAttributeUpdate(int messageSize) {
    String topic = mqttClient.messageTopic();
    String payload;

    while (mqttClient.available()) {
        payload += (char)mqttClient.read();
    }

    Serial.println("Attribute update: " + payload);

    // Example payload:
    // {"shared":{"setRPM":1200,"setPH":6.5,"setTemp":32}}

    float rpm = extractNumber(payload, "\"setRPM\"");
    if (!isnan(rpm)) {
        reactorState.targetRPM = (int)rpm;
        Serial.println("Updated setRPM = " + String(reactorState.targetRPM));
        printSetpoints();
    }

    float ph = extractNumber(payload, "\"setPH\"");
    if (!isnan(ph)) {
        reactorState.targetPH = ph;
        Serial.println("Updated setPH = " + String(reactorState.targetpH));
        printSetpoints();
    }

    float temp = extractNumber(payload, "\"setTemp\"");
    if (!isnan(temp)) {
        reactorState.targetTemp = temp;
        Serial.println("Updated setTemp = " + String(reactorState.targetTemp));
        printSetpoints();
    }
}


// ===================================================
// MQTT connect + subscribe to shared attributes
// ===================================================
void connectMQTT() {
    Serial.print("Connecting to MQTT... ");

    mqttClient.setUsernamePassword(token, "");

    if (!mqttClient.connect(mqttServer, mqttPort)) {
        Serial.print("FAILED, Code = ");
        Serial.println(mqttClient.connectError());
        return;
    }

    Serial.println("Connected to MQTT.");

    mqttClient.onMessage(onAttributeUpdate);
    mqttClient.subscribe("v1/devices/me/attributes");

    Serial.println("Subscribed to shared attributes.");
}


// ===================================================
// Telemetry send
// ===================================================
void sendTelemetry() {
    String payload =
        "{\"ph\":" + String(reactorState.currentPH) +
        ",\"rpm\":" + String(reactorState.currentRPM) +
        ",\"temp\":" + String(reactorState.currentTemp) +
        "}";

    mqttClient.beginMessage("v1/devices/me/telemetry");
    mqttClient.print(payload);
    mqttClient.endMessage();

    Serial.println("Telemetry sent: " + payload);
}


// ===================================================
// Init + Loop
// ===================================================
void initUI() {
    connectWiFi();
    connectMQTT();
}

void handleUI() {
    if (!mqttClient.connected()) {
        connectMQTT();
    }

    mqttClient.poll();
    sendTelemetry();
}










