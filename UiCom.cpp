#include <WiFi.h>
#include <ArduinoMqttClient.h>

#include "UiCom.h"
#include "StateHolder.h"

#include "esp_wifi.h"
#include "esp_eap_client.h"

const char* ssid = "eduroam";
const char* username = "zcabddw@ucl.ac.uk";
const char* password = "toVmom-gaggyx-0jejgi";

const char* mqttServer = "demo.thingsboard.io";
const int mqttPort = 1883;
const char* token = "exn0x81mpavwoa4oizjb";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

void connectWiFi() {
    WiFi.mode(WIFI_STA);

    esp_wifi_sta_enterprise_enable();

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

float extract(const String& p, const String& key) {
    int i = p.indexOf(key);
    if (i < 0) return NAN;
    i = p.indexOf(":", i);
    if (i < 0) return NAN;
    int s = i + 1;
    while (s < p.length() && (p[s] == ' ' || p[s] == '\"')) s++;
    int e = s;
    while (e < p.length() && (isdigit(p[e]) || p[e] == '.' || p[e] == '-')) e++;
    return p.substring(s, e).toFloat();
}

void printSetpoints() {
    Serial.println("========== CURRENT SETPOINTS ==========");
    Serial.println("Target RPM : " + String(reactorState.targetRpm));
    Serial.println("Target pH  : " + String(reactorState.targetpH));
    Serial.println("Target Temp: " + String(reactorState.targetTemp));
    Serial.println("========================================");
}

void onAttributeUpdate(int size) {
    String payload;
    while (mqttClient.available()) payload += (char)mqttClient.read();

    Serial.println("Attribute update: " + payload);

    float rpm = extract(payload, "setRPM");
    if (!isnan(rpm)) {
        reactorState.targetRpm = rpm;
        Serial.println("Updated setRPM = " + String(reactorState.targetRpm));
        printSetpoints();
    }

    float ph = extract(payload, "setPH");
    if (!isnan(ph)) {
        reactorState.targetpH = ph;
        Serial.println("Updated setPH = " + String(reactorState.targetpH));
        printSetpoints();
    }

    float temp = extract(payload, "setTemp");
    if (!isnan(temp)) {
        reactorState.targetTemp = temp;
        Serial.println("Updated setTemp = " + String(reactorState.targetTemp));
        printSetpoints();
    }
}

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












