#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "LiquidCrypstal.h"

const char* ssid     = "GSIX";
const char* password = "sk0psk0K4le";
const char* headers[] = {"Connection", "close", "Access-Control-Allow-Origin", "*", "application/json"};

byte pins[11] = {D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10};

unsigned int reset=0;

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
LiquidCrypstal LCD(D1, D2, D3);

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

  for (int i=0; i<9; i++) pinMode(pins[i], OUTPUT);
  digitalWrite(D0, HIGH);
  digitalWrite(D4, HIGH);
  delay(3000);
  LCD.clear();
  LCD.home();
  LCD.print("Office Shooting:");
}

void loop() {
  server.handleClient();
  delay(100);
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

  LCD.setCursor(0,1);
  LCD.print(since);
}
