#include "Ticker.h"
#include <PubSubClient.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <ArduinoOTA.h>
#include "credentials.h"

#define DHT_PIN    2       // Digital pin connected to the DHT sensor
#define MOTOR_PIN1 8
#define MOTOR_PIN2 9
#define PUMP_PIN   10

const int SATURATED         =  9;
const int ADEQUATELY_WET    = 19;
const int IRRIGATION_ADVICE = 59;
const int IRRIGATION        = 99;

void sendData();
void checkRoof();
void checkPump();
    
Ticker sendDataTicker(sendData,60000);
Ticker checkRoofTicker(checkRoof,5 * 60000);
Ticker checkPumpTicker(checkPump,5 * 60000);

char buffer[512];

boolean roofOpen = false;

const char* place             = "GREENHOUSE";
const char* typeTemperature   = "TEMPERATURE";
const char* typeHumidity      = "HUMIDITY";
const char* typeMoisture      = "MOISTURE";
const char* temperatureTopic  = "sensordata/greenhouse/temperature";
const char* humidityTopic     = "sensordata/greenhouse/humidity";
const char* moistureTopic     = "sensordata/greenhouse/moisture";
const char* commandTopic      = "sensordata/greenhouse/command";
const char* statusTopic       = "sensordata/greenhouse/status";

WiFiClient   wificlient;
PubSubClient client(wificlient);

DHT              dht(DHT_PIN, DHT22);


void pump(boolean on) {
    digitalWrite(PUMP_PIN, on?HIGH:LOW);
    if (checkConnection()) {
        StaticJsonDocument<200> doc;

        doc["device"] = "PUMP";
        doc["data"]["status"] = on?"ON":"OFF";

        serializeJson(doc, buffer);
        client.publish(statusTopic, buffer);
    }   
}

// Callback function
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload, length);

    const char* command = doc["command"];
    if (!command) {
        return;
    }
    Serial.print("command:   "); Serial.println(command);    
    if (strcmp("PUMP",command) == 0) {
        const char* pumpState = doc["data"]["state"];
        if (pumpState) {
            if (strcmp("ON",pumpState) == 0) {
                pump(true);
            } else {
                pump(false);
            }
        }
    } else if (strcmp("ROOF",command) == 0) {
        const char* roofState = doc["data"]["state"];
        if (roofState) {
            if (strcmp("OPEN",roofState) == 0) {
                openRoof();
            } else {
                closeRoof();
            }
        }
    }

}


boolean checkConnection() {
    
    if (WiFi.status() != WL_CONNECTED) {
        wifiConnect();
    }
    return mqttConnect();
}
/**************************************************************************/
/*
    Try to connect to the MQTT broker, after 10 tries, we return false
*/
/**************************************************************************/
boolean mqttConnect(void) {

    if (client.connected()) {
        return true;
    }

    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }
    
    client.setServer(mqttServer, 1883);
    client.setCallback(mqttCallback);
    
    int  tries = 10;
    while (!client.connected()) {
        tries--;
        if (tries == 0) {
            return false;
        }
        if (!client.connect(place, mqttUser, mqttPassword)) {
            delay(5000);
        } else {
            client.subscribe(commandTopic);
        }
    }
    return true;
}

/**************************************************************************/
/*
    Try to connect to the WIFI
*/
/**************************************************************************/
boolean wifiConnect(void) {
  WiFi.begin(ssid, password);
  // Wait for connection
  int tries = 10;
  boolean retVal = true;
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    tries--;
    if (tries <= 0) {
      // got no connection to WiFi, going to sleep
      return false;
    }
  }
  return retVal;
}

void sendMoistureData() {
    Serial.print("sending moisture data...");
    int moistureSensor1 = 100 - getMoistureSensor1();
    int moistureSensor2 = 100 - getMoistureSensor2();

    StaticJsonDocument<200> doc;

    doc["place"]  = place;    
    doc["sensor"] = "Moisture Sensor1";
    doc["type"]   = typeMoisture;
    doc["moisture"] =  moistureSensor1;

    serializeJson(doc, buffer);
    client.publish(moistureTopic, buffer);

    doc["sensor"] = "Moisture Sensor2";
    doc["moisture"] =  moistureSensor2;

    serializeJson(doc, buffer);
    client.publish(moistureTopic, buffer);
}

void sendDHT22Data() {

    float humidity    = dht.readHumidity();
    float temperature = dht.readTemperature();
    
    StaticJsonDocument<200> doc;
    
    if (isnan(humidity) || isnan(temperature)) {
        doc["error"] = "DHT22 read error";
        serializeJson(doc, buffer);
        client.publish("sensordata/test/status", buffer);
    } else {

        doc["place"] = place;
        doc["sensor"] = "DHT22";
        doc["type"] = typeTemperature;
        doc["temperature"] =  temperature;  

        serializeJson(doc, buffer);
        client.publish(temperatureTopic, buffer);
    
        doc.remove("temperature");
    
        doc["type"] = typeHumidity;
        doc["humidity"] = humidity;
        serializeJson(doc, buffer);
        client.publish(humidityTopic, buffer);
    }
}

void sendData() {
    Serial.print("sending data...");
    if (checkConnection()) {
        sendDHT22Data();
        client.loop();
        sendMoistureData();
        Serial.println("done.");
    } else {
        Serial.println("error.");
    }    
}


void checkRoof()  {
    float temperature = dht.readTemperature();
    if (isnan(temperature)) {
        return;
    }
    if (temperature < 25.0) {
        closeRoof();
    }

    if (temperature > 30.0) {
        openRoof();
    }
}

void checkPump()  {
    
    int moistureSensor1 = getMoistureSensor1();
    int moistureSensor2 = getMoistureSensor2();

    if (IRRIGATION_ADVICE <= max(moistureSensor1,moistureSensor2)) {
        pump(true);
        return;
    }
    
    if (ADEQUATELY_WET >= min(moistureSensor1,moistureSensor2)) {
        pump(false);
        return;
    }
}

void setup() {

    pinMode(PUMP_PIN, OUTPUT);     // pump pin to output
    digitalWrite(PUMP_PIN, LOW);   // pump off

    pinMode(MOTOR_PIN1, OUTPUT);
    pinMode(MOTOR_PIN2, OUTPUT);

    digitalWrite(MOTOR_PIN1, LOW);   // motor off
    digitalWrite(MOTOR_PIN2, LOW);   // motor off
    
    //Initialize serial and wait for port to open:
    Serial.begin(9600);
    
    dht.begin();
 
    wifiConnect();

    Serial.println("Connected to wifi");

    ArduinoOTA.begin(WiFi.localIP(), "Greenhouse", "greenhouse", InternalStorage);
    
    sendDataTicker.start();
    checkRoofTicker.start();
    checkPumpTicker.start();
    
    sendData();
    pump(false);
}

void loop() {

    client.loop();

    ArduinoOTA.poll();
   
    sendDataTicker.update();
    checkRoofTicker.update();
    checkPumpTicker.update();
}
