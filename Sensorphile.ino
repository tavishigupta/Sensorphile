#include <ESP8266WiFi.h>

const char* ssid = "UWIOTLAB#2"; // SSID of the router 
const char* password = ""; // password to connect to the internet

const int httpPort = 80; // port 80 is used for HTTP traffic
const char* host = "4911c0a1.ngrok.io"; // host connected to

const int sampleWindow = 1000; // sample window width in mS (50 mS = 20Hz)
unsigned int sample; // reading from the sound sensor

String PostData; // variable to store the data to be posted
double volts; // the average volts

void setup() {
  Serial.begin(115200); // set the baud rate
  WiFiClient client; // create new WiFiClient

  // connect to WiFi network using SSID and password
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  // checks if connected ti WiFi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); // prints the local IP address
}

void loop() {
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  unsigned long startMillis= millis();  // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level
 
  unsigned int signalMax = 1024; // maximum signal collected
  unsigned int signalMin = 0; // minimum signal collected
 
  // collect data for 50 mS
  while (millis() - startMillis < sampleWindow) {
    sample = analogRead(A0); // collect the analog data from the sensor
    
    if (sample < 1024) { // toss out spurious readings
      if (sample > signalMax) {
        signalMax = sample;  // save just the max levels
      }
      else if (sample < signalMin) {
        signalMin = sample;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  volts = (peakToPeak * 5.0) / 1024;  // convert to volts
 
  Serial.println(volts); // print average of collected voltage

  // post data if average voltage is between 4.0 and 6.0
  if(volts > 4.0 & volts < 6.0){
    PostData = (String)volts; // cast volts as string
    // if connected to the host, post volts to the REST API
    if (client.connect(host, httpPort)) {
      client.println(String("POST ") + "/decibel/testText HTTP/1.1");
      client.println("Host: 4911c0a1.ngrok.io");
      client.println("Cache-Control: no-cache");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.print("Content-Length: ");
      client.println(PostData.length());
      client.println();
      client.println(PostData);
      
      // Read all the lines of the reply from server and print them to Serial
      while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
      }
    }
  }

  // print close connection
  Serial.println();
  Serial.println("closing connection");
}
