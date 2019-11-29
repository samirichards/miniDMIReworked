#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define _stackSize (6748/4)

//Display specific libraries
#include <Adafruit_GFX.h> 
#include <Adafruit_SSD1306.h>
#define OLED_RESET -1
#define OLED_SCREEN_I2C_ADDRESS 0x3C
Adafruit_SSD1306 display(OLED_RESET);
#define OLED_SCREEN_I2C_ADDRESS 0x3C


//Settings for DMI
const char* ssid = "Sami's iPhone";
const char* password = "password101";
std::string stationCode = "RUN";
//End of settings

BearSSL::WiFiClientSecure secureClient;
HTTPClient http;

//Defining train service class
class TrainService
{
private:
    /* data */
public:
    String stationName;
    String originStation;
    String destination;
    String operatorName;
    String operatorCode;
    String nrccMessage;
    String platform;
    String cancelReason;
    String delayReason;
    String std;
    String etd;
    String callingPoints;
};

//Allow this to be global so as to be accessible to any method which needs it
//I was tempted to make this an array to have a provision for showing more services but that would complicate things and would be impossible anyway (not enough ram)
TrainService service;

//Methods to do with networking---------

//This method handles connecting to a WiFi network, displaying connection progress on the display
void connectToNetwork() {
  WiFi.begin(ssid, password);
  display.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    display.print(".");
    display.display();
    delay(2000);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connected");
  display.print("Code: ");
  display.print(stationCode.c_str());
  display.print("\n");
  display.display();
  Serial.println(WiFi.localIP());
  delay(1000);
}

//This method gets the next 2 departures from the selected station
bool getDepartures(){
  //Only proceed if the WiFi connection is still alive
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("About to connect to server");
    Serial.println(("https://api.departureboard.io/api/v2.0/getDeparturesByCRS/"+stationCode+"/?apiKey=43962b41-cb16-4109-9018-88a7779a037c&numServices=1&serviceDetails=true").c_str());
    //Begin the http transaction to the server
    secureClient.setInsecure();
    http.begin(secureClient, ("https://api.departureboard.io/api/v2.0/getDeparturesByCRS/"+stationCode+"/?apiKey=43962b41-cb16-4109-9018-88a7779a037c&numServices=1&serviceDetails=true").c_str());
    delay(500);
    int httpCode = http.GET();

    //If the transaction was successful then begin parsing the json document
    if (httpCode > 0){
      yield();
      DynamicJsonDocument doc(5120);
      String data = http.getString();
      Serial.println(data);
      DeserializationError err = deserializeJson(doc, data);
      data = "";
      yield();
      http.end();
      //Print debug messages if the deserialization was not completed successfully
      if (err) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());
      }
      //Parsing code goes here
      //-----------------------------------------
      //TODO
      //Make a backup of this first, it works wholy thanks to luck
      //Never make an attempt to modify any of the code written here right now, or the http gods will smite you down
      //Make a new function which gets the document here by reference and decodes it into some objects to describe the upcoming services and their stops, as well as other tings you might need
      //Make sure the program disposes of the json document after that since it hogs memory
      //After that make it so that the program displays this information from the objects and refreshes them after a minute or so, but be very liberal and don't really care if it is over a minute (It's not critical that this is 100%)
      //Then make it so that if there are changes to expected time there is a beep or something
      //Oh yeah another thing is make sure there is a way to display all of the information you need on that tiny screen (Such as switching to times and such once every 5 seconds)
      //I think that's it for now, I'm going to bed
      //END OF TODO
      //-----------------------------------------
      return true;
    }
    //If the transaction wasn't successful then print the error code and end the connection
    else{
      Serial.println("\n client.connect() returned");
      Serial.print(httpCode);
      http.end();
      return false;
    }
  }
  //Print debug information that the WiFi connection is not allive still
  else{
    Serial.println("Cannot run getDepartures, WiFi is no longer connected");
    return false;
  }
}

//End of networking methods---------------

void populateService(DynamicJsonDocument &input){
  service.stationName = input["locationName"].as<String>();
  Serial.println(service.stationName);
  service.originStation = input["trainServices"]["0"]["origin"]["0"]["locationName"].as<String>();
  Serial.println(service.originStation);
  service.destination = input["trainServices"]["0"]["destination"]["0"]["locationName"].as<String>();
  Serial.println(service.destination);
  service.operatorName = input["trainServices"]["0"]["operator"].as<String>();
  Serial.println(service.operatorName);
  service.operatorCode = input["trainServices"]["0"]["operatorCode"].as<String>();
  Serial.println(service.operatorCode);
  service.nrccMessage = input["nrccMessages"]["0"]["message"].as<String>();
  Serial.println(service.nrccMessage);
  service.platform = input["trainServices"]["0"]["platform"].as<String>();
  Serial.println(service.platform);
  service.cancelReason = input["trainServices"]["0"]["cancelReason"].as<String>();
  Serial.println(service.cancelReason);
  service.delayReason = input["trainServices"]["0"]["delayReason"].as<String>();
  Serial.println(service.delayReason);
  service.std = input["trainServices"]["0"]["std"].as<String>();
  Serial.println(service.std);
  service.etd = input["trainServices"]["0"]["etd"].as<String>();
  Serial.println(service.etd);
  for (JsonArray::iterator it=input["trainServices"]["0"]["subsequentCallingPointsList"]["0"]["subsequentCallingPoints"].as<JsonArray>().begin(); it!=input["trainServices"]["0"]["subsequentCallingPointsList"]["0"]["subsequentCallingPoints"].as<JsonArray>().end(); ++it) {
    service.callingPoints = service.callingPoints + it["locationName"].as<String>().c_str() + "(" + it["st"].as<String>().c_str() + "), ";
    //NEED TO GET THIS CONCATINATION WORKING
  }
  Serial.println(Service.callingPoints);
}


void setup() {
  // -- OLED -------------
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_I2C_ADDRESS);
  display.display();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextWrap(false);
  // a line is 21 chars in this size
  
  connectToNetwork();
  getDepartures();
}

void loop() {
  // put your main code here, to run repeatedly:

}
