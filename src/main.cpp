#include <WiFi.h>
#include <ESP32Servo.h>
#include <WebServer.h>

const char* WIFI_SSID = "Berk";
const char* WIFI_PASSWORD = "barisberk";

const int SERVO_PIN = 18; // Servo pin
const int LED_PIN = 33; // Button for LED
const int BUTTON_PIN = 16;  // Button Pin for servo
Servo myservo;
int currentAngle = 0;
bool ledState = false;
bool lastButtonState = HIGH; // Last version of Button, in the beginning HIGH (button is not toggled)

WebServer server(80);

void handleRoot() {
  if (server.hasArg("angle")) {
    int angle = server.arg("angle").toInt();
    currentAngle += angle;
    if (currentAngle > 180) {
      currentAngle = 0;
    }
    myservo.write(currentAngle);
    server.send(200, "text/plain", "Angle set to " + String(currentAngle));
  } else {
    server.send(400, "text/plain", "Angle parameter missing");
  }
}

void handleGetAngle() {
  server.send(200, "text/plain", String(currentAngle));
}

void handleLed() {
  if (server.hasArg("state")) {
    String state = server.arg("state");
    if (state == "on") {
      digitalWrite(LED_PIN, HIGH);
      ledState = true;
    } else if (state == "off") {
      digitalWrite(LED_PIN, LOW);
      ledState = false;
    } else if (state == "toggle") {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    }
    server.send(200, "text/plain", "LED is " + state);
  } else {
    server.send(400, "text/plain", "State parameter missing");
  }
}

// Sending HTTP request to Node-Red about the LED 
void notifyNodeRED(const char* state) {
  WiFiClient client;
  if (client.connect("172.20.10.2", 1880)) {  // Node-RED's IP adress
    String url = "/ledState?state=" + String(state);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: 172.20.10.2\r\n" +  // Node-RED's IP adress
                 "Connection: close\r\n\r\n");
    delay(10);
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  }
  client.stop();
}

void setup() {
  Serial.begin(115200);
  myservo.attach(SERVO_PIN);
  myservo.write(currentAngle);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // enable pull-up feature for button pin 

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/angle", handleGetAngle);
  server.on("/led", handleLed);
  server.begin();
}

void loop() {
  server.handleClient();

  // Checking the button's status 
  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) { // if button is toggled
    if (ledState) {
      digitalWrite(LED_PIN, LOW);
      ledState = false;
      Serial.println("LED is off");
      notifyNodeRED("off");
    } else {
      digitalWrite(LED_PIN, HIGH);
      ledState = true;
      Serial.println("LED is on");
      notifyNodeRED("on");
    }
    delay(50); // short delay for Debounce 
  }
  lastButtonState = buttonState;
}
