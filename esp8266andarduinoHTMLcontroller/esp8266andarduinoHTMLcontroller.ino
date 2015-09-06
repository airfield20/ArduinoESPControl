  #include <SoftwareSerial.h>
 
#define DEBUG true
#define hbridge1 9
#define hbridge2 10
SoftwareSerial esp8266(2,3); // make RX Arduino line is pin 2, make TX Arduino line is pin 3.
                             // This means that you need to connect the TX line from the esp to the Arduino's pin 2
                             // and the RX line from the esp to the Arduino's pin 3
void setup()
{
  Serial.begin(9600);
  esp8266.begin(9600); // your esp's baud rate might be different
  
  pinMode(11,OUTPUT);
  digitalWrite(11,LOW);
  
  pinMode(12,OUTPUT);
  digitalWrite(12,LOW);
  
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);

  pinMode(10,OUTPUT);
  digitalWrite(10,LOW);

  pinMode(9,OUTPUT);
  digitalWrite(9,LOW);

  initESP();
  
}
 
void loop()
{
  if(esp8266.available()) // check if the esp is sending a message 
  {
 
    
    if(esp8266.find("+IPD,"))
    {
     delay(1000); // wait for the serial buffer to fill up (read all the serial data)
     // get the connection id so that we can then disconnect
    //Serial.println("READ IN: " + esp8266.read()-48);
     int connectionId = esp8266.read()-48; // subtract 48 because the read() function returns 
                                           // the ASCII decimal value and 0 (the first decimal number) starts at 48
          
     esp8266.find("pin="); // advance cursor to "pin="
     
     int commandID = (esp8266.read()-48)*10; // get first number i.e. if the pin 13 then the 1st number is 1, then multiply to get 10
     commandID += (esp8266.read()-48); // get second number, i.e. if the pin number is 13 then the 2nd number is 3, then add to the first number
     Serial.println("Read In: " + commandID);
     //digitalWrite(commandID, !digitalRead(commandID)); // toggle pin    
     //digitalWrite(commandID-3, !digitalRead(commandID-3));
     switch(commandID){
      case 12:
        openBlinds();
      break;
      case 13:
        closeBlinds();
      break;
      case 14:
        initESP();
      break;
     }
     
     // make close command
     String closeCommand = "AT+CIPCLOSE="; 
     closeCommand+=connectionId; // append connection id
     closeCommand+="\r\n";
     
     sendData(closeCommand,1000,DEBUG); // close connection
    }
  }
}
 
/*
* Name: sendData
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    
    esp8266.print(command); // send the read character to the esp8266
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
    }
    
    return response;
}

void initESP(){
  digitalWrite(13,HIGH);
  sendData("AT+RST\r\n",2000,DEBUG); // reset module
  sendData("AT+CWJAP=\"Co_Chill Network (2.4)\",\"760-mph-1220-kmh-m1\"\n",4000,DEBUG); //connects to wifi network
  delay(5000);
  sendData("AT+CWMODE=3\r\n",1000,DEBUG);// configure as access point
  delay(5000); //delay allowing for connectivity 
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  //sendData("AT+CIPSTART=\"TCP\",\"192.168.1.73\",\"1234\"\n",1000,DEBUG); //starts tcp port 1234 with static ip
  
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80
  digitalWrite(13,LOW);
  delay(1000);

  //sendData("AT+CIPSEND=4\n",1000,DEBUG);
  //sendData("1234567\n",1000,DEBUG);
}

void openBlinds(){
digitalWrite(hbridge1,HIGH);
digitalWrite(hbridge2,LOW);
delay(6000);
digitalWrite(hbridge1,LOW);
digitalWrite(hbridge2,LOW);
sendData("GET http://192.168.1.73:1234/?openingTheBlinds HTTP/1.0\r\n\r\n\r\n",1000,DEBUG);
delay(3000);
sendData("AT+CIPCLOSE", 1000, DEBUG);
}

void closeBlinds(){
digitalWrite(hbridge1,LOW);
digitalWrite(hbridge2,HIGH);
delay(6000);
digitalWrite(hbridge1,LOW);
digitalWrite(hbridge2,LOW);
}

