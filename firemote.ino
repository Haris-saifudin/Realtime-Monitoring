#include <ESP8266WiFi.h>
#include "FirebaseESP8266.h"
#include <TinyGPS++.h>
#include <SoftwareSerial.h>


#define LOplus 14 //D5
#define LOmin 12 //D6
#define EmergencyButton D7
#define LED D3

#define FIREBASE_HOST "example.firebaseio.com"
#define FIREBASE_AUTH "6TcksQOYIyvkfkfksskkfskfksf"

FirebaseData firebaseData;
FirebaseJson json;
void printResult(FirebaseData &data);

//gps
TinyGPSPlus gps;
//pin TX-->D2, RX-->D1
//4=D2, 5=D1
SoftwareSerial ss(4,5);


//inisialisasi variabel global
const char* ssid ="ssid";
const char* password ="password";
float heartRate;
String pathGPS= "/1/GPS/";
String pathHR="/1/HeartRate";
String pathFlag="/1/Switch";
String Long;
float latitude =0, longitude=0;

void printResult(FirebaseData &data)
{

    if (data.dataType() == "int")
        Serial.println(data.intData());
    else if (data.dataType() == "float")
        Serial.println(data.floatData(), 5);
    else if (data.dataType() == "double")
        printf("%.9lf\n", data.doubleData());
    else if (data.dataType() == "boolean")
        Serial.println(data.boolData() == 1 ? "true" : "false");
    else if (data.dataType() == "string")
        Serial.println(data.stringData());
    else if (data.dataType() == "json")
    {
        Serial.println();
        FirebaseJson &json = data.jsonObject();
        //Print all object data
        Serial.println("Pretty printed JSON data:");
        String jsonStr;
        json.toString(jsonStr,true);
        Serial.println(jsonStr);
        Serial.println();
        Serial.println("Iterate JSON data:");
        Serial.println();
        size_t len = json.iteratorBegin();
        String key, value = "";
        int type = 0;
        for (size_t i = 0; i < len; i++)
        {
            json.iteratorGet(i, type, key, value);
            Serial.print(i);
            Serial.print(", ");
            Serial.print("Type: ");
            Serial.print(type == JSON_OBJECT ? "object" : "array");
            if (type == JSON_OBJECT)
            {
                Serial.print(", Key: ");
                Serial.print(key);
            }
            Serial.print(", Value: ");
            Serial.println(value);
        }
        json.iteratorEnd();
    }
    else if (data.dataType() == "array")
    {
        Serial.println();
        //get array data from FirebaseData using FirebaseJsonArray object
        FirebaseJsonArray &arr = data.jsonArray();
        //Print all array values
        Serial.println("Pretty printed Array:");
        String arrStr;
        arr.toString(arrStr,true);
        Serial.println(arrStr);
        Serial.println();
        Serial.println("Iterate array values:");
        Serial.println();
        for (size_t i = 0; i < arr.size(); i++)
        {
            Serial.print(i);
            Serial.print(", Value: ");

            FirebaseJsonData &jsonData = data.jsonData();
            //Get the result data from FirebaseJsonArray object
            arr.get(jsonData, i);
            if (jsonData.typeNum == JSON_BOOL)
                Serial.println(jsonData.boolValue ? "true" : "false");
            else if (jsonData.typeNum == JSON_INT)
                Serial.println(jsonData.intValue);
            else if (jsonData.typeNum == JSON_DOUBLE)
                printf("%.9lf\n", jsonData.doubleValue);
            else if (jsonData.typeNum == JSON_STRING ||
                     jsonData.typeNum == JSON_NULL ||
                     jsonData.typeNum == JSON_OBJECT ||
                     jsonData.typeNum == JSON_ARRAY)
                Serial.println(jsonData.stringValue);
        }
    }
}

void heart(){
 Serial.print("ECG :");
  if(digitalRead(LOplus)||(digitalRead(LOmin))){
    Serial.println("heart-rate filed"); 
    Firebase.setString(firebaseData, pathHR+"/Status",("not valid"));
  }
  else{
    heartRate = analogRead(A0);
    Serial.println(heartRate);
    Firebase.setString(firebaseData, pathHR+"/Status",("valid"));
    if (Firebase.setDouble(firebaseData, pathHR+"/value",(heartRate)))
    {
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.print("VALUE: ");
      printResult(firebaseData);
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILEd SEND Heart Rate");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
   
    }
  }
}

void emergency(){
  int  buttonState = digitalRead(EmergencyButton);
  Serial.print("switch:");
  Serial.println( buttonState);
  if (buttonState == HIGH){
    if (buttonState == HIGH){
      if (Firebase.setDouble(firebaseData, pathFlag,(1)))
      {
        digitalWrite(LED, HIGH);
        Serial.println("PATH: " + firebaseData.dataPath());
        Serial.print("VALUE: ");
        printResult(firebaseData);
        Serial.println("------------------------------------");
        Serial.println();
      }
      else
      {
        Serial.println("FAILEd SEND EMERGENCY ");
        Serial.println("REASON: " + firebaseData.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
      }
    }
  }
  else{
    digitalWrite(LED, LOW);
    if (Firebase.setDouble(firebaseData, pathFlag,(0)))
    {
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.print("VALUE: ");
      printResult(firebaseData);
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILEd SEND Sw");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
  }
}
void setup(){
  
  Serial.begin(9600);
  ss.begin(9600);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  Serial.println("Terhubung ke ");
  Serial.println(ssid);
  WiFi.begin(ssid,password);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  while(WiFi.status()!=WL_CONNECTED){
    delay(100);
    Serial.print(".");
  }
  
  Serial.println("WiFi Terhubung");
  Serial.println(WiFi.localIP()); 
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
  
  pinMode(LOplus,INPUT);
  pinMode(LOmin,INPUT);
  pinMode(EmergencyButton, INPUT);
 
}


void loop(){
    while (ss.available() > 0)
    if (gps.encode(ss.read()))
    {
     Serial.println("gps ok");
     if(gps.location.isValid()){
          latitude = gps.location.lat();
          longitude = gps.location.lng();
          Serial.println("#"+String(latitude,6)+"$"+String(longitude,6)); 
          if ( Firebase.setDouble(firebaseData, pathGPS+"latitude",(latitude))&&Firebase.setDouble(firebaseData, pathGPS+"longitude",(longitude)))
          {
            Serial.println("Send Longitude & Latitude ");
            Serial.println("PATH: " + firebaseData.dataPath());
            Serial.println("TYPE: " + firebaseData.dataType());
            Serial.println("------------------------------------");
            Serial.println();
          }
          else
          {
            Serial.println("FAILEd SEND LONGITUDE & LATITUDE");
            Serial.println("REASON: " + firebaseData.errorReason());
            Serial.println("------------------------------------");
            Serial.println();
          }
          delay(5);  
          heart();
          delay(5);
          emergency();
      }  
   }  
}
