#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

//COMPLETAR
#define RST_PIN PREENCHER               //PINO DE RESET
#define SS_PIN PREENCHER                //PINO SDA aka SERIAL DATA

#include "MFRC522.h" //https://www.instructables.com/id/WiFi-RFID-Reader/
MFRC522 rfid(SS_PIN, RST_PIN);          //ATRIBUINDO A VARIÁVEL RFID COMO REF A BIBLIOTECA E PASSANDO PARÂMETROS
  
//Colocar o nome do wifi e senha
const char* ssid     = "wifi-user";
const char* password = "wifi-pswd";

// Colocar o URL do servidor
const char* serverName = "http://localhost/arduino-aic/tag-id";

// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key 
String apiKeyValue = "b79e0ad612c6b94c2f956036a149e5c0";

// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void setup() {
  Serial.begin(9600);
  SPI.begin();                        //INICIALIZA O BARRAMENTO SPI
  rfid.PCD_Init();                    //INICIALIZA MFRC522
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    //tagID type ???
    String tagID = "";

    // Your Domain name with URL path or IP address with path
    http.begin(serverName);
    
    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    //Algo para ler a tagID https://randomnerdtutorials.com/security-access-using-mfrc522-rfid-reader-with-arduino/
    if(mfrc522.PICC_ReadCardSerial()){
      for (byte i = 0; i < mfrc522.uid.uidByte[i]; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
        tagID.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
        tagID.concat(String(rfid.uid.uidByte[i], HEX));

/***  for (byte i = 0; i < 4; i++) {
        tagID +=
        (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
        String(rfid.uid.uidByte[i], HEX) +
        (i!=3 ? ":" : "");
      }
      tagID.toUpperCase();*/
      }
    }
    else{
      tagID = "";
      return 0;
    }

    // Prepare your HTTP POST request data
    String httpRequestData = "api_key=" + apiKeyValue + "&tagID=" + tagID;
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
    
    // You can comment the httpRequestData variable above
    // then, use the httpRequestData variable below (for testing purposes without the BME280 sensor)
    //String httpRequestData = "api_key=tPmAT5Ab3j7F9&sensor=BME280&location=Office&value1=24.75&value2=49.54&value3=1005.14";

    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);
     
    // If you need an HTTP request with a content type: text/plain
    //http.addHeader("Content-Type", "text/plain");
    //int httpResponseCode = http.POST("Hello, World!");
    
    // If you need an HTTP request with a content type: application/json, use the following:
    //http.addHeader("Content-Type", "application/json");
    //int httpResponseCode = http.POST("{\"value1\":\"19\",\"value2\":\"67\",\"value3\":\"78\"}");
        
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
    rfid.PICC_HaltA(); //PARADA DA LEITURA DO CARTÃO
    rfid.PCD_StopCrypto1(); //PARADA DA CRIPTOGRAFIA NO PCD
  }
  else {
    Serial.println("WiFi Disconnected");
  }
  //Send an HTTP POST request every 1 second
  delay(1000);  
}