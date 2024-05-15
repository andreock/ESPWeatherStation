/*
 * Copyright (c) 2024 Andrea Canale.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <Adafruit_BMP085.h>
#include <Wire.h>
#include "mqtt_client.h"

#include "DHT.h"
#include "MiCS4514.hpp"
#include <ArduinoJson.h>
#include <WiFi.h>
#include "config.h"

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
MiCS4514 mics4514(PRE_PIN, NOX_PIN, RED_PIN);
esp_mqtt_client_handle_t mqtt_client;

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
      Serial.println("MQTT_EVENT_CONNECTED");
      break;
    case MQTT_EVENT_DISCONNECTED:
      Serial.println("MQTT_EVENT_DISCONNECTED");
      break;
    case MQTT_EVENT_ERROR:
      Serial.println("MQTT_EVENT_ERROR");
      break;
    default:
      break;
  }
}

void mqtt_app_start(void) {
  esp_mqtt_client_config_t mqtt_cfg{
    .uri = MQTT_BROKER_URL,
    .keepalive = 60
  };
  mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(mqtt_client, (esp_mqtt_event_id_t)-1, mqtt_event_handler, NULL);
  esp_mqtt_client_start(mqtt_client);
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Trying to Reconnect");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("DHTxx test!"));
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(500);
    Serial.print(WiFi.status());
  }
  Serial.println(WiFi.localIP());
  if (!bmp.begin()) {
    Serial.println(
      "Could not find a valid BMP085/BMP180 sensor, check wiring!");
    while (1) {
    }
  }
  mics4514.begin();
  dht.begin();
  mqtt_app_start();
}

void send_sensor_data(float temperature, float humidity, int pressure, float nox, float co) {
  JsonDocument doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["pressure"] = pressure;
  doc["nox"] = nox;
  doc["co"] = co;
  String msg = "";
  serializeJson(doc, msg);
  esp_mqtt_client_publish(mqtt_client, "sensors", msg.c_str(), 0, 1, 0);
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.printf("Humidity: %f\nTemperature:%f\n", humidity, temperature);

  int pressure = bmp.readPressure();
  Serial.printf("Pressure%i\n", pressure);

  float nox = mics4514.get_nox();
  float co = mics4514.get_co();
  Serial.printf("NOX: %f\nCO:%f\n", nox, co);
  send_sensor_data(temperature, humidity, pressure, nox, co);

  delay(60000);
}