/*
* Welcome to the skirpt to turn your pc remotely on.
* There are some key Variables that need to be set:
* Set them here:
*/
//Main Variables:
const char * WiFi_Name = "Paste Wifi Name Here"; //Often also called SSID
const char * WiFi_Password = "Paste Wifi Password here";
const char * Computer_Password = "PC Password";
//Both in ms: (1/1000s)
const uint16_t  Press_Time = 500; //Time the button should be pressed when the website button is pressed. 
const uint16_t  Boot_Time = 30000; //Time it waits to boot the PC up
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
//Website:
const char index_html[] PROGMEM = R"rawliteral(
<head><title>ESP32</title></head>
<body>
<h1>Turn PC on</h1>
Enter the Password:
<form action="/get">
<input type="password" name="pd" id= "p">
<input type="submit" value="Start Computer">
<button onclick="S()">Show Password</button>
</form>
<script> 
var P = true;
a=document.getElementById("p"); 
function S() 
{
if(PSWD === true) {
a.setAttribute("type","text");
P = false;
}
else 
{
a.setAttribute("type","password");
PSWD = true;
}
}
</script>
</body>
)rawliteral";
/*void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}*/
//Turn pc on:
void TurnComputerOn() 
{
  //Start Connection:
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(KickstartPin, HIGH);
  //The buttons needs often roughly be pressed for ~ half a second
  delay(Press_Time);
  //turn it off, cause we dont want to press it the whole time
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(KickstartPin, LOW);
  //Wait for it to boot
  delay(Boot_Time);
  //Select Account:
  bleKeyboard.press(KEY_LEFT_CTRL);
  delay(100);
  bleKeyboard.releaseAll();
  //Write Password:
  bleKeyboard.print(Computer_Password);
  delay(100);
  //Press Enter:
  bleKeyboard.press(KEY_NUM_ENTER);
  delay(100);
  bleKeyboard.releaseAll();
}
//wbsite functions:
void handleGet() {

  if(!server.hasArg(InputFieldName)) {
    server.send(200, textHtml, "No password");
    return;
  }

  String inputMessage = server.arg(InputFieldName);

  if(strcmp(Computer_Password, inputMessage.c_str()) == 0)
  {
      TurnComputerOn();
      server.send(200, textHtml, "Success");
  }
  else
  {
      server.send(200, textHtml, "Wrong password");
  }
}
void handleRoot() {
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
  Serial.print("IP");
  Serial.println(WiFi.localIP());
  //Create Website:
  server.on("/", handleRoot);
  server.on("/get", handleGet);
  server.onNotFound([](){
    server.send(404, "text/plain", "Not found");
  });

  server.begin();
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
}