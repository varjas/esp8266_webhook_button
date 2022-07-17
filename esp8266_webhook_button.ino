#include <ESP8266WiFi.h>

const char* ssid = "networkName-IoT";
// Plain text password
const char* password = "networkPassword";

// Hostname only, no scheme, no path
const char* host = "subdomain.domain.tld";
const int port = 80;

int request_count = 0;

String urlPath = "/";
String data = "{\"device_id\": \"3BcknMPZG6\"}";

// Use WiFiClient class to create TCP connections
WiFiClient client;

int buttonState = HIGH;
int previousState = LOW;

const int buttonPin = 12;
const int light1 = 5;
const int light2= 4;
const int light3= 13;
const int light4 = 15;
const int lights[] = {light1, light2, light3, light4};

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(light1, OUTPUT);
  pinMode(light2, OUTPUT);
  pinMode(light3, OUTPUT);
  pinMode(light4, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client. Otherwise by default, it
    would try to act as both a client and an access-point and could cause
    network-issues with other WiFi-devices on the network. */
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
  delay(1000);
  cycle_led();
}

void cycle_led() {
  int count = 3;
  digitalWrite(LED_BUILTIN, HIGH);
  for (int i = count; i > 0; i--) {
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    digitalWrite(LED_BUILTIN, HIGH); 
  }
}

void cycle_colors() {
  int count = 3;
  for (int i = count; i > 0; i--) {
    for (int light_index = 3; light_index >= 0; light_index--) {
      digitalWrite(lights[light_index], HIGH);
      delay(30);
      digitalWrite(lights[light_index], LOW);
    }
  }
}

void loop() {
  // Rough debounce to reduce state transition noise
  delay(30);
  buttonState = digitalRead(buttonPin);
  
  // Handle button state transitions
  if (previousState == HIGH && buttonState == LOW) {
    previousState = LOW;
    ++request_count;
    Serial.print("Send request: ");
    Serial.println(request_count);
    
    // send_request();
    digitalWrite(LED_BUILTIN, LOW);
    cycle_colors();
  } else if (previousState == LOW && buttonState == HIGH) {
    previousState = HIGH;
    digitalWrite(LED_BUILTIN, HIGH);
  
  // Continue cycling lights if the button is still pressed
  } else if (previousState == LOW && buttonState == LOW) {
    cycle_colors();
  }
}

void send_request() {
  Serial.print("Seconds up: ");
  Serial.println(int(millis() / 1000));
  Serial.print("Available: ");
  Serial.println(client.available());
  Serial.print("Boolean: ");
  Serial.println(!client.available());
  Serial.print("Connected: ");
  Serial.println(client.connected());

  if (!client.connected()) {
    Serial.print("connecting to ");
    Serial.println(host);
 
    if (!client.connect(host, port)) {
      Serial.println("connection failed");
      return;
    }
  }

  Serial.println(request_count);
  Serial.print("Requesting URL: ");
  Serial.println(urlPath);
  
  // Send request to the server
  client.print(String("GET ") + urlPath + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" + 
    "Connection: keep-alive\r\n"
    "Content-Type: application/json\r\n" +
    "Content-Length: " + data.length() + "\r\n"
    + data + "\n"
  );

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the response lines from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println("closing connection");
  Serial.println();
}
