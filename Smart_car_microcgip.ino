// v1.3
//Library including
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

//SET SSID HERE
#define ssid "Smart_car_01" //SET SSID HERE
//SET SSID HERE

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
<!DOCTYPE html>
<html>

<head>
    <style>
        .button {
            background-color: #990033;
            border: none;
            border-radius: 4px;
            color: white;
            padding: 30px 30px;
            font-size: 20px;
            text-align: center;
            direction: none;
            text-decoration: none;
        }
    </style>
    <meta charset="UTF-8" http-equiv="Content-Type" content="text/html">
    <meta name="viewport" content="width=device-width, initial-scale=1,height = device-height">
    <title>Control_car</title>
</head>

<body onload='init()'>
    <center>
        <h1>Car control</h1>
        <br> \
        <input type="button" class="button" id="btnf" value="forward">
        <br><br>
        <input type="button" class="button" id="btnl" value="<< Left &nbsp;">
        <input type="button" class="button" id="btnr" value="Right >>">
        <br><br>
        <input type="button" class="button" id="btnb" value="backward">
        <div id="status">...</div>
    </center>
</body>
<script type="text/javascript">function init() {

        document.getElementById('btnf').addEventListener('touchstart', movef, false);
        document.getElementById('btnf').addEventListener('touchend', stopcar, false);

        document.getElementById('btnl').addEventListener('touchstart', movel, false);
        document.getElementById('btnl').addEventListener('touchend', stopcar, false);

        document.getElementById('btnr').addEventListener('touchstart', mover, false);
        document.getElementById('btnr').addEventListener('touchend', stopcar, false);

        document.getElementById('btnb').addEventListener('touchstart', moveb, false);
        document.getElementById('btnb').addEventListener('touchend', stopcar, false);

    } function move(dir) {
        document.getElementById('status').innerHTML = dir;
        var requst = new XMLHttpRequest();
        requst.open('GER', '/engines/' + dir, true);
        requst.send(null);
    }
    function movef() {
        move('f');
    }
    function movel() {
        move('l');
    } function mover() {
        move('r');
    }
    function moveb() {
        move('b');
    } function stopcar() {
        document.getElementById('status').innerHTML = 's';
        var requst = new XMLHttpRequest();
        requst.open('GER', '/engines/s',true);
        requst.send(null);
    }
</script>
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
  server.on("/engines/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT))
    {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      Direction = inputMessage.toInt();
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
  Serial.println(Direction);
  // switch (Direction)
  // {
  // case Stop:
  //   analogWrite(MAPWM, 0);
  //   analogWrite(MBPWM, 0);
  //   break;
  // case Forward:
  //   digitalWrite(MA, HIGH);
  //   digitalWrite(MB, HIGH);
  //   analogWrite(MAPWM, Speed);
  //   analogWrite(MBPWM, Speed);
  //   break;
  // case Backward:
  //   digitalWrite(MA, LOW);
  //   digitalWrite(MB, LOW);
  //   analogWrite(MAPWM, Speed);
  //   analogWrite(MBPWM, Speed);
  //   break;
  // case Left:
  //   digitalWrite(MA, LOW);
  //   digitalWrite(MB, HIGH);
  //   analogWrite(MAPWM, Speed);
  //   analogWrite(MBPWM, Speed);
  //   break;
  // case Right:
  //   digitalWrite(MA, HIGH);
  //   digitalWrite(MB, LOW);
  //   analogWrite(MAPWM, Speed);
  //   analogWrite(MBPWM, Speed);
  //   break;
  // }


} // End Loop