// For 1 battery
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
float bipcurrent1 = 0.1;      // For check current connection with battery A
float bipcurrent2 = 0.1;      // unconnect plug

PZEM004Tv30 pzem1(D5, D6);    // (D3)=RX , (D5) = TX   // connect to "TX , RX"
PZEM004Tv30 pzem2(D7, D8);    // (D6)=RX , (D7) = TX   // connect to "TX , RX"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {                                    // สำหรับเซ็ทค่าตั้งต้น

  Serial.begin(9600);                             // เลือกช่องแสดงค่า Serial
  Serial.print("Connecting to ");
  Serial.println(ssid);                           // ชุดโค้ดสำหรับแสดงการเชื่อมต่อ WIFI
  WiFi.softAPdisconnect(true);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(server, port);                 // ชุดโค้ดสำหรับเชื่อมต่อ Server
  client.setCallback(callback);

  pinMode(Trig1,INPUT_PULLUP);  // Lock1 Trig1=Hight close door (กดปุ่มเป็น LOW)
  pinMode(Trig2,INPUT_PULLUP);  // Lock2 Trig2=Hight close door (กดปุ่มเป็น LOW)
//  pinMode(Test,INPUT_PULLUP);   // setup Testbutton กดปุ่มเป็น LOW
  pinMode(Relay1,OUTPUT); // setup output
  pinMode(Relay2,OUTPUT); // setup output
  digitalWrite(Relay1,HIGH); // สั่งรอไว้
  digitalWrite(Relay2,HIGH); // สั่งรอไว้

}

void reconnect() {                                // ฟังก์ชั่นเชื่อมต่อใหม่อีกครั้ง
  while (!client.connected()) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password )) {//(UID, Identify, Scode) 
      Serial.println("Server Connected");
      client.subscribe("msg/swapper");            // เป็น Subscribe ใน topic ไหน
    }
    else {
      Serial.println("Try again in 5 s");
      delay(5000);
    }
  }
}

void callback(char* topic,byte* payload, unsigned int length) {       // ฟังก์ชั่นรับ ข้อความ msg
 
  String msg;
  for (int i = 0; i < length; i++) {
    msg = msg + (char)payload[i];
  }
  if (String(topic) == "msg/swapper") {    //  in topic 
    if (msg == "unlock"){                  //  msg command to unlock
      if (process == 0){                      // ถ้า process1 เริ่มสั่งที่ ตู้หนึ่ง
        Serial.println("Cabinet1 Start");
        Serial.println("in Process");
        Serial.println(process);
        unlock1 =1;
        cabinetlock1(unlock1);
      }
      if (process == 3){                     // ถ้า process3 เริ่มสั่งที่ ตู้สอง
        Serial.println("cabinet2 Start Process 3");
        Serial.println("in Process");
        Serial.println(process);
        unlock3 =1;
        cabinetlock3(unlock3);
        
      }
    }
    delay(1000);  
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long currentMillis = millis();

  //////////////////////////////////////////////////
  /////////////////@ start at 1 @///////////////////
  //////////////////////////////////////////////////
    
  bool ReadTriger1 = digitalRead(Trig1);
  if(ReadTriger1 == LOW && process == 1){     // when closed Locker 1 && process == 1
        unlock1 =0;                           // confirm lock
        cabinetlock1(unlock1);                // ไปรันที่ ฟังก์ชัน cabinetlock1 ให้ปิด
        Serial.println("in Process1 comfirm lock");
        Serial.println(process);
           delay(1000);
        float current = pzem1.current();      // ประกาศรับค่า กระแสที่ตู้หนึ่ง
        Serial.print("Current: "); Serial.print(current); Serial.println("A");   
       
      if (current >= bipcurrent1){                                  // after check Bip complete to step2 flow
        Serial.println("Cabinetlock1 Close complete");
        Serial.println("Done 1");
          delay(1000);
        unlock2 =1;                           // สั่งให้ 2 เปิด
        cabinetlock2(unlock2);                // ไปรันที่ ฟังก์ชัน cabinetlock2 ให้เปิด
        Serial.println("Cabinetlock2 Open complete");
        Serial.println("Please connected plug 2");
          delay(1000);
        if (currentMillis - previousMillis >= interval) { //ถ้าสถานะค้างเกินเวลา  60 นาทีให้ restart 
       // save the last time you blinked the LED
          previousMillis = currentMillis;
          ESP.restart();
        } 
      }
      if(current < bipcurrent1 || current == NAN){      // after check Bip "NOT" complete retry again 
        Serial.println("Plug1 is not connect");
        unlock1 =1;
        cabinetlock1(unlock1);
          delay(1000);
        Serial.println("Please connect plug1");
        if (currentMillis - previousMillis >= interval) { //ถ้าสถานะค้างเกินเวลา  60 นาทีให้ restart 
       // save the last time you blinked the LED
          previousMillis = currentMillis;
          ESP.restart();
        }  
      }
      delay(1000);
    }
//  continue from 1 to 2 //
  bool ReadTriger2 = digitalRead(Trig2);      // when closed Locker 2  
  if(ReadTriger2 == LOW && process == 2){     // confirm lock
      unlock2 =0;  
      cabinetlock2(unlock2);                  // ไปรันที่ ฟังก์ชัน cabinetlock1 ให้ปิด
        Serial.println("in Process2 comfirm lock");
        Serial.println(process);
           delay(1000);
        float current = pzem2.current();
        Serial.print("Current: "); Serial.print(current); Serial.println("A");
        Serial.println("Done 2");
        process=3;    

    delay(1000);                
  }

  //////////////////////////////////////////////////
  /////////////////@ start at 2 @///////////////////
  //////////////////////////////////////////////////

  bool ReadTriger3 = digitalRead(Trig2);      // when closed Locker 2
  if(ReadTriger3 == LOW && process == 4){    
        unlock3 =0;                           // confirm lock
        cabinetlock3(unlock3);
        Serial.println("in Process4 comfirm lock");
        Serial.println(process);
           delay(1000);
        float current = pzem2.current();
        Serial.print("Current: "); Serial.print(current); Serial.println("A");
            
      if (current >= bipcurrent2){                             // after check Bip complete to step2 flow
        Serial.println("Cabinetlock3 Close complete");
        Serial.println("Done 4");
          delay(1000);
        unlock4 =1;                           // confirm lock
        cabinetlock4(unlock4);
        Serial.println("Cabinetlock3 Open complete");
        Serial.println("Please connected plug 1");
          delay(1000);
        if (currentMillis - previousMillis >= interval) {
       // save the last time you blinked the LED
          previousMillis = currentMillis;
          ESP.restart();
        } 
      }
      if(current < bipcurrent2){                               // after check Bip "NOT" complete retry again 
        Serial.println("Plug2 is not connect");
        unlock3 =1;
        cabinetlock3(unlock3);
          delay(1000);
        Serial.println("Please connect plug2");
      }
      delay(1000);
    }
//  continue from 2 to 1 //
  bool ReadTriger4 = digitalRead(Trig1);      // when closed Locker 1
  if(ReadTriger4 == LOW && process == 5){    // confirm lock
        unlock4 =0;                           // confirm lock
        cabinetlock4(unlock4);
        Serial.println("in Process5 comfirm lock");
        Serial.println(process);
           delay(1000);
        float current = pzem1.current();
        Serial.print("Current: "); Serial.print(current); Serial.println("A");
        Serial.println("Done 4");
        process=0; 
    delay(1000);                
  }
  
}


void cabinetlock1(int unlock1) {
  if (unlock1 == 1){
    Serial.println("unlock1! process1");
    digitalWrite(Relay1,LOW);
    delay(500);
    process =1;                               //status Lock1=LOW  Process=1
    digitalWrite(Relay1,HIGH);
  } 
  else if (unlock1 == 0) {
    Serial.println("Lock1!");
    digitalWrite(Relay1,HIGH);
    delay(500);
    //process =0;
  }  
}

void cabinetlock2(int unlock2) {
  if (unlock2 == 1){
    Serial.println("unlock2! process2");
    digitalWrite(Relay2,LOW);
    delay(500);
    process =2;                               //status Lock2=LOW  Process=2
    digitalWrite(Relay2,HIGH);
  } 
  else if (unlock2 == 0) {
    Serial.println("Lock2! process3");
    digitalWrite(Relay2,HIGH);
    delay(500);
  }  
}

void cabinetlock3(int unlock3) {
  if (unlock3 == 1){
    Serial.println("unlock3! process4");
    digitalWrite(Relay2,LOW);
    process =4;
    delay(500);                               //status Lock3=LOW  Process=4
    digitalWrite(Relay2,HIGH);
  } 
  else if (unlock3 == 0) {
    Serial.println("Lock3!");
    digitalWrite(Relay2,HIGH);
    delay(500);    
  }  
}

void cabinetlock4(int unlock4) {
  if (unlock4 == 1){
    Serial.println("unlock4! process5");
    digitalWrite(Relay1,LOW);
    delay(500);
    process =5;                               //status Lock2=LOW  Process=5
    digitalWrite(Relay1,HIGH);
  } 
  else if (unlock4 == 0) {
    Serial.println("Lock4! process0");
    digitalWrite(Relay1,HIGH);
    delay(500);
  }  
}