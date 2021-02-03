// v1.3
//Library including
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

//SET SSID HERE
#define ssid "SMART_CAR_05" //SET SSID HERE

// PIN DEFINING
#define MA D3
#define MB D4
#define MAPWM D1
#define MBPWM D2
#define Speed 850

// Variable use for transmittion, so don't touch.
String sliderValue = "0";
const char *PARAM_INPUT = "value";

// Direction , Slider Value
int Direction = 0, S = 0;

#define Stop 0
#define Forward 1
#define Backward 2
#define Left 3
#define Right 4

//SPEED CONTROL

// Server library
AsyncWebServer server(80);

//HTML + CSS + JAVASCRIPT MAIN PAGE /////////////////////////
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>SMART CAR</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;  touch-action: none;}
    h2 {font-size: 2.3rem;}
    p {font-size: 1.9rem;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
    .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #FFD65C;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #003249; cursor: pointer;}
    .slider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; }
  #main {  align-items: auto;  display: flex;  height: 100vh;  justify-content: auto;  margin: 46px;  width: 100vw;}
  #wrapper {border: 3px solid red;width: 300px; height: 300px; }
  .joystick { background-color: grey; border-radius: 100%; cursor: pointer; height: 100%; user-select: none; width: 100%; }
  </style>
</head>
<body>
  <h2>SMART CAR</h2>
  <p><span id="textSliderValue">0</span></p>
  <p><input type="range" onchange="updateSliderPWM(this)" id="pwmSlider" min="0" max="100" value="0" step="1" class="slider"></p>
  <p><div id="main"><div id="wrapper"></div></div></p>
<script>

var Direction = 0;
var OldDirection = 0;
function updateSliderPWM(element) {
  var sliderValue = document.getElementById("pwmSlider").value;
  document.getElementById("textSliderValue").innerHTML = sliderValue;
  console.log(sliderValue);
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/slider?value="+sliderValue, true);
  xhr.send();
}
const joystick = createJoystick(document.getElementById('wrapper'));
function createJoystick(parent) {
  const maxDiff = 100;
  const stick = document.createElement('div');
  stick.classList.add('joystick');

  stick.addEventListener('mousedown', handleMouseDown);
  document.addEventListener('mousemove', handleMouseMove);
  document.addEventListener('mouseup', handleMouseUp);
  stick.addEventListener('touchstart', handleMouseDown);
  document.addEventListener('touchmove', handleMouseMove);
  document.addEventListener('touchend', handleMouseUp);

  let dragStart = null;
  let currentPos = { x: 0, y: 0 };

  function handleMouseDown(event) {
    stick.style.transition = '0s';
    if (event.changedTouches) {
      dragStart = {
        x: event.changedTouches[0].clientX,
        y: event.changedTouches[0].clientY,
      };
      return;
    }
    dragStart = {
      x: event.clientX,
      y: event.clientY,
    };
  
  Direction = 0;

  }

  function handleMouseMove(event) {
    if (dragStart === null) return;
    event.preventDefault();
    if (event.changedTouches) {
      event.clientX = event.changedTouches[0].clientX;
      event.clientY = event.changedTouches[0].clientY;
    }
    const xDiff = event.clientX - dragStart.x;
    const yDiff = event.clientY - dragStart.y;
    const angle = Math.atan2(yDiff, xDiff);
    const distance = Math.min(maxDiff, Math.hypot(xDiff, yDiff));
    const xNew = distance * Math.cos(angle);
    const yNew = distance * Math.sin(angle);
    stick.style.transform = `translate3d(${xNew}px, ${yNew}px, 0px)`;
    currentPos = { x: xNew, y: yNew };
  if((yNew < "-20")&&(xNew>"-45"&&xNew<"45")) Direction =1;
  if((yNew > "20")&&(xNew>"-45"&&xNew<"45")) Direction =2;
    if((xNew < "-20")&&(yNew>"-45"&&yNew<"45")) Direction =3;
  if((xNew > "20")&&(yNew>"-45"&&yNew<"45")) Direction =4;
  if((xNew < "20"&&xNew > "-20")&&(yNew<"20"&&yNew>"-20")) Direction = 0;
  }

  function handleMouseUp(event) {
    if (dragStart === null) return;
    stick.style.transition = '.2s';
    stick.style.transform = `translate3d(0px, 0px, 0px)`;
    dragStart = null;
    currentPos = { x: 0, y: 0 };
    Direction = 0;
  }

  parent.appendChild(stick);
  return {
    getPosition: () => currentPos,
  };
}

setInterval(function(){sendData();},1);

function sendData()
{
  if(Direction != OldDirection)
  {
    OldDirection = Direction;
      var xhr = new XMLHttpRequest();
  xhr.open("GET", "/D?value="+Direction, true);
  xhr.send();
  document.getElementById("textSliderValue").innerHTML = Direction;
  }
}

</script>
</body>
</html>
)rawliteral";

//END OF HTML PAGE

void setup()
{

  //Pin Configuration
  pinMode(MA, OUTPUT);
  pinMode(MAPWM, OUTPUT);
  pinMode(MB, OUTPUT);
  pinMode(MBPWM, OUTPUT);

  digitalWrite(MA, LOW);
  digitalWrite(MB, LOW);
  analogWrite(MAPWM, 0);
  analogWrite(MBPWM, 0);
  //Serial monitor and WIFI Initialize
  IPAddress local_ip = {1, 1, 1, 1};
  IPAddress gateway = {192, 168, 1, 1};
  IPAddress subnet = {255, 255, 255, 0};

  Serial.begin(115200);
  Serial.println();
  Serial.print("Setting");
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println(WiFi.softAP(ssid, "", 1, 0, 1) ? "Ready" : "Failed!");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  //END

  //SENDING WEBPAGE TO IP
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  //GET 'X' AXIS OF ANALOG JOY STICK
  server.on("/D", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT))
    {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      Direction = inputMessage.toInt();
      Serial.print("RECEIVE : ");
      Serial.println(Direction);
    }
    else
      inputMessage = "No message sent";
    request->send(200, "text/plain", "OK");
  });

  //GET SLIDER VALUE
  server.on("/slider", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT))
    {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      sliderValue = inputMessage;
      S = sliderValue.toInt();
    }
    else
      inputMessage = "No message sent";
    request->send(200, "text/plain", "OK");
  });

  //START SERVER.
  server.begin();
}

void loop()
{

  switch (Direction)
  {
  case Stop:
    analogWrite(MAPWM, 0);
    analogWrite(MBPWM, 0);
    break;
  case Forward:
    digitalWrite(MA, LOW);
    digitalWrite(MB, HIGH);
    analogWrite(MAPWM, Speed);
    analogWrite(MBPWM, Speed);
    break;
  case Backward:
    digitalWrite(MA, HIGH);
    digitalWrite(MB, LOW);
    analogWrite(MAPWM, Speed);
    analogWrite(MBPWM, Speed);
    break;
  case Left:
    digitalWrite(MA, LOW);
    digitalWrite(MB, LOW);
    analogWrite(MAPWM, Speed);
    analogWrite(MBPWM, Speed);
    break;
  case Right:
    digitalWrite(MA, HIGH);
    digitalWrite(MB, HIGH);
    analogWrite(MAPWM, Speed);
    analogWrite(MBPWM, Speed);
    break;
  }
}