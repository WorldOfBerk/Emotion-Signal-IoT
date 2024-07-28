#include <WiFi.h>
#include <HTTPClient.h>

const char* WIFI_SSID = "Berk";
const char* WIFI_PASSWORD = "barisberk";

const int SERVO_BUTTON_PIN = 16;  // Servo control button
const int LED_BUTTON_PIN = 18;    // LED control button

bool lastServoButtonState = HIGH; // Last state of servo button, at begin HIGH (not toggled)
bool lastLedButtonState = HIGH;   // Last state of LED button, at begin a HIGH (not toggled)

void setup() {
  Serial.begin(115200);
  pinMode(SERVO_BUTTON_PIN, INPUT_PULLUP); // Enable pull-up feature for Servo
  pinMode(LED_BUTTON_PIN, INPUT_PULLUP);   // Enable pull-up feature for LED

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check servo button status
  bool servoButtonState = digitalRead(SERVO_BUTTON_PIN);
  if (servoButtonState == LOW && lastServoButtonState == HIGH) { // If servo button pressed
    // Check servo (Turn 90 degree)
    HTTPClient http;
    http.begin("http://172.20.10.2/?angle=90"); // ESP32 A's IP Address
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
    }
    http.end();

    delay(50); // Short delay for debounce
  }
  lastServoButtonState = servoButtonState;

  // Check LED button state
  bool ledButtonState = digitalRead(LED_BUTTON_PIN);
  if (ledButtonState == LOW && lastLedButtonState == HIGH) { // If LED button pressed
    // Change state of LED
    HTTPClient http;
    http.begin("http://172.20.10.2/led?state=toggle"); // ESP32 A's IP address
    int httpCode = http.GET(); //Get
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
    }
    http.end();

    delay(50); // Short delay for debounce
  }
  lastLedButtonState = ledButtonState;
}