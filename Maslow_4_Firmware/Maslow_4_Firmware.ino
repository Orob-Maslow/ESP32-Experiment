#include <WiFi.h>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer
#include "AS5048A.h"
#include "html.h"
#include "ESP32MotorControl.h" 	// https://github.com/JoaoLopesF/ESP32MotorControl
#include "MiniPID.h" //https://github.com/tekdemo/MiniPID
#include "Adafruit_TLC59711.h" //https://github.com/adafruit/Adafruit_TLC59711
#include "comands.h"

#define NUM_TLC59711 1

#define tlcData   5
#define tlcClock  21

const char* ssid = "Barbour";
const char* password = "hammockreader";
 
AsyncWebServer server(80);

AS5048A angleSensor(17);

float RotationAngle = 0.0;
float AngleCurrent  = 0.0;
float AnglePrevious = 0.0;
float errorDist = 0.0;

const int MotorIn1 = 12;
const int MotorIn2 = 14;

MiniPID pid = MiniPID(1000,.1,0);

Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711, tlcClock, tlcData);

void setup(){
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());
  
  angleSensor.init();
 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", HTML);
  });
  
  server.on("/settarget", HTTP_POST, [](AsyncWebServerRequest *request){
 
    int paramsNr = request->params();
    Serial.println(paramsNr);
 
    for(int i=0;i<paramsNr;i++){
 
        AsyncWebParameter* p = request->getParam(i);
        parseCommand(p->name(), p->value());
    }
 
    request->redirect("/");
  });
  
  server.on("/position", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(RotationAngle/360, 5).c_str());
  });
  
  server.on("/target", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(setPoint, 5).c_str());
  });
  
  server.on("/errorDist", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(errorDist, 5).c_str());
  });

 
  server.begin();
  
  MotorControl.attachMotor(MotorIn1, MotorIn2);
  motorStop();
  
  pid.setOutputLimits(-255,255);
  
  tlc.begin();
  //tlc.write();

}
 
void loop(){
    
    //Find the position
    AngleCurrent = angleSensor.RotationRawToAngle(angleSensor.getRawRotation());
    angleSensor.AbsoluteAngleRotation(&RotationAngle, &AngleCurrent, &AnglePrevious);
    errorDist = setPoint - (RotationAngle/360.0);
    
    //Set the speed of the motor
    motorSpeed(int(pid.getOutput(RotationAngle/360.0,setPoint)));
    
    //Print it out
    //Serial.println(int(pid.getOutput(RotationAngle/360.0,setPoint)));
    Serial.println(AngleCurrent);
    
    tlc.setPWM(1,65535);
    // tlc.setPWM(2,65535);
    // tlc.setPWM(3,65535);
    // tlc.setPWM(4,65535);
    // tlc.setPWM(5,65535);
    // tlc.setPWM(6,65535);
    // tlc.setPWM(8,65535);
    // tlc.setPWM(9,65535);
    // tlc.setPWM(10,65535);
    // tlc.setPWM(11,65535);
    // tlc.setPWM(12,65535);
    // tlc.write();
    
}