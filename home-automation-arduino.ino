#include <SPI.h>
#include <Ethernet.h>
#include <HTTPserver.h>

const int LOW_PIN = 3;
const int HIGH_PIN = 5;

const char token[] = "059b9576-89ea-468e-81fb-564d1331055c";

// Enter a MAC address and IP address for your controller below.
byte mac[] = {  0x90, 0xA2, 0xDA, 0x00, 0x2D, 0xA1 };

// The IP address will be dependent on your local network:
byte ip[] = { 192, 168, 1, 117 };

// the router's gateway address:
byte gateway[] = { 192, 168, 1, 1};

// the subnet mask
byte subnet[] = { 255, 255, 255, 0 };

int isTokenValid = 0;

// Initialize the Ethernet server library
EthernetServer server(3000);

// derive an instance of the HTTPserver class with custom handlers
class myServerClass : public HTTPserver
  {
  virtual void processPostType        (const char * key, const byte flags);
  virtual void processPostArgument    (const char * key, const char * value, const byte flags);
  virtual void processHeaderArgument  (const char * key, const char * value, const byte flags); 
  virtual void processGetArgument     (const char * key, const char * value, const byte flags);
  };  // end of myServerClass

myServerClass myServer;

// -----------------------------------------------
//  User handlers
// -----------------------------------------------

void myServerClass::processPostType (const char * key, const byte flags)
  {
  println(F("HTTP/1.1 200"));
  println(F("Content-Type: text/json\n"
            "Connection: close\n"
            "Server: HTTPserver/1.0.0 (Arduino)"));
  println();   // end of headers
  } // end of processPostType

void myServerClass::processPostArgument (const char * key, const char * value, const byte flags)
  {
    if(isTokenValid == 1){
       if (memcmp (key, "pin_", 4) == 0 && isdigit (key [4]))
        {
        int which = atoi (&key [4]);
        if (which >= LOW_PIN && which <= HIGH_PIN)
          if(atoi (&value [0]) == 0) { //Click
            digitalWrite(which, LOW);
            delay(500);
            digitalWrite(which, HIGH); 
          } 
          else if(atoi (&value [0]) == 1){ //Hold
              if(digitalRead(which) == HIGH){
                digitalWrite(which, LOW);  
              }
              else{
                digitalWrite(which, HIGH);  
              }
          }
        }
      }
  }

void myServerClass::processGetArgument (const char * key, const char * value, const byte flags)
  {
    if (strcmp (key, "pin") == 0){
      int which = atoi (value);
      int pinRead = 0;
      if(which == 0){
        print (F("["));
        for(int i = LOW_PIN; i<= HIGH_PIN; i++){
          Serial.print("asd");
          print (F("{"));
          pinRead = digitalRead(i);
            if(pinRead == HIGH){
              print (F("\"pin\":""\""));
              print(i);
              print(F("\","));
              print (F("\"status\":""\"HIGH\""));
            }
            else if(pinRead == LOW){
              print (F("\"pin\":""\""));
              print(i);
              print(F("\","));
              print (F("\"status\":""\"LOW\""));  
            }
            if(i == HIGH_PIN){
              print (F("}"));
            }else{
              print (F("},"));
            }
          }
         print (F("]"));
      }
      else if (which >= LOW_PIN && which <= HIGH_PIN)
        {
          print (F("{"));
          if(digitalRead(which) == HIGH){
            print (F("\"status\":""\"HIGH\""));
          }
          else if(digitalRead(which) == LOW){
            print (F("\"status\":""\"LOW\""));  
          }
          print (F("}"));
        }
      else
        {
        print (F("\"error\":""\"bad pin number\""));
        } 
    }
  }
  
void myServerClass::processHeaderArgument  (const char * key, const char * value, const byte flags)
{
  if(memcmp (key, "Authorization", 13) == 0 && memcmp(value, token, sizeof(token)) == 0)
  {
      isTokenValid = 1;
  }
}

// -----------------------------------------------
//  End of user handlers
// -----------------------------------------------

void setup ()
  {
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip, gateway, subnet);
  Serial.begin(9600);
  for (int i = LOW_PIN; i <= HIGH_PIN; i++){
    pinMode (i, OUTPUT);
    digitalWrite(i, HIGH);
    }
    
  }  // end of setup



void loop ()
  {
  EthernetClient client = server.available();
  if (!client)
    {    return;
    }

  myServer.begin (&client);
  while (client.connected() && !myServer.done)
    {
    while (client.available () > 0 && !myServer.done)
      myServer.processIncomingByte (client.read ());

    }  // end of while client connected

   if(isTokenValid == 0){
      myServer.println(F("\"error\":""\"unauthorized\""));
    }
   if(isTokenValid == 1){
      isTokenValid = 0;
    }
  myServer.flush ();

  // give the web browser time to receive the data
  delay(1);
  // close the connection:
  client.stop();

  }  // end of loop
