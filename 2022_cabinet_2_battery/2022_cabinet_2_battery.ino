// cabinet for 2 battery slot

#include "ESP8266WiFi.h"
#include "PubSubClient.h"
#include "PZEM004Tv30.h"

#define Relay1 2          //unlock1 D4        // command to "relay"
#define Relay2 4          //unlock2 D2        // command to "relay"
#define Trig1 5           //TrigLock1 D1      RX // auxaraly contect Magnetic "SW - GND"
#define Trig2 0           //TrigLock2 D3      // auxaraly contect Magnetic "SW - GND"
//#define Test 15         //Start button
//ไมใช้ #define Trig1 3    //TrigLock1 RX      // auxaraly contect Magnetic "SW - GND"
//ไมใช้ #define Trig2 15   //TrigLock2 D8      // auxaraly contect Magnetic "SW - GND"
//ไมใช้ #define Test RX    //Start button      // Local "SW - GND"

//const char* ssid = "iHubzZ_2.4G";
//const char* password = "Sleep1ess";
const char* ssid = "AIS 4G Hi-Speed Home WiFi_444913";
const char* password = "50444913";
const char* server = "driver.cloudmqtt.com";                    //สำหรับระบุ server
const int   port = 18855;                                       //สำหรับระบุ port
const char* mqtt_user = "TEST12";                                
const char* mqtt_password = "12345";
//const char* UID = "0b39e953-06e2-4c82-a204-27dc26978168";     //สำหรับระบุตัวตน
//const char* Identify = "Yw9FbvV7JH13tGAyr5mnNr8K9ovrY4PX";    //สำหรับระบุตัวตน
//const char* Scode = "$GD5h)m0vi4PxB-dm6au7h(dZEYPmAU*";       //สำหรับระบุตัวตน  

unsigned long previousMillis = 3000;         // will store last time LED was updated
const long interval = 600000;               // interval at which to blink (milliseconds)



char msg[100];
int unlock1 = 0;   
int unlock2 = 0;
int unlock3 = 0;   
int unlock4 = 0;
int process = 0;
float bipcurrent1 = 0.08;     // For check current connection with battery A
float bipcurrent2 = 0.08;      // unconnect plug

PZEM004Tv30 pzem1(D5, D6);    // (D3)=RX , (D5) = TX   // connect to "TX , RX"
PZEM004Tv30 pzem2(D7, D8);    // (D6)=RX , (D7) = TX   // connect to "TX , RX"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {

  Serial.begin(9600);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.softAPdisconnect(true);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(server, port);
  client.setCallback(callback);

  pinMode(Trig1,INPUT_PULLUP);  // Lock1 Trig1=Hight close door (กดปุ่มเป็น LOW)
  pinMode(Trig2,INPUT_PULLUP);  // Lock2 Trig2=Hight close door (กดปุ่มเป็น LOW)
  pinMode(Relay1,OUTPUT); // setup output
  pinMode(Relay2,OUTPUT); // setup output
  digitalWrite(Relay1,HIGH); // สั่งรอไว้
  digitalWrite(Relay2,HIGH); // สั่งรอไว้

}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password )) {//(UID, Identify, Scode)
      Serial.println("Server Connected");
      client.subscribe("msg/swapper");
    }
    else {
      Serial.println("Try again in 5 s");
      delay(5000);
    }
  }
}

void callback(char* topic,byte* payload, unsigned int length) {
 
  String msg;
  for (int i = 0; i < length; i++) {
    msg = msg + (char)payload[i];
  }
  if (String(topic) == "msg/swapper") {    //  in topic 
    if (msg == "unlock"){                  //  msg command to unlock
      if (process == 0){
        Serial.println("Cabinet1 Start");
        Serial.println("in Process");
        Serial.println(process);
        unlock1 =1;
        cabinetlock1(unlock1);
      }
    }
    delay(1000);  
  }
}

void cabinetlock1(int unlock1) {
  if (unlock1 == 1){
    Serial.println("Relay unlock1! process1");
    digitalWrite(Relay1,LOW);                     // relay active LOW  ===  form  open >>> to closed 
    digitalWrite(Relay2,LOW);
    delay(500);
    process =1;                                   // status Lock1=LOW  Process=1
    digitalWrite(Relay1,HIGH);                    // relay === form close >>> to opened 
    digitalWrite(Relay2,HIGH);
  } 
  else if (unlock1 == 0) {
    Serial.println("Lock1!");
    digitalWrite(Relay1,HIGH);
    digitalWrite(Relay2,HIGH);
    delay(500);
  }  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long currentMillis = millis();

  bool ReadTriger1 = digitalRead(Trig1);
  bool ReadTriger2 = digitalRead(Trig2);
  if(ReadTriger1 == LOW && ReadTriger2 == LOW && process == 1){                    // when closed Locker 1 && process == 1
        unlock1 = 0;                                                               // confirm lock
        cabinetlock1(unlock1);
        Serial.println("in Process comfirm lock");                                 
        Serial.println(process);
           delay(1000);
           
        float current = pzem1.current();
        Serial.print("Current: "); Serial.print(current); Serial.println("A");    // 
        
      if (current >= bipcurrent1){                                                // after check Bip complete to step2 flow
        Serial.println("Cabinetlock1 Close complete");
        Serial.println("Done 1");
          delay(1000);
          process =0;
        if (currentMillis - previousMillis >= interval) {
       // save the last time you blinked the LED
          previousMillis = currentMillis;
          ESP.restart();
        } 
      }
      if(current < bipcurrent1 || current == NAN){                               // after check Bip "NOT" complete retry again 
        Serial.println("Plug1 is not connect");
        unlock1 =1;
        cabinetlock1(unlock1);
          delay(1000);
        Serial.println("Please connect plug1");
        if (currentMillis - previousMillis >= interval) {
       // save the last time you blinked the LED
          previousMillis = currentMillis;
          ESP.restart();
        }  
      }
      delay(1000);
    }
}
