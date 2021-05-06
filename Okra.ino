#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>
#include <Wire.h>
#include <SFE_BMP180.h> 
#include <HTTPClient.h>

#include <SoftwareSerial.h> 
SoftwareSerial gsm(16,17);

int count;
char input[12];

SFE_BMP180 bmp180;

//firebase
#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>
//change
// Set these to run example.
#define FIREBASE_HOST "leaf-disease-f05af-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "qIgP8rlWrp8dB0kzGRzzK1snS947qOH8gpIifjTl"
#define WIFI_SSID "sunitha"
#define  WIFI_PASSWORD "sunitha07n"

#include "DHTesp.h"
#include "Ticker.h"

#ifndef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP32 ONLY!)
#error Select ESP32 board.
#endif


//https://script.google.com/macros/s/AKfycbyq6sRzrNgL8D-efYM0sljFGJTC7y4ekipSQlIpwzyy4hGDrYVc1YceJw/exec
//String GOOGLE_SCRIPT_ID = "AKfycbyq6sRzrNgL8D-efYM0sljFGJTC7y4ekipSQlIpwzyy4hGDrYVc1YceJw";
String GOOGLE_SCRIPT_ID = "AKfycbyuWaffV5jYEuNC_BuPkkiMmNRiKX3joEcLwrndF-QAsHy5-pDJY1SQ"; // Replace by your GAS service id
const int sendInterval = 996 *5; // in millis, 996 instead of 1000 is adjustment, with 1000 it jumps ahead a minute every 3-4 hours

const char * root_ca=\
"-----BEGIN CERTIFICATE-----\n" \
"MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n" \
"A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n" \
"Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n" \
"MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n" \
"A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n" \
"hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n" \
"v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n" \
"eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n" \
"tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n" \
"C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n" \
"zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n" \
"mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n" \
"V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n" \
"bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n" \
"3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n" \
"J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n" \
"291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n" \
"ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n" \
"AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n" \
"TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n" \
"-----END CERTIFICATE-----\n";
WiFiClientSecure client;

int pir=23;
int ir=19;
int ldr=18;
int relay=2;
double T, P;
int gasvalue;
int soilvalue;
int rainvalue;
int pirvalue;
int irvalue;
int ldrvalue;

DHTesp dht;

void tempTask(void *pvParameters);
bool getTemperature();
void triggerGetTemp();

/** Task handle for the light value read task */
TaskHandle_t tempTaskHandle = NULL;
/** Ticker for temperature reading */
Ticker tempTicker;
/** Comfort profile */
ComfortState cf;
/** Flag if task should run */
bool tasksEnabled = false;
/** Pin number for DHT11 data pin */
int dhtPin = 15;
float Tempi,Humii;
/**
 * initTemp
 * Setup DHT library
 * Setup task and timer for repeated measurement
 * @return bool
 *    true if task and timer are started
 *    false if task or timer couldn't be started
 */
bool initTemp() {
  byte resultValue = 0;
  // Initialize temperature sensor
  dht.setup(dhtPin, DHTesp::DHT11);
  Serial.println("DHT initiated");

  // Start task to get temperature
  xTaskCreatePinnedToCore(
      tempTask,                       /* Function to implement the task */
      "tempTask ",                    /* Name of the task */
      4000,                           /* Stack size in words */
      NULL,                           /* Task input parameter */
      5,                              /* Priority of the task */
      &tempTaskHandle,                /* Task handle. */
      1);                             /* Core where the task should run */

  if (tempTaskHandle == NULL) {
    Serial.println("Failed to start task for temperature update");
    return false;
  } else {
    // Start update of environment data every 20 seconds
    tempTicker.attach(20, triggerGetTemp);
  }
  return true;
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker getTempTimer
 */
void triggerGetTemp() {
  if (tempTaskHandle != NULL) {
     xTaskResumeFromISR(tempTaskHandle);
  }
}

/**
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *    pointer to task parameters
 */
void tempTask(void *pvParameters) {
  Serial.println("tempTask loop started");
  while (1) // tempTask loop
  {
    if (tasksEnabled) {
      // Get temperature values
      getTemperature();
    }
    // Got sleep again
    vTaskSuspend(NULL);
  }
}

bool getTemperature() {
  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity newValues = dht.getTempAndHumidity();
 
  // Check if any reads failed and exit early (to try again).
  if (dht.getStatus() != 0) {
    Serial.println("DHT11 error status: " + String(dht.getStatusString()));
    return false;
  }
  float heatIndex = dht.computeHeatIndex(newValues.temperature, newValues.humidity);
  float dewPoint = dht.computeDewPoint(newValues.temperature, newValues.humidity);
  float cr = dht.getComfortRatio(cf, newValues.temperature, newValues.humidity);
  Serial.println();
  //Serial.println(" Temperature:" + String(newValues.temperature) + " Humidity:" + String(newValues.humidity) + " I:" + String(heatIndex)); //+ " D:" + String(dewPoint) + " " + comfortStatus);
  Serial.print("Temperature:");
  Serial.println(String(newValues.temperature));  
  Serial.print("Humidity:");
  Serial.println(String(newValues.humidity));   
  Tempi=newValues.temperature;
  Humii=newValues.humidity;  
  return true;
}



void setup() 
{
  
  
  Serial.begin(115200);  
  gsm.begin(9600);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
  Serial.print(".");
  delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setString("Test", "Data updated");
  initTemp();
  WiFi.mode(WIFI_STA);
  bool success = bmp180.begin();
  if (success) {
    //Serial.println("BMP180 init success");
  }
  
  // Signal end of setup() to tasks
  tasksEnabled = true;
  pinMode(pir, INPUT);
  pinMode(ir, INPUT); 
  pinMode(ldr, INPUT);
  pinMode(relay, OUTPUT);


}
 void SendMessage1()
{
  gsm.println("AT+CMGF=1"); 
  delay(1000);  
  gsm.println("AT+CMGS=\"+919030701443\"\r"); // number change
  delay(1000);
//  gsm.println(e);
// delay(100);
 gsm.print("motion detected");
 gsm.println((char)26);
  delay(1000);
}
void SendMessage2()
{
  gsm.println("AT+CMGF=1"); 
  delay(1000);  
  gsm.println("AT+CMGS=\"+919030701443\"\r"); // number change
  delay(1000);
//  gsm.println(e);
// delay(100);
 gsm.print("moisture level is low");
 gsm.println((char)26);
  delay(1000);
}
void SendMessage3()
{
  gsm.println("AT+CMGF=1"); 
  delay(1000);  
  gsm.println("AT+CMGS=\"+919030701443\"\r"); // number change
  delay(1000);
//  gsm.println(e);
// delay(100);
 gsm.print("motor is on");
 gsm.println((char)26);
  delay(1000);
}
void SendMessage4()
{
  gsm.println("AT+CMGF=1"); 
  delay(1000);  
  gsm.println("AT+CMGS=\"+919606146401\"\r"); // number change
  delay(1000);
//  gsm.println(e);
// delay(100);
 gsm.print("motion detected");
 gsm.println((char)26);
  delay(1000);
}
void SendMessage5()
{
  gsm.println("AT+CMGF=1"); 
  delay(1000);  
  gsm.println("AT+CMGS=\"+919606146401\"\r"); // number change
  delay(1000);
//  gsm.println(e);
// delay(100);
 gsm.print("moisture level is low");
 gsm.println((char)26);
  delay(1000);
}
void SendMessage6()
{
  gsm.println("AT+CMGF=1"); 
  delay(1000);  
  gsm.println("AT+CMGS=\"+919606146401\"\r"); // number change
  delay(1000);
//  gsm.println(e);
// delay(100);
 gsm.print("motor is on");
 gsm.println((char)26);
  delay(1000);
}
void smokesensor()
{
  gasvalue=analogRead(34);
  Serial.print("Smoke value:");
  Serial.println(gasvalue);
  Firebase.setFloat("Smoke",gasvalue);
  if(gasvalue>1200)
  {
    Serial.print("Smoke Detected");
    Firebase.setString("Smoke_Alert", "Smoke_Detected");
  }
  else
  {
    Serial.print("Smoke Not Detected");
    Firebase.setString("Smoke_Alert", "Smoke_Not_Detected");
  }
  Serial.println();
}

void pirsensor()
{
  pirvalue=digitalRead(pir);
  Serial.print("Pir value:");
  Serial.println(pirvalue);  
  Firebase.setFloat("pir",pirvalue);
  if(pirvalue==1)
  {
    Serial.print("Motion Detected");
    SendMessage1();
    SendMessage4();
    Firebase.setString("Pir_Alert", "Motion_Detected");
  }
  else
  {
    Serial.print("Motion Not Detected");
    Firebase.setString("Pir_Alert", "Motion_Not_Detected");
  }
  Serial.println();
}

void soilsensor()
{
  soilvalue=analogRead(35);
  Serial.print("Soil value:");
  
  Serial.println(soilvalue);
  Firebase.setFloat("Soil",soilvalue);
  if(soilvalue>3000)
  {
    Serial.print("Normal Detected");
    SendMessage2();
    SendMessage5();
    Firebase.setString("Moisture_Alert", "Normal_Water_Detected");
    digitalWrite(relay, HIGH);
    SendMessage3();
    SendMessage6();
    delay(2000);
    digitalWrite(relay, LOW);
  }
  else
  {
    
    Serial.print("High Water Detected");
    Firebase.setString("Moisture_Alert", "High_Water_Detected");
    digitalWrite(relay, LOW);
  }
  Serial.println();
}

void rainsensor()
{
  rainvalue=analogRead(32);
  Serial.print("Rain value:");
  Serial.println(rainvalue);
  Firebase.setFloat("Rain",rainvalue);
  if(rainvalue<1700)
  {
    Serial.print("Rain Detected");
    Firebase.setString("Rain_Alert", "Rain_Detected");
  }
  else
  {
    Serial.print("Rain Not Detected");
    Firebase.setString("Rain_Alert", "Rain_Not_Detected");
  }
  Serial.println();
}

void irsensor()
{
  irvalue=digitalRead(ir);
  Serial.print("Ir value:");
  Serial.println(irvalue);  
  Firebase.setFloat("Ir",irvalue);
  if(irvalue==0)
  {
    Serial.print("Object Detected");

    Firebase.setString("Ir_Alert", "Object_Detected");
  }
  else
  {
    Serial.print("Object Not Detected");
    Firebase.setString("Ir_Alert", "Object_Not_Detected");
  }
  Serial.println();  
}

void ldrsensor()
{
  ldrvalue=digitalRead(ldr);
  Serial.print("LDR value:");
  Serial.println(ldrvalue);  
  Firebase.setFloat("LDR",ldrvalue);
  if(ldrvalue==1)
  {
    Serial.print("Dark Detected");
    Firebase.setString("LDR_Alert", "Dark_Detected");
  }
  else
  {
    Serial.print("Light Detected");
    Firebase.setString("LDR_Alert", "Light_Detected");
  }
  Serial.println();  
}

void pressure() {

  char status;  
  bool success = false;

  status = bmp180.startTemperature();


    if (status != 0) {
      status = bmp180.startPressure(3);

      if (status != 0) {
        delay(status);
        status = bmp180.getPressure(P, T);

        if (status != 0) {
          Serial.print("Pressure: ");
          Serial.print(P);
          Serial.println(" hPa");
          Firebase.setFloat("Pressure",P);
        }
      }
    }
}

void SendAlarm()//use this function to notify if something wrong (example sensor says -128C)
// don't forget to set true for enableSendingEmails in google script
{
   sendData("alarm=fixme"); 
}

void sendData(String params) {
   HTTPClient http;
   String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+params;
   //Serial.print(url);
    //Serial.print("Making a request");
    http.begin(url, root_ca); //Specify the URL and certificate
    int httpCode = http.GET();  
    http.end();
    //Serial.println(": done "+httpCode);
}

void loop() 
{
      if(Serial.available()>0)
   {
    count=0;
    while(Serial.available() && count < 12)
    {
      input[count]= Serial.read();
       count++;
       delay(5);    }

   Serial.println(input);
   Firebase.setString("leaf disesse alert",input); 
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
   
//
  // String va=String(input);
    
      }



  
  temperature();
  Firebase.setFloat("Temperature",Tempi);
  Firebase.setFloat("Humidity",Humii); 
  
  smokesensor();
  pirsensor();
  soilsensor();
  rainsensor();
  irsensor();
  ldrsensor();
  pressure();
  delay(1000);
  sendData("Temperature=" + String(Tempi) +"&Humidity="+String(Humii)+"&Pressure="+String(P)+"&Smoke_Value="+String(gasvalue)
  +"&Soil_Moisture="+String(soilvalue)+"&Rain_fall="+String(rainvalue)+"&PIR="+String(pirvalue)+
  "&IR="+String(irvalue)+"&LDR="+String(ldrvalue));  
  delay(1000);
}


void temperature() {
  if (!tasksEnabled) {
    // Wait 2 seconds to let system settle down
    delay(2000);
    // Enable task that will read values from the DHT sensor
    tasksEnabled = true;
    if (tempTaskHandle != NULL) {
      vTaskResume(tempTaskHandle);
    }   
  }    
  yield();
}
