#define BLYNK_TEMPLATE_ID "TMPL3FDWGPaNG"
#define BLYNK_TEMPLATE_NAME "Iot project"
#define BLYNK_AUTH_TOKEN "key/token"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <NewPing.h>

// Blynk auth token from your email
char auth[] = "key/token";  
char ssid[] = "1122";   // WiFi ID
char pass[] = "123456789"; // WiFi password

// Ultrasonic sensor pins
#define TRIG_PIN 13
#define ECHO_PIN 12
#define MAX_DISTANCE 400  // Maximum distance (in cm)

// Relay pin
#define RELAY_PIN 14

// Flow sensor pins
#define FLOW_SENSOR_1_PIN 26  
#define FLOW_SENSOR_2_PIN 27

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);  // NewPing setup
BlynkTimer timer;  // Blynk timer to send sensor data periodically

bool isRelayOn = false;  // To track relay state
unsigned long lastCheckTime = 0;  // To store the last check time
const unsigned long checkInterval = 1000L;  // **1 second** check interval for testing

// Function to measure flow sensor values (placeholder, implement actual logic)
float measureFlow(int pin) {
  int pulseCount = pulseIn(pin, HIGH);  // Example pulse reading
  return pulseCount * 0.1;  // Example conversion to flow rate (L/min)
}

// Function to control relay based on Blynk button
BLYNK_WRITE(V3) {
  isRelayOn = param.asInt();  // Get button value (0 or 1)
  digitalWrite(RELAY_PIN, isRelayOn ? LOW : HIGH);  // Inverted logic: LOW turns ON, HIGH turns OFF
  Blynk.virtualWrite(V2, isRelayOn ? "Motor is on" : "Motor is off");
  Serial.println(isRelayOn ? "Relay manually turned ON via Blynk" : "Relay manually turned OFF via Blynk");
}

// Function to send sensor data to Blynk
void sendSensorData() {
  unsigned long currentTime = millis();

  // Check if 1 second has passed (for testing)
  if (currentTime - lastCheckTime >= checkInterval) {
    lastCheckTime = currentTime;  // Update last check time

    // Measure distance
    int distance = sonar.ping_cm();
    if (distance == 0) {
      Serial.println("No object detected or out of range");
      Blynk.virtualWrite(V1, distance);
    } else {
      Blynk.virtualWrite(V1, distance);
      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.println(" cm");

      // Automatic relay control based on distance
      if (!isRelayOn && distance > 10) {
        digitalWrite(RELAY_PIN, LOW);  // Inverted logic: LOW turns ON
        Blynk.virtualWrite(V2, "Motor is on");
        Serial.println("Relay automatically turned ON (Distance > 10 cm)");
      } else if (!isRelayOn && distance <= 10) {
        digitalWrite(RELAY_PIN, HIGH);  // Inverted logic: HIGH turns OFF
        Blynk.virtualWrite(V2, "Motor is off");
        Serial.println("Relay automatically turned OFF (Distance <= 10 cm)");
      }
    }

    // Measure flow sensors
    float flow1 = measureFlow(FLOW_SENSOR_1_PIN) * 0.001;
    float flow2 = measureFlow(FLOW_SENSOR_2_PIN) * 0.001;

    // Send flow sensor data to Blynk virtual pins (for chart display)
    Blynk.virtualWrite(V4, flow1);  // Flow sensor 1 to virtual pin V4
    Blynk.virtualWrite(V5, flow2);  // Flow sensor 2 to virtual pin V5

    // Debugging output for flow sensors
    Serial.print("Flow 1: ");
    Serial.print(flow1);
    Serial.println(" L/min");

    Serial.print("Flow 2: ");
    Serial.print(flow2);
    Serial.println(" L/min");
  }
}

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);

  // Set relay and flow sensor pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FLOW_SENSOR_1_PIN, INPUT);
  pinMode(FLOW_SENSOR_2_PIN, INPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Keep the relay off initially (inverted logic)

  // Connect to WiFi and Blynk
  Blynk.begin(auth, ssid, pass);

  // Setup a function to run every second to check if it's time to send sensor data
  timer.setInterval(1000L, sendSensorData);
}

void loop() {
  // Run Blynk
  Blynk.run();
  
  // Run the timer
  timer.run();
}
