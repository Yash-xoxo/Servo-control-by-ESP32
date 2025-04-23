#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ssid = "ESP32_ArmControl";
const char* password = "12345678";

WebServer server(80);

// Define servo objects
Servo servos[6];

// Define corresponding GPIO pins for servos
const int servoPins[6] = {12, 13, 14, 25, 26, 17}; // Adjust pins as needed

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Arm Controller</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
  .flex-container {
  display: flex;
  justify-content: center;
  flex-wrap: wrap;
  gap: 20px;
}
.section {
  flex: 1 1 300px;
}
  input[type=range] {
  height: 16px;
  -webkit-appearance: none;
  background: #000000;
  border-radius: 10px;
}
input[type=range]::-webkit-slider-thumb {
  -webkit-appearance: none;
  height: 24px;
  width: 24px;
  background: #4285f4;
  border-radius: 50%;
  cursor: pointer;
}
input[type=range]::-moz-range-thumb {
  height: 24px;
  width: 24px;
  background: #4285f4;
  border-radius: 50%;
  cursor: pointer;
}

    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 20px;
      background: #474747;
      text-align: center;
    }
    h2 {
      margin-bottom: 30px;
    }
    .section {
      background: rgb(153, 153, 153);
      border-radius: 16px;
      box-shadow: 0 4px 10px rgba(0,0,0,0.1);
      margin: 20px auto;
      padding: 20px;
      max-width: 500px;
    }
    .slider-group {
      margin-bottom: 20px;
    }
    .slider-group h4 {
      margin: 10px 0 5px;
    }
    input[type=range] {
      width: 80%;
    }
  </style>
</head>
<body>
  <h2>🤖 ESP32 Dual Arm Controller</h2>
  <div class="flex-container">
    <div class="section">
      <h3>🦾 Left Arm (3 Micro Servos)</h3>
      <div class="slider-group">
        <h4>Servo 1</h4>
        <input type="range" min="0" max="180" value="90" id="servo1" oninput="updateServo(1, this.value)">
        <p>Angle: <span id="val1">90</span>°</p>
      </div>
      <div class="slider-group">
        <h4>Servo 2</h4>
        <input type="range" min="0" max="180" value="90" id="servo2" oninput="updateServo(2, this.value)">
        <p>Angle: <span id="val2">90</span>°</p>
      </div>
      <div class="slider-group">
        <h4>Servo 3</h4>
        <input type="range" min="0" max="180" value="90" id="servo3" oninput="updateServo(3, this.value)">
        <p>Angle: <span id="val3">90</span>°</p>
      </div>
    </div>

    <div class="section">
      <h3>🦿 Right Arm (2 Standard + 1 Micro)</h3>
      <div class="slider-group">
        <h4>Servo 4</h4>
        <input type="range" min="0" max="180" value="90" id="servo4" oninput="updateServo(4, this.value)">
        <p>Angle: <span id="val4">90</span>°</p>
      </div>
      <div class="slider-group">
        <h4>Servo 5</h4>
        <input type="range" min="0" max="180" value="90" id="servo5" oninput="updateServo(5, this.value)">
        <p>Angle: <span id="val5">90</span>°</p>
      </div>
      <div class="slider-group">
        <h4>Servo 6</h4>
        <input type="range" min="0" max="180" value="90" id="servo6" oninput="updateServo(6, this.value)">
        <p>Angle: <span id="val6">90</span>°</p>
      </div>
    </div>
  </div>
  <script>
    function updateServo(id, angle) {
      document.getElementById("val" + id).innerText = angle;
      fetch(`/servo${id}/move?angle=${angle}`);
    }
  </script>
</body>
</html>
)rawliteral";

// Generates HTML for sliders
String generateSlidersHTML() {
  String sliders = "";
  for (int i = 1; i <= 6; i++) {
    sliders += "<div class='slider-container'>";
    sliders += "<h3>Servo " + String(i) + "</h3>";
    sliders += "<input type='range' min='0' max='180' value='90' id='servo" + String(i) +
               "' oninput='updateServo(" + String(i) + ", this.value)'>";
    sliders += "<p>Angle: <span id='val" + String(i) + "'>90</span>°</p>";
    sliders += "</div>";
  }
  return sliders;
}

void handleRoot() {
  String page = htmlPage;
  page.replace("%SLIDERS%", generateSlidersHTML());
  server.send(200, "text/html", page);
}

void handleServoMove(int servoNumber) {
  if (!server.hasArg("angle")) {
    server.send(400, "text/plain", "Missing angle parameter");
    return;
  }

  int angle = server.arg("angle").toInt();
  angle = constrain(angle, 0, 180);
  if (servoNumber >= 1 && servoNumber <= 6) {
    servos[servoNumber - 1].write(angle);
  }
  server.send(200, "text/plain", "Angle set");
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 6; i++) {
    servos[i].setPeriodHertz(50);
    servos[i].attach(servoPins[i]);
    servos[i].write(90);
  }

  WiFi.softAP(ssid, password);
  Serial.println("WiFi AP started. Connect to:");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  for (int i = 1; i <= 6; i++) {
    server.on(("/servo" + String(i) + "/move").c_str(), [i]() {
      handleServoMove(i);
    });
  }
  server.begin();
}

void loop() {
  server.handleClient();
}
