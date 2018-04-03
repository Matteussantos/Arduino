// Bibliotecas
//Blynk

#define BLYNK_PRINT Serial
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>

// Use esses parametros para Arduino Mega
#define EspSerial Serial1

// Use esses parametros para Arduino Uno:

//#include <SoftwareSerial.h>
//SoftwareSerial EspSerial(2, 3); // RX, TX

// Especificar ESP8266 baud rate:
#define ESP8266_BAUD 115200

ESP8266 wifi(&EspSerial);

// Sensor DHT

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN            2         // Conectado ao pino 2
// Selecione o modelo do sensor DHT
#define DHTTYPE           DHT11     // DHT 11 
//#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

// Infravermelho conectado ao pino 9 (Usar transistor BC458)

#include <IRremote.h>
#include <Wire.h>

// token, ssid, e pass para Blynk

char auth[] = "xxxxx";
char ssid[] = "Mateus";
char pass[] = "comaverduras";



// Parametros Infravermelho

IRsend irsend;

// Normalmente nao e necessario usar o RECV_PIN
int RECV_PIN = 11;
IRrecv irrecv (RECV_PIN);

const int AC_TYPE  = 1;
// 0 : Ar condicionado tipo torre
// 1 : Ar condicionado tipo parede ou split
//

int AC_HEAT = 0;
// 0 : cooling
// 1 : heating

int AC_POWER_ON    = 1;
// 0 : off
// 1 : on

int AC_AIR_ACLEAN  = 1;
// 0 : off
// 1 : on --> power on

int AC_TEMPERATURE = 18;
// temperature : 18 ~ 30

int AC_FLOW        = 3;
// 0 : low
// 1 : mid
// 2 : high
// Se o ar condicionado é parede ou split usar o cod. 3


const int AC_FLOW_TOWER[3] = {0, 4, 6};
const int AC_FLOW_WALL[4]  = {0, 2, 4, 5};

unsigned long AC_CODE_TO_SEND;

int r = LOW;
int o_r = LOW;

byte a, b;

void ac_send_code(unsigned long code)
{
  Serial.print("code to send : ");
  Serial.print(code, BIN);
  Serial.print(" : ");
  Serial.println(code, HEX);

  irsend.sendLG(code, 28);
}

void ac_activate(int temperature, int air_flow)
{

  int AC_MSBITS1 = 8;
  int AC_MSBITS2 = 8;
  int AC_MSBITS3 = 0;
  int AC_MSBITS4 ;
  if ( AC_HEAT == 1 ) {
    // heating
    AC_MSBITS4 = 4;
  } else {
    // cooling
    AC_MSBITS4 = 0;
  }
  int AC_MSBITS5 = temperature - 15;
  int AC_MSBITS6 ;

  if ( AC_TYPE == 0) {
    AC_MSBITS6 = AC_FLOW_TOWER[air_flow];
  } else {
    AC_MSBITS6 = AC_FLOW_WALL[air_flow];
  }

  int AC_MSBITS7 = (AC_MSBITS3 + AC_MSBITS4 + AC_MSBITS5 + AC_MSBITS6) & B00001111;

  AC_CODE_TO_SEND =  AC_MSBITS1 << 4 ;
  AC_CODE_TO_SEND =  (AC_CODE_TO_SEND + AC_MSBITS2) << 4;
  AC_CODE_TO_SEND =  (AC_CODE_TO_SEND + AC_MSBITS3) << 4;
  AC_CODE_TO_SEND =  (AC_CODE_TO_SEND + AC_MSBITS4) << 4;
  AC_CODE_TO_SEND =  (AC_CODE_TO_SEND + AC_MSBITS5) << 4;
  AC_CODE_TO_SEND =  (AC_CODE_TO_SEND + AC_MSBITS6) << 4;
  AC_CODE_TO_SEND =  (AC_CODE_TO_SEND + AC_MSBITS7);

  ac_send_code(AC_CODE_TO_SEND);

  AC_POWER_ON = 1;
  AC_TEMPERATURE = temperature;
  AC_FLOW = air_flow;
}

void ac_change_air_swing(int air_swing)
{
  if ( AC_TYPE == 0) {
    if ( air_swing == 1) {
      AC_CODE_TO_SEND = 0x881316B;
    } else {
      AC_CODE_TO_SEND = 0x881317C;
    }
  } else {
    if ( air_swing == 1) {
      AC_CODE_TO_SEND = 0x8813149;
    } else {
      AC_CODE_TO_SEND = 0x881315A;
    }
  }

  ac_send_code(AC_CODE_TO_SEND);
}

void ac_power_down()
{
  AC_CODE_TO_SEND = 0x88C0051;

  ac_send_code(AC_CODE_TO_SEND);

  AC_POWER_ON = 0;
}

void ac_air_clean(int air_clean)
{
  if ( air_clean == 1) {
    AC_CODE_TO_SEND = 0x88C000C;
  } else {
    AC_CODE_TO_SEND = 0x88C0084;
  }

  ac_send_code(AC_CODE_TO_SEND);

  AC_AIR_ACLEAN = air_clean;
}

BLYNK_WRITE(V3)
{
  int pinValue = param.asInt();
if(pinValue == 1)
  {
     
  ac_activate(18, 3);
  //delay(5000);
  //ac_activate(27, 0);
  //delay(5000);


  if ( r != o_r) {

    /*
    # a : mode or temp    b : air_flow, temp, swing, clean, cooling/heating
    # 18 ~ 30 : temp      0 ~ 2 : flow // on
    # 0 : off             0
    # 1 : on              0
    # 2 : air_swing       0 or 1
    # 3 : air_clean       0 or 1
    # 4 : air_flow        0 ~ 2 : flow
    # 5 : temp            18 ~ 30
    # + : temp + 1
    # - : temp - 1
    # m : change cooling to air clean, air clean to cooling
    */
    Serial.print("a : ");
    Serial.print(a);
    Serial.print("  b : ");
    Serial.println(b);

    switch (a) {
      case 0: // off
        ac_power_down();
        break;
      case 1: // on
        ac_activate(AC_TEMPERATURE, AC_FLOW);
        break;
      case 2:
        if ( b == 0 || b == 1 ) {
          ac_change_air_swing(b);
        }
        break;
      case 3: // 1  : clean on, power on
        if ( b == 0 || b == 1 ) {
          ac_air_clean(b);
        }
        break;
      case 4:
        if ( 0 <= b && b <= 2  ) {
          ac_activate(AC_TEMPERATURE, b);
        }
        break;
      case 5:
        if (18 <= b && b <= 30  ) {
          ac_activate(b, AC_FLOW);
        }
        break;
      case '+':
        if ( 18 <= AC_TEMPERATURE && AC_TEMPERATURE <= 29 ) {
          ac_activate((AC_TEMPERATURE + 1), AC_FLOW);
        }
        break;
      case '-':
        if ( 19 <= AC_TEMPERATURE && AC_TEMPERATURE <= 30 ) {
          ac_activate((AC_TEMPERATURE - 1), AC_FLOW);
        }
        break;
      case 'm':
        /*
          if ac is on,  1) turn off, 2) turn on ac_air_clean(1)
          if ac is off, 1) turn on,  2) turn off ac_air_clean(0)
        */
        if ( AC_POWER_ON == 1 ) {
          ac_power_down();
          delay(100);
          ac_air_clean(1);
        } else {
          if ( AC_AIR_ACLEAN == 1) {
            ac_air_clean(0);
            delay(100);
          }
          ac_activate(AC_TEMPERATURE, AC_FLOW);
        }
        break;
      default:
        if ( 18 <= a && a <= 30 ) {
          if ( 0 <= b && b <= 2 ) {
            ac_activate(a, b);
          }
        }
    }

    o_r = r ;
  }
  delay(100);
}
else {
          if ( pinValue == 0) {
            AC_CODE_TO_SEND = 0x88C0051;

  ac_send_code(AC_CODE_TO_SEND);

  AC_POWER_ON = 0;
          }
           
}

}

  
void setup()
{
  // Debug console
  Serial.begin(9600);

  delay(10);

  // Set ESP8266 baud rate
  EspSerial.begin(ESP8266_BAUD);
  delay(10);

  Blynk.begin(auth, wifi, ssid, pass);
  dht.begin();
    sensor_t sensor;
  dht.temperature().getSensor(&sensor);
    // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
    // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;

 
}


void loop()
{

  
   // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Serial.print("Temperature: ");
    Serial.print(event.temperature);
    Blynk.virtualWrite(V1, event.temperature); // Temperature for gauge
    Serial.println(" *C");
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    Serial.print("Humidity: ");
    Serial.print(event.relative_humidity);
    Blynk.virtualWrite(V2, event.relative_humidity); // Temperature for gauge
    Serial.println("%");
  }






{

  Blynk.run();
}
}

void receiveEvent(int howMany)
{
  a = Wire.read();
  b = Wire.read();
  r = !r ;
}
