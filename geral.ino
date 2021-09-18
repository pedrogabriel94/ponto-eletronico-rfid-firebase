
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

//-------------------------------------------------------------

constexpr uint8_t RST_PIN = D3;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D4;     // Configurable, see typical pin layout above
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

#define FIREBASE_HOST "URL_BANCO_FIREBASE"   
#define FIREBASE_AUTH "TOKEN_FIREBASE"   

#define WIFI_SSID "NOME_WIFI"
#define WIFI_PASSWORD "SENHA_WIFI"

String tag; 

//---------------NTP---------------------------
const long utcOffsetInSeconds = -10800;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
//---------------------------------------------

void setup() {
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  delay(1000);  
  pinMode(4, OUTPUT); 
  pinMode(5, OUTPUT); 
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                  
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); // connect to firebase
  timeClient.begin();

}

void loop() {
  timeClient.update();
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  if (rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }
    if(tag == "16096122122" || tag == "48136129122"){
      StaticJsonBuffer<256> jsonBuffer;
      JsonArray& array = jsonBuffer.createArray();      
      array.add(tag);
      array.add(timeClient.getEpochTime());
      Firebase.push("autorizados", array);
      digitalWrite(4, HIGH); 
      delay(2000);              
      digitalWrite(4, LOW);
    }else{
      StaticJsonBuffer<256> jsonBuffer;
      JsonArray& array = jsonBuffer.createArray();
      array.add(tag);
      array.add(timeClient.getEpochTime());
      Firebase.push("nao_autorizados", array);
      digitalWrite(5, HIGH); 
      delay(2000);              
      digitalWrite(5, LOW);
    }
    tag = "";
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

}
