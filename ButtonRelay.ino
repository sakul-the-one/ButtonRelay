/*
* Welcome to the skirpt to turn your pc remotely on.
* There are some key Variables that need to be set:
* Set them here:
*/
//Main Variables:
//const char * WiFi_Name = "";
//const char * WiFi_Password = "";
//const char * Computer_Password = "";
#include "passwords.h"
//Both in ms: (1/1000s)
const uint16_t  Press_Time = 500; //Time the button should be pressed when the website button is pressed. 
/*
* Thats it!
* Feel free to use!
* If you want to have some fun with some other settings, have fun here:
*/
const char * KeyBoardName = "Password"; //Max. 15 chars!
const char * KeyBoardProducerName = "Sakul"; //Max. 15 Cahrs!
uint8_t BatteryPercentage = 69; //Often Capout at 100. You may try more though.
const int ButtonOffset = 125; //In ms. This helps to stop the constant clicking of the Relay

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
uint16_t MainKey = 0;
char * EncodedPSW;
bool TurnOn = false;
int Timer = 0;
//Website:
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Boot System</title>
<meta name="viewport" content="width=device-width,initial-scale=1">
   <style>
        * {margin: 0; padding: 0;}
        body {background: black;}
canvas {display: block;}
    </style>
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
canvas {
  position: fixed;
  top: 0;
  left: 0;
  z-index: 0;
}
.box{
width:260px;
position: relative;
 z-index: 1;
}
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
<canvas id="c"></canvas>
<div class="box">
<h1>> SYSTEM BOOT</h1>
<input type="password" name="pd" id="pswd" placeholder="PASSWORD">
<button onclick="SendPSW()">START COMPUTER</button>
<button onclick="S()">SHOW PASSWORD</button>
</div>
</body>
<script> 
var PSWD = true;
let KEY = null;

window.onload = function(){
  fetch("/key")
  .then(r => r.text())
  .then(k => {
      KEY = parseInt(k);
      console.log("Key:", KEY);
  });
}
a=document.getElementById("pswd");
a.addEventListener("keypress",function(e){
  if(e.key==="Enter") SendPSW();
});
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
function SendPSW() 
{
  if(KEY === null){
        alert("No Criptic key received yet");
        return;
    }
	if(a.value.length === 0)
    {
    	alert("No Password entered!");
        return;
    }
	let Cripto = Encode(a.value, KEY);
  let send = encodeURIComponent(btoa(Cripto)); //encodeURIComponent(
  //alert("org: " + a.value + " key: " + KEY + " enc: " + send + " dbg: " + Cripto + "; lenght: " + send.length);
  window.location = "/get?pd=" + send;
}
function Encode(str, key)
{
    let length = str.length;
    let ret = "";
    for(let i = 0; i < length; i++)
    {
        let xor_key = ((key >> (8 * (i % 2))) * i) & 255;
        ret += String.fromCharCode(str.charCodeAt(i) ^ xor_key);
    }
    ret += String.fromCharCode(length);
    return ret;
}
//NAVAS:
var c = document.getElementById("c");
var ctx = c.getContext("2d");

//making the canvas full screen
c.height = window.innerHeight;
c.width = window.innerWidth;

//english characters
var english = "1001010101110101010101010010101000101011101111010101010110101010101010101110000101";
//converting the string into an array of single characters
english = english.split("");

var font_size = 15;
var columns = c.width/font_size; //number of columns for the rain
//an array of drops - one per column
var drops = [];
//x below is the x coordinate
//1 = y co-ordinate of the drop(same for every drop initially)
for(var x = 0; x < columns; x++)
	drops[x] = 1; 

//drawing the characters
function draw()
{
	//Black BG for the canvas
	//translucent BG to show trail
	ctx.fillStyle = "rgba(0, 0, 0, 0.05)";
	ctx.fillRect(0, 0, c.width, c.height);
	
	ctx.fillStyle = "#0F0"; //green text
	ctx.font = font_size + "px arial";
	//looping over drops
	for(var i = 0; i < drops.length; i++)
	{
		//a random chinese character to print
		var text = english[Math.floor(Math.random()*english.length)];
		//x = i*font_size, y = value of drops[i]*font_size
		ctx.fillText(text, i*font_size, drops[i]*font_size);
		
		//sending the drop back to the top randomly after it has crossed the screen
		//adding a randomness to the reset to make the drops scattered on the Y axis
		if(drops[i]*font_size > c.height && Math.random() > 0.975)
			drops[i] = 0;
		
		//incrementing Y coordinate
		drops[i]++;
	}
}

setInterval(draw, 33);
</script>
</html>

)rawliteral";
// //console.log("org: " + a.value + " enc: " + btoa(Cripto) + " dbg: " + Cripto + ";");
/*void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}*/
//Base64:
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};
//Convert string to base64
char * base64_encode(const char * data, size_t input_length, size_t *output_length) 
{
    *output_length = (4 * ((input_length + 2) / 3));

    char *encoded_data = (char *) malloc((*output_length) + 2); //+1 for Null character
    memset(encoded_data, '\0', *output_length + 1);
    if (encoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';
    //encoded_data[*output_length] = '\0';
    Serial.printf("Lenght: %i ", *output_length);
    return encoded_data;
}
//Custom Hashing (?) Algorythm
char* encode(const char* str, uint16_t key) 
{
    int lenght = getStringLenght((char *)str);
    char * ret = (char *)malloc(lenght + 1);
    for (uint8_t i = 0; i < lenght; i++) 
    {
        uint8_t xor_key = (key >> (8 * (i % 2))) * i;
        ret[i] = str[i] ^ xor_key;
    }
    ret[lenght] = (char)lenght;
    //ret[lenght+1] = '\0';
    return ret;
}
int getStringLenght(const char * str) 
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
  bleKeyboard.begin(); //turning it back on, since we need it
  bleKeyboard.setDelay(100);
  //delay(500);
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
  delay(300);
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
#pragma region ServerFunctions
//website functions:

//Handle Server Start Request
void handleGet() {

  if(!server.hasArg(InputFieldName)) {
    server.send(200, textHtml, "No password");
    return;
  }

  String inputMessage = server.arg(InputFieldName);

  if(CompareStrings(EncodedPSW, inputMessage) == true)
  {
    server.send(200, textHtml, "Success <br><a href='/'>Home</a> <script> setTimeout(() => { window.location = '/'}, 4500); </script>");
    TurnComputerOn();
  }
  else
  {
    server.send(200, textHtml, "Wrong password. wait 5 seconds<br><a href='/'>Home</a> <script> setTimeout(() => { window.location = '/'}, 4500); </script>");
    delay(5000);
  }
  Serial.printf("Received: %s; Expected: %s\n", inputMessage, EncodedPSW);
}
//Handle Website Key Request
void handleKey() {
  //what I tried first:
  /*free(EncodedPSW);
  uint16_t key = MainKey;
  size_t temp = 0;
  char * tmpStr = encode(Computer_Password, key, password_length);
  EncodedPSW = base64_encode((const unsigned char * )tmpStr, password_length + 1, &temp);
  Serial.printf("Key: %i; EncodedPSW: %s\n", key, EncodedPSW);
  server.send(200, "text/plain", String(key));
  free(tmpStr);*/
  //what worked in https://godbolt.org/ Idk why the other one isnt working
  free(EncodedPSW);
  char * out = NULL;
  uint16_t key = MainKey;
  size_t outLenght = 0;
  server.send(200, "text/plain", String(key));
  out = encode(Computer_Password, key);
  //printf("In: %s; Out: %s\n", Computer_Password, out);
  EncodedPSW = base64_encode(out, getStringLenght(Computer_Password) + 1, &outLenght);
  free(out);
  //printf("in base64: %s; lenght in: %i", out_2, outLenght);
}
//Handle website Init
void handleRoot() 
{
  server.send_P(200, textHtml, index_html);
}
#pragma endregion ServerFunctions
//Main:
void setup() {
  //Start Communication with PC
  Serial.begin(115200);
  Serial.print("Hi \n\n");
  bleKeyboard.begin(); //Turn it on before Wifi, since otherwise it wouldnt work
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
  if (WiFi.waitForConnectResult() != WL_CONNECTED) //Check for Error
  {
    Serial.println("E: WF");
    return;
  }
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  //Create Website&Handles:
  server.on("/", handleRoot);
  server.on("/get", handleGet);
  server.on("/key", handleKey);
  server.onNotFound([](){
    server.send(404, "text/plain", "Not found");
  });

  server.begin();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  //Not needed, just there to pre malloc
  EncodedPSW = encode(Computer_Password, MainKey);
  MainKey += 0xA0B2;
  bleKeyboard.end();//Turn it off, since we dont need it yet
}
//Main Loop:
void loop() {
  MainKey++;
  if(!started) //Check if Started, cause when it has started, there is no server/WLan anymore
    server.handleClient(); //So server doesnt crash
  int test = 0; //Button Pin Statement
  test = digitalRead(InputPin); //Get if the Button is turned on
  //Manage offset timer:
  if (test && !TurnOn)
    Timer++;
  else if (!test && TurnOn)
    Timer--;
  //If Timer meets condition, change status
  if (Timer >= ButtonOffset) TurnOn = true;
  else if (Timer <= 0) TurnOn = false;

  if(TurnOn)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(KickstartPin, HIGH);
  }
  else 
  {
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(KickstartPin, LOW);
  }
  if(!bleKeyboard.isConnected() && started) //When PC turns off (disconnects from the chip) and PC started, restart board, so it can be started again
    esp_restart();
}