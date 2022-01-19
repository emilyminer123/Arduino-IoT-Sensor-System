
/*
 *
 * E-121-C
 * Bella Moretti, Emily Miner, Philip Mascaro
 * Group C-5
 * 10/18/2020
 */


#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include "info.h"
#include "WiFiManager.h"
#include "DHT.h"

  //The ESP8266 recognizes different pins than what is labelled on the WeMos D1 
  #if defined(d1)  //Defines Wemos D1 R1 pins to GPIO pins
    #define D0 3
    #define D1 1
    #define D2 16
    #define D8 0
    #define D9 2
    #define D5 14
    #define D6 12
    #define D7 13
    #define D10 15
  #endif 
  #if defined(d1_mini) //Defines Wemos D1 R2 pins to GPIO pins
    #define A0 0
    #define D0 16
    #define D1 5
    #define D2 4
    #define D3 0
    #define D4 2
    #define D5 14
    #define D6 12
    #define D7 13
    #define D8 15
  #endif

//Set up the DHT11
#define DHTPIN D7  //PLUG THE DHT 11 ONLY INTO D5, D6, D7 on either D1-R1 or D1-R2 Board. 
                   // DO NOT PLUG INTO OTHERS AS THE MAPPING IS NOT THE SAME. DHT11 WILL BURN. 
#define DHTTYPE DHT11
DHT dht(DHTPIN,DHTTYPE);


//This begins the configuration of the wifi settings.
const char* ssid = "redacted";
const char* password = "redacted";

 //These establish the variable for the blinking light. 
 int const TIME_LIGHT_SHINES = 1000;


/* The cycle_number variable is set to 0, however,
 * since this variable is not constant it will change. */
  int cycle_number = 0;

//MQTT Settings
const char* mqtt_server = "155.246.62.110";
const char* MQusername = "jojo";
const char* MQpassword = "hereboy";

//MQTT Publish Topics (replace the X’s with the MAC of your board)
const char* MQtopic1 = "E121/XXXX/Temperature";
const char* MQtopic2 = "E121/XXXX/Humidity";
const char* MQtopic3 = "E121/XXXX/Light";

//MQTT Data Model
const int DataModel = 1; // 1: Sin others are not implement
const int DataTempMin = 20;
const int DataTempMax = 50; // Make sure max > min.
const int DataHumMin = 30;
const int DataHumMax = 60; // Make sure max > min.
const int DataCyclePeriod = 2; // In minutes

//Define variables relevant to connecting the client to LabView Data.
WiFiClient espClient;
info board_info;
PubSubClient client(espClient);
long lastMsg = 0;
char msg1[20],msg2[20],msg3[20];
int value = 0;
float temp, hum,freq, light;
int tm = 0;
int dt,ta,ha;

//This sets up the wifi connection and checks the wifi status. 
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//Confirm that the connection was successful
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

/* The purpose of this is to check whether
 * the client is connected and reconnects if not.
 * If the client is connected it will say connected,
 * and if not it will say that the connection has failed
 * and will try to reconnect in 5 seconds. This will 
    keep looping until the client is connected.*/
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // This creates a random client ID.
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    //Attempt to connect.
    if (client.connect(clientId.c_str(),MQusername,MQpassword)) {
//    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Five second delay before trying again.
      delay(5000);
    }
  }
}


/*This sets up the BUILTIN_LED pin as an output and prints out the Mac
 * address, while also setting up the mqtt server and initializing 
 * the LED light on the Wemos board. */
void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  //Serial.begin(115200);
  Serial.begin(9600);
  delay(3000);
  Serial.println("Wemos POWERING UP ......... ");
  Serial.print("Mac Address:");
  Serial.println(board_info.mac());
  setup_wifi();
  dht.begin();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

/*This will continue to loop and check
 * whether the client is connected as the program is 
 * running. If disconnected it will try to reconnect. */
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  dt = now - lastMsg ;
  if (dt > 2000) {
  }
   setup_dhtt();

   
     /*This code causes the light to blink for two seconds and prints out on the Serial Monitor that the code is running.
     * everytime this code loops.  */
      digitalWrite(LED_BUILTIN, LOW);
      delay(TIME_LIGHT_SHINES);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(TIME_LIGHT_SHINES);   
      Serial.println("running");
}


void setup_dhtt()
{
    Serial.println("Temperature and Humidity");
    temp = dht.readTemperature(true);
    hum = dht.readHumidity();
    light = (float) analogRead(A0);
    light = light/310;
    snprintf (msg1, 20, "%d", (int) temp);
    snprintf (msg2, 20, "%d", (int) hum);
    snprintf (msg3, 20, "%f", light);
    Serial.println(msg1);
    Serial.println(msg2);
    Serial.println(msg3);
    delay(5000);
    
    Serial.print("Published :" );
    Serial.print(MQtopic1);
    Serial.print(" with value: " );
    Serial.println(msg1);
    client.publish(MQtopic1, msg1);
    Serial.print("Published :" );
    Serial.print(MQtopic2);
    Serial.print(" with value: " );
    Serial.println(msg2);
    client.publish(MQtopic2, msg2);
    Serial.print("Published :" );
    Serial.print(MQtopic3);
    Serial.print(" with value: " );
    Serial.println(msg3);
    client.publish(MQtopic3, msg3);

}
