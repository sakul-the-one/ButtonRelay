/*
* Welcome to the skirpt to turn your pc remotely on.
* There are some key Variables that need to be set:
* Set them here:
*/
//Main Variables:
#include "passwords.h"
//Both in ms: (1/1000s)
const uint16_t  Press_Time = 500; //Time the button should be pressed when the website button is pressed. 
//const uint16_t  Boot_Time = 30000; //Time it waits to boot the PC up in ms. So 30000 are 30 seconds
/*
* Thats it!
* Feel free to use!
* If you want to have some fun with some other settings, have fun here:
*/
const char * KeyBoardName = "Password"; //Max. 15 chars!
const char * KeyBoardProducerName = "Sakul"; //Max. 15 Cahrs!
uint8_t BatteryPercentage = 69; //Often Capout at 100. You may try more though.

//Libaries:
#define USE_NIMBLE
#include <BleKeyboard.h>
BleKeyboard bleKeyboard(KeyBoardName, KeyBoardProducerName, BatteryPercentage);
#include <WiFi.h>
#include <WebServer.h>
#include <string>
WebServer server(80);

//Pins:
//Pins for the Start Button
#define InputPin 35
#define ButtonPin 13
//Logic pin to start Motherboard:
#define KickstartPin 14
//Some other variables:
const char * InputFieldName PROGMEM = "pd";
const char * textHtml PROGMEM = "text/html";
bool started = false;
//Website:
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Boot System</title>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
body{
  margin:0;
  background:#000;
  color:#00ff9c;
  font-family:monospace;
  display:flex;
  justify-content:center;
  align-items:center;
  height:100vh;
}
.box{width:260px}
h1{font-size:20px;margin-bottom:10px}
input{
  width:100%;
  background:#000;
  color:#00ff9c;
  border:1px solid #00ff9c;
  padding:6px;
  margin:6px 0;
  font-family:monospace;
  width:100%;
  max-width:247px; /* or whatever width you want. */
}
button,input[type=submit]{
  background:#000;
  color:#00ff9c;
  border:1px solid #00ff9c;
  padding:6px;
  cursor:pointer;
  width:100%;
  max-width:320px; /* or whatever width you want. */
}
button:hover,input[type=submit]:hover{
  background:#00ff9c;
  color:#000;
}
</style>
</head>
<body>
<div class="box">
<h1>> SYSTEM BOOT</h1>
<form action="/get">
<input type="password" name="pd" id="pswd" placeholder="PASSWORD">
<input type="submit" value="START COMPUTER">
</form>
<button onclick="S()">SHOW PASSWORD</button>
</div>

<script> 
var PSWD = true;
a=document.getElementById("pswd"); 
function S() 
  {
  if(PSWD === true) {
    a.setAttribute("type","text");
    PSWD = false;
  }
  else {
    a.setAttribute("type","password");
    PSWD = true;
  }
}
</script>
</body>
</html>

)rawliteral";
/*void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}*/
int getStringLenght(char * str) 
{
  int r = 0;
  for (; str[r] != '\0'; r++);
  return r;
}
bool CompareStrings(char * org, String str2) 
{
  int orgLen = getStringLenght(org); 
  if (orgLen != str2.length()) return false;
  for (int i = 0; i < orgLen; i++) 
  {
    if (org[i] != str2[i])
      return false;
  }
  return true;
}
//Turn pc on:
void TurnComputerOn() 
{
  Serial.println("Turning PC on");
  //Start Connection:
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(KickstartPin, HIGH);
  //The buttons needs often roughly be pressed for ~ half a second
  delay(Press_Time);
  //turn it off, cause we dont want to press it the whole time
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(KickstartPin, LOW);
  //Wait for it to boot
  //delay(Boot_Time);
  //server.end();
  WiFi.disconnect();;
  delay(1000);
  //bleKeyboard.begin();
  while (!bleKeyboard.isConnected())
  {
    delay(1000); //If not connected, wait a second
    Serial.println("Waiting for PC connection");
  }
  delay(2000); //wait 2 seconds for good meassures
  Serial.println("entering password");
  digitalWrite(LED_BUILTIN, HIGH);
  //Select Account:
  bleKeyboard.press(KEY_LEFT_CTRL);
  delay(200);
  bleKeyboard.releaseAll();
  delay(200);
  //Write Password:
  bleKeyboard.print(Computer_Password);
  delay(200);
  //Press Enter:
  bleKeyboard.press(KEY_RETURN);
  delay(200);
  bleKeyboard.releaseAll();
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Turned PC on");
  started = true;
}
//website functions:
void handleGet() {

  if(!server.hasArg(InputFieldName)) {
    server.send(200, textHtml, "No password");
    return;
  }

  String inputMessage = server.arg(InputFieldName);

  if(CompareStrings((char *)Computer_Password, inputMessage) == true)
  {
    server.send(200, textHtml, "Success");
    TurnComputerOn();
  }
  else
  {
    server.send(200, textHtml, "Wrong password. wait 5 seconds");
    delay(5000);
  }
}

void handleRoot() 
{
  server.send_P(200, textHtml, index_html);
}

void setup() {
  //Start Keyboard and enter password:
  bleKeyboard.begin();
  Serial.begin(115200);
  //Setting all the Pins:
  //Debug light:
  pinMode(LED_BUILTIN, OUTPUT);
  //Init both Buttons
  pinMode(InputPin, INPUT);
  pinMode(ButtonPin, OUTPUT);
  //Motherboard Start Pin:
  pinMode(KickstartPin, OUTPUT);
  //Set ButtonPin always on high
  digitalWrite(ButtonPin, HIGH);

  //Connect to Wlan:
  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFi_Name, WiFi_Password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("E: WF");
    return;
  }
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  //Create Website:
  server.on("/", handleRoot);
  server.on("/get", handleGet);
  server.onNotFound([](){
    server.send(404, "text/plain", "Not found");
  });

  server.begin();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  server.handleClient();
  int test = 0;
  test = digitalRead(InputPin);
  if(test)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(KickstartPin, HIGH);
  }
  else 
  {
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(KickstartPin, LOW);
  }
  if(!bleKeyboard.isConnected() && started)
    esp_restart();
}