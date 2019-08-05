/* Select ESP8266 Genaric */

#include <ESP8266WiFi.h>     
#include <PubSubClient.h>    
#include <DHT.h>      
                      
#define DHTPIN 2     // what pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Initialize DHT sensor for normal 16mhz Arduino
//DHT dht(DHTPIN, DHTTYPE);
// Example to initialize DHT sensor for ESP8266:
DHT dht(DHTPIN, DHTTYPE, 20);

int debug = 0;              /* Set this to 1 for serial debug output */
const char *ssid =   "your_ssid";            //Replace with your Wifi SSID
const char *pass =   "your_wifi_password";   //Replace with your Wifi password
const char *mqtt_server = "xxx.xxx.xxx.xxx"; // Set to our MQTT broker IP
unsigned long previousMillis = 0;      
unsigned long interval = 60000;      
WiFiClient espTemp;
PubSubClient client(espTemp);   // Unique per broker

void setup(){
  Serial.begin(9600);
  delay(10);
  debug and Serial.println();
  debug and Serial.println();
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  // We start by connecting to a WiFi network
  WiFi.disconnect();
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);         /* Client only */
  WiFi.begin(ssid, pass);
  int retries = 0;
  int tries = 60;
  //Wait for Wifi connection for 5 minutes before resetting ESP.
  while ((WiFi.status() != WL_CONNECTED) && (retries < tries)) {
    retries++;
    delay(5000);
    Serial.print(".");
  } 
  if (retries >= tries) {
    Serial.print("Reset ESP, No wifi");
    ESP.restart();
  }
  debug and Serial.println("");
  debug and Serial.println("WiFi connected");
  debug and Serial.println("IP address: ");
  debug and Serial.println(WiFi.localIP());
}

void reconnect() {
  int retries = 0;
  int tries = 60;
  // Loop every 5 seconds until MQTT broker is connected. After 5 minutes check Wifi.
  while ((!client.connected()) && (retries < tries)) { 
    retries++;
    debug and Serial.print("Attempting MQTT connection... ");
    // Attempt to connect
    if (client.connect("espTemp")) {
      debug and Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  } 
  // Reconnect to wifi if we cannot get to MQTT Broker
  if (retries >= tries) {
    Serial.println("Could not establish MQTT connection. Reconnect WiFi.");
    setup_wifi();
  }
}

void loop(){
  client.loop();
  if (!client.connected()) {
    delay(10);
    reconnect();
  }
  unsigned long currentMillis = millis();
    if(currentMillis - previousMillis > interval) {
      Serial.print(" "); //Blink the LED
      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      float h = dht.readHumidity();
      // Read temperature as Celsius
      //float t = dht.readTemperature();
      // Read temperature as Fahrenheit
      float f = dht.readTemperature(true);
        // Check if any reads failed and exit early (to try again).
        if (isnan(h) || isnan(f)) {
          Serial.println("Failed to read from DHT sensor!");
          return;
        }
      previousMillis = currentMillis;
      char buffer[10];
      dtostrf(f,0, 1, buffer);
      debug and Serial.println("DTH sensor read and transmitted: ");
      debug and Serial.print(buffer);
      debug and Serial.println("F");
      client.publish("OpenHab/weather/temperature",buffer);
      dtostrf(h,0, 1, buffer);
      debug and Serial.print(buffer);
      debug and Serial.println("%");
      client.publish("OpenHab/weather/humidity",buffer);
  }
  delay(20);
} 
