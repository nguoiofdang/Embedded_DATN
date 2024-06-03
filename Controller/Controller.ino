#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Arduino_JSON.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// Thong tin cua wifi muon truy cap
#define WIFI_SSID "Tang 2"
#define WIFI_PASSWORD "12345678"

// Firebase project API Key
#define API_KEY ""

// RTDB URLefine the DTSB URL
#define DATABASE_URL ""

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

long timeSend = 0;
bool signupOk = false;
bool control = false;
String _confirm = "confirm";

typedef struct objectControl
{
  String address;
  bool control;
} objectControl;

objectControl objControl;

void arrayToHex(uint8_t *mac_addr, String *stringID)
{
  char hexID[18];

  sprintf(hexID, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac_addr[0],mac_addr[1],mac_addr[2],
          mac_addr[3],mac_addr[4],mac_addr[5]);
  *stringID = String(hexID);
}

void hexToArray(uint8_t *mac_addr, String hexID)
{
  sscanf(hexID.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
          mac_addr[0],mac_addr[1],mac_addr[2],
          mac_addr[3],mac_addr[4],mac_addr[5]);
  *stringID = String(hexID);
}

String feedback(String address, String node)
{
  return "Remote/" + address + "/" + node;
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{

  String address;
  arrayToHex(mac_addr, &address)

  if (sendStatus == 0)
  {
    Serial.printf("\nSend success");
    String path = feedback(objControl.address, _confirm);
    Firebase.RTDB.setBool(&fbdo, path, objControl.control);
  } 
  else 
  {
    Serial.printf("Send fail\n");
  }

}

void streamCallback(FirebaseStream data)
{

  const size_t capacity = JSON_OBJECT_SIZE(2) + 30;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, data.jsonString());

  if (doc["address"] == "")
  {
    return;
  }

  objControl.control = doc["control"];
  objControl.address = String(doc["address"]);

  control = true;

}

void setup() {
  Serial.begin(115200)
  Wifi.mode(WIFI_STA);
  Wifi.begin(WIFI_SSID, WIFI_PASSWORD);

  while(Wifi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  config.api_ley = API_KEY;
  config.database_url = DATABASE_URL
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("Firebase ok");
    signupOK = true
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWifi(true);
  Firebase.RTDB.beginStream(&fbdo, "/Control");
  Firebase.RTDB.setStreamCallback(&fbdo, streamCallback, streamTimeoutCalback);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
}

void loop() {
  // put your main code here, to run repeatedly:

}
