#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "LiquidCrypstal.h"

#include <OneWire.h>
#include <DallasTemperature.h>

const char* ssid     = "Gajtanless";
const char* password = "google.com.mk";
const char* headers[] = {"Connection", "close", "Access-Control-Allow-Origin", "*", "application/json"};

byte pins[11] = {D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10};







//timers
unsigned int reset=0;
unsigned int measure=0;

//analog moving average
#define A0_MEASUREMENTS_COUNT 50
int A0values[A0_MEASUREMENTS_COUNT];
float A0average=0;
byte A0counter=0;












//NTC
float Vin=3.3;     // [V]        
float Rt=10000;    // Resistor t [ohm]
float R0=11000;    // value of rct in T0 [ohm]
float T0=298.15;   // use T0 in Kelvin [K]
float Vout=0.0;    // Vout in A0 
float Rout=0.0;    // Rout in A0
// use the datasheet to get this data.
float T1=283.8;      // [K] in datasheet 0º C
float T2=333.15;      // [K] in datasheet 100° C
float RT1=19510;   // [ohms]  resistence in T1
float RT2=3385;    // [ohms]   resistence in T2
float beta=0.0;    // initial parameters [K]
float Rinf=0.0;    // initial parameters [ohm]   
float TempK=0.0;   // variable output
float TempC=0.0;   // variable output









//buton
byte flashState=1;
unsigned int debounce;

OneWire oneWire(D6);
DallasTemperature DS18B20(&oneWire);


byte smiley[8] = {
  B00000,
  B01010,
  B01010,
  B00000,
  B10001,
  B11111,
  B00000,
  B00000
};


ESP8266WebServer server(80);
LiquidCrypstal LCD(D1, D2, D5);

void root()
{
  server.sendHeader(headers[0], headers[1]);
  server.sendHeader(headers[2], headers[3]);

  if ( server.hasArg("pin") && server.hasArg("state") )
  {
    String pin = server.arg("pin");
    byte pinD = 0;
    byte state = (server.arg("state")).equals("0") ? 0:1;
    if (pin.equals("0") || pin.equals("D0") || pin.equals("d0")) pinD = 0;
    else if (pin.equals("1") || pin.equals("D1") || pin.equals("d1")) pinD = 1;
    else if (pin.equals("2") || pin.equals("D2") || pin.equals("d2")) pinD = 2;
    else if (pin.equals("3") || pin.equals("D3") || pin.equals("d3")) pinD = 3;
    else if (pin.equals("4") || pin.equals("D4") || pin.equals("d4")) pinD = 4;
    else if (pin.equals("5") || pin.equals("D5") || pin.equals("d5")) pinD = 5;
    else if (pin.equals("6") || pin.equals("D6") || pin.equals("d6")) pinD = 6;
    else if (pin.equals("7") || pin.equals("D7") || pin.equals("d7")) pinD = 7;
    else if (pin.equals("8") || pin.equals("D8") || pin.equals("d8")) pinD = 8;
    else if (pin.equals("9") || pin.equals("D9") || pin.equals("d9")) pinD = 9;
    else if (pin.equals("10") || pin.equals("D10") || pin.equals("d10")) pinD = 10;

    digitalWrite(pins[pinD], state);
  }

  if (server.hasArg("led"))
  {
        (server.arg("led")).equals("0") ? LCD.ledOff():LCD.ledOn();
  }

  if (server.hasArg("clear"))
  {
        LCD.clear();
        LCD.home();
  }
  
  if (server.hasArg("text"))
  {
        LCD.print(server.arg("text"));
  }
  
  if (server.hasArg("allahu"))
  {
        if (server.arg("allahu").equals("akbar")) reset=millis();
  }
  
  
  String response;
  response = "{ ";
  for (int i=0; i<8; i++)
  {
    response += "\"D";
    response += i;
    response += "\": ";
    response += digitalRead(pins[i])? "true":"false";
    response += ", ";
  }

  response += "\"D8\": ";
  response += digitalRead(pins[8])? "true":"false";
  response += " }";
  response += "\n//Available commands: clear, text, led, [pin, state]";
  
  server.send(200, headers[4], response);
}


void setup() {
  Serial.begin(115200);
  delay (10);
/*
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  LCD.createChar(0, smiley); 
  LCD.begin(16,2);
//  LCD.write(byte(0));
  LCD.noDisplay();
  LCD.print("esp8266.local");
  LCD.setCursor(0,1);
  LCD.print(WiFi.localIP());
  LCD.display();
  LCD.ledOn();

  server.on("/", root);
  server.begin();
  Serial.println("Server started");

  MDNS.begin("esp8266");
  MDNS.addService("http", "tcp", 80);
*/


  //NTC
  beta=(log(RT1/RT2))/((1/T1)-(1/T2));
  beta=3333;
  Rinf=R0*exp(-beta/T0);


  LCD.createChar(0, smiley); 
  LCD.begin(16,2);
  for (int i=0; i<9; i++) pinMode(pins[i], OUTPUT);
  pinMode(D3, INPUT_PULLUP);
  digitalWrite(D0, HIGH);
  digitalWrite(D4, HIGH);
  delay(3000);
  LCD.clear();
  LCD.home();
  LCD.display();
  LCD.print("R       D");
}


byte pinState=1, lastState=1;
void loop() {
  //server.handleClient();
  delay(5);
  
  pinState=digitalRead(D3);
  if ( pinState!=flashState ) debounce=millis();
  if ( millis()-debounce>50 )
  {
    if (pinState != lastState)
    {
      /*if (pinState==1)*/ reset=millis();
      lastState=pinState;
    }
  }
  flashState=pinState;
  
  String since;
  unsigned long now = (millis()-reset)/1000;
  if (now/60/60<10)   since += "0";
  since += now/60/60;
  since += "h ";
  if (now/60%60<10)   since += "0";
  since += now/60%60;
  since += "m ";
  if (now%60<10)   since += "0";
  since += now%60;
  since += "s ago         ";

//  Serial.println(analogRead(A0));
  DS18B20.requestTemperatures(); 
  float temp = DS18B20.getTempCByIndex(0);
  LCD.setCursor(0,1);
  
  if (millis()-measure>10)
  {
    A0values[A0counter] = analogRead(A0)*1.044-9; //CALIBRATION
    if (A0values[A0counter]>1024) A0values[A0counter]=1024;
    if (A0values[A0counter]<0) A0values[A0counter]=0;
    
    A0counter = (A0counter+1)%A0_MEASUREMENTS_COUNT;
    
    A0average=0;
    for (int i=0; i<A0_MEASUREMENTS_COUNT; i++) A0average+=A0values[i];
    A0average = A0average/A0_MEASUREMENTS_COUNT;
    measure=millis();


    float Vout=Vin*((float)(A0average)/1024.0); // calc for ntc
    float Rout=(10000*Vout/(Vin-Vout));
    TempK=(beta/log(Rout/Rinf)); // calc for temperature
    TempC=TempK-273.15;
  }

  if (millis()-reset>500)
  {
    LCD.print(TempC);/*0.003222656*/
    LCD.print("                    ");
    LCD.setCursor(8,1);
    LCD.print(A0average);
    reset=millis();
  }
}
