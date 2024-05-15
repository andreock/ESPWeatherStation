# ESPWeatherStation

A simple weather station made with ESP32 using arduino-esp32, InfluxDB, Telegram, Mosquitto and Grafana. All orchestrated by docker-compose

## Installation

Make sure to have docker and Arduino IDE installed.

### Flash ESP32

In the Arduino IDE install DHT11, ArduinoJSON, Adafruit_BMP085 and [MiCS4514](https://github.com/andreock/MiCS4514-Arduino) libraries then flash the code. Make sure to change SSID,password and MQTT URL in config.h. You can also change pin definition.

Now we can setup our server:

### Configure InfluxDB

```bash
docker-compose up -d
```

Go at http://[URL]:8086 and complete configuration. Rembember the bucket and the organization name.

Create an API token in InfluxDB and put it under telegraf config at server/telegraf/telegraf.conf line 128(influxdb_v2 token) then put also bucket and organization name(line 129 and 130).

Now restart all the containers with

```bash
docker-compose restart
```

### Configure Grafana

Go at http://[URL]:3000 and create a new data source for our dashboard. Go under connection then data source and add a InfluxDB data source. 

- Select Flux as query language.
- Use http://infludb:8086 as URL(it uses DNS resolution of docker to retrieve the IP)
- Put your username and password for the basic auth
- Put your Organization, token and Bucket
- Save and test. All should be fine now

Now go in the dashboard panel and under the new button select Import and import the JSON of the dashboard that you can find under server folder and select the InfluxDB data source.

![Dashboard screenshot](https://github.com/andreock/ESPWeatherStation/blob/main/screenshot/dashboard.png?raw=true)