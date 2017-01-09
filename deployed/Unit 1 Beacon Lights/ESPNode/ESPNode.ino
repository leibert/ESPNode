//Hardware definition file
#include <ESP8266WiFi.h>

//WIFI ssid and password in include file
#include <passwords.h>

//Location of ESPHerder
const String masterhost="ioshit.net";
char* host = "192.168.0.31";
const int httpPort=80;


//define channel properties
//CH# = Channel type (RGB, DIGital, DIMmer)
//CH#desc = Alphanumberic Description
//CH#IO array contain IO pin mapping
//CH#STATE array for internal tracking

//Define IoS Node
//description
const char* desc = "Downstairs Alarms";
//Node Short code
const char* nodename = "DWNALARMS";
//# of devices
const int numChannels = 4; //This is needed because C sucks. Should be 1 less than array size


//Channel Array Entry = {TYPE,IOa,IOb,IOc,STATEa,STATEb,STATEc,INVERTFLAG}
//TYPE: 1=Switch,2=PWM, 3=RGB PWM, 11=Digital Input
//RGB uses a,b,c; otherwise only use a
//Invertflag used if HIGH is off
//STATE should be init 0, used to keep track of last values

//First array entry Channel[0] should be all zeros. Channels start at Channel[1] to align with CHnum being passed to function


//Array of Channel Properties
static int Channel[(numChannels + 1)][8];

//Array of Channel Descriptions
static String Channeldesc[(numChannels + 1)];

//Channel Init
void initChannel(int CHID, int type, int pin1, int pin2, int pin3, int flag, String desc) {
  Serial.println("in init");
  Serial.println("initing " + String(CHID) + ": " + desc);
  Channel[CHID][0] = type;  //TYPE CODE
  Channel[CHID][1] = pin1;  //IO PIN A
  Channel[CHID][2] = pin2;  //IO PIN B (for RGB)
  Channel[CHID][3] = pin3;  //IO PIN C (for RGB)
  Channel[CHID][4] = 0;     //Internal State Tracking for PIN A
  Channel[CHID][5] = 0;     //Internal State Tracking for PIN B
  Channel[CHID][6] = 0;     //Internal State Tracking for PIN C
  Channel[CHID][7] = flag;  //Inverted IO flag
  Channeldesc[CHID] = desc;

}

int BLULED = 2; //ESP BLUE LED FOR DEBUGGING



//Setup timeout clock
int mSec = 0;
int seconds = 0;
int minutes = 0;
int timeoutperiod = 120; //timeout in seconds to turn off light, no timeout if 0

bool minuteFLAG = false; //Flag to prevent instruction from firing multiple times a minute


//Timer
int MCLKmsec, MCLKsec, MCLKminutes, MCLKhours;
int TMRmsec, TMRsec, TMRminutes, TMRhours;
int minutehold; //Variable to keep track of minute change

//setup for command processing
int chnum;
String action, value;

//init WiFi propoerties
WiFiClient client;
WiFiServer server(80);




//connect to wifi
void startWIFI() {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //connect to network using cons credentials
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) { //wait for connection
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected"); //Connected to WiFi Network

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

}














////Convert 0-100 value to  0-255 value for Analog Write. Peg low/high to rails
int PWMconvert(int val) {
  int PWM = 0;
  if (val < 5)
    PWM = 0;
  else if (val > 98)
    PWM = 1023;
  else
    PWM = (val / 100.0) * 1023.0;
  return PWM;
}

//void CHFade(int chnum, int newvalue, int decay){
//  if (decay>500)
//    decay=100;
//  int oldvalue = Channel[chnum][4];
//  int PWMval = PWMconvert(newvalue);
//  while(PWMval!=oldvalue){
//    if (PWMval>oldvalue)
//      oldvalue--;
//    else 
//      oldvalue++;
//    if (Channel[chnum][7] == 1) //check inversion flag
//      analogWrite(Channel[chnum][1], (100 - oldval)); //HIGH is off
//    else
//      analogWrite(Channel[chnum][1], oldval); //LOW is OFF
//    Channel[chnum][4]=oldval;
//    delay(decay);
//  }
//}


//void RGBFade(int chnum, int R, int G, int B, int decay) {
//  if (Channel[chnum][0] < 3) {
//    CHFade(chnum,R,decay);
//  }
//  else if(Channel[chnum][0]==3){
//    if (decay>500)
//      decay=100;
//    int oldvalueA = Channel[chnum][4];
//    int oldvalueB = Channel[chnum][4];
//    int oldvalueC = Channel[chnum][4];
//    int PWMvalA = PWMconvert(R);
//    int PWMvalB = PWMconvert(G);
//    int PWMvalC = PWMconvert(B);
//    while((PWMvalA!=oldvalueA)||(PWMvalB!=oldvalueB)||(PWMvalC!=oldvalueC)){
//      if (PWMvalA>oldvalueA)
//        oldvalueA--;
//      else if(PWMvalA<oldvalueA)
//        oldvalueA++;
//
//      if (PWMvalB>oldvalueB)
//        oldvalueB--;
//      else if(PWMvalB<oldvalueB)
//        oldvalueB++;
//
//      if (PWMvalC>oldvalueC)
//        oldvalueC--;
//      else if(PWMvalC<oldvalueC)
//        oldvalueC++;
//        
//      if (Channel[chnum][7] == 1) //check inversion flag
//        analogWrite(Channel[chnum][1], (100 - oldvalA)); //HIGH is off
//        analogWrite(Channel[chnum][2], (100 - oldvalB)); //HIGH is off
//        analogWrite(Channel[chnum][3], (100 - oldvalC)); //HIGH is off
//      else
//        analogWrite(Channel[chnum][1], oldvalA); //LOW is OFF
//        analogWrite(Channel[chnum][2], oldvalB); //LOW is OFF
//        analogWrite(Channel[chnum][3], oldvalC); //LOW is OFF
//      Channel[chnum][4]=oldvalA;
//      Channel[chnum][5]=oldvalB;
//      Channel[chnum][6]=oldvalC;
//      
//      delay(decay);
//    }
//
//  }
//}


//Switch Channel to Full-On
void switchON(int chnum) {
  if (Channel[chnum][0] < 3) { //Check to make sure Channel is single output
    if (Channel[chnum][7] == 1) {//Check inversion flag
      digitalWrite(Channel[chnum][1], LOW); 
    }
    else {
      digitalWrite(Channel[chnum][1], HIGH);
    }
    Channel[chnum][4] = 100; //update state tracking
  }
  else if (Channel[chnum][0] == 3) { //Channel is RGB, all channels need to be set to full on
    if (Channel[chnum][7] == 1) { //check inversion flag
      digitalWrite(Channel[chnum][1], LOW);
      digitalWrite(Channel[chnum][2], LOW);
      digitalWrite(Channel[chnum][3], LOW);
    }
    else {
      digitalWrite(Channel[chnum][1], HIGH);
      digitalWrite(Channel[chnum][2], HIGH);
      digitalWrite(Channel[chnum][3], HIGH);
    }
    Channel[chnum][4] = 100; //update state tracking
    Channel[chnum][5] = 100;
    Channel[chnum][6] = 100;

  }
}

//Switch Channel OFF, set to full off
void switchOFF(int chnum) {

  if (Channel[chnum][0] < 3) {  //Check to see if single output device
    if (Channel[chnum][7] == 1) { //Check inversion flag
      //      analogWrite(Channel[chnum][1], 255);
      digitalWrite(Channel[chnum][1], HIGH);
    }

    else {
      //      analogWrite(Channel[chnum][1], 0);
      digitalWrite(Channel[chnum][1], LOW);
    }
    Channel[chnum][4] = 0;  //update state tracking
  }
  else if (Channel[chnum][0] == 3) {  //RGB Channel
    if (Channel[chnum][7] == 1) {
      digitalWrite(Channel[chnum][1], HIGH);
      digitalWrite(Channel[chnum][2], HIGH);
      digitalWrite(Channel[chnum][3], HIGH);
    }
    else {
      digitalWrite(Channel[chnum][1], LOW);
      digitalWrite(Channel[chnum][2], LOW);
      digitalWrite(Channel[chnum][3], LOW);
    }
    Channel[chnum][4] = 0;  //update state tracking
    Channel[chnum][5] = 0;
    Channel[chnum][6] = 0;

  }
}


//Turn-off all output channels
void BLACKOUT() {
  Serial.println("BLACKOUT MODE");
  for (int i = 1; i <= numChannels; i++) {
    switchOFF(i);
  }
}

//Turn-on all channels
void FULLON() {
  for (int i = 1; i <= numChannels; i++) {
    switchON(i);
  }
}


//Toggle channel
void ChannelTOGGLE(int chnum) { //Toggle Channel from OFF to ON (or ON to OFF)
  if (Channel[chnum][0] < 3) {  //Single Pin Channel, set pin to MAX or MIN
    if (Channel[chnum][4] < 30) { //PIN is mostly off, turn on
      switchON(chnum); //turn channel on
    }
    else {    //otherwise turn off
      switchOFF(chnum); //turn channel off
    }
  }
  else if (Channel[chnum][0] == 3) {  //RGB Channel, sum values and round to tell if its mostly on or off
    if ((Channel[chnum][4] + Channel[chnum][5] + Channel[chnum][6]) < 150) //Mostly off, turn on all pins
      switchON(chnum);  //turn channel on
    else //otherwise turn off channel
      switchOFF(chnum); //turn channel off

  }
}

//Dim all outputs on Channel
void ChannelDIM(int chnum, int value) {
  switch (Channel[chnum][0]) {//check type of channel
    case 1: //Digital IO, turn on and off
      {
        if (value > 50) //if value more than 50%
          switchON(chnum); //turn channel full on
        else
          switchOFF(chnum); //otherwise turn off
      }

    case 2: //Single Channel PWMable. Convert to Analog Write and set pin
      {
        int PWMval = (1024 * (value / 100.0));
        Serial.println("single PWM MODE");
        Serial.println(PWMval);
        if (Channel[chnum][7] == 1) //check inversion flag
          analogWrite(Channel[chnum][1], (100 - PWMval)); //HIGH is off
        else
          analogWrite(Channel[chnum][1], PWMval); //LOW is OFF
        Channel[chnum][4] = value;  //set state tracker
      }

    case 3: //RGB PWMable. Convert to Analog Write values and set all three pins at same value
      {
        int PWMval = (1024 * (value / 100.0));
        Serial.println("RGB PWM as one MODE");
        Serial.println(PWMval);
        if (Channel[chnum][7] == 1) {//check inversion flag
          analogWrite(Channel[chnum][1], (100 - PWMval)); //set PWM on each pin assuming HIGH is off
          analogWrite(Channel[chnum][2], (100 - PWMval));
          analogWrite(Channel[chnum][3], (100 - PWMval));
        }
        else {
          analogWrite(Channel[chnum][1], 0);  //set PWM on each pin assuming HIGH is on
          analogWrite(Channel[chnum][2], 0);
          analogWrite(Channel[chnum][3], 0);
        }
        Channel[chnum][4] = value;  //set state tracker for each pin
        Channel[chnum][5] = value;
        Channel[chnum][6] = value;
        break;
      }
  }
}


//Channel DIM, RGB values must be specified
void RGBDIM(int chnum, int R, int G, int B) {
  if (Channel[chnum][0] < 3) { //Single output channel, refer to correct function
    ChannelDIM(chnum, R);
  }
  else if (Channel[chnum][0] == 3) {  //Valid RGB Channel
    Serial.println("RGB PWM MODE");
    int RPWMval = PWMconvert(R); //Convert to PWM value for analog write
    int GPWMval = PWMconvert(G);
    int BPWMval = PWMconvert(B);

    if (Channel[chnum][7] == 1) {//check inversion flag
      analogWrite(Channel[chnum][1], (100 - RPWMval)); //set PWM on each pin assuming HIGH is off
      analogWrite(Channel[chnum][2], (100 - GPWMval));
      analogWrite(Channel[chnum][3], (100 - BPWMval));
    }
    else {
      analogWrite(Channel[chnum][1], RPWMval);  //set PWM on each pin assuming HIGH is on
      analogWrite(Channel[chnum][2], GPWMval);
      analogWrite(Channel[chnum][3], BPWMval);
    }
    Channel[chnum][4] = R;  //set state tracker for each pin
    Channel[chnum][5] = G;
    Channel[chnum][6] = B;
  }

}

//Dim 1 color of an RGB Channel,
void RGBSDIM(int chnum, int value, char color) {
  Serial.println("VALUE IS" + color);
  Serial.println("VALUE IS" + value);
  if (Channel[chnum][0] < 3) { //Single output channel, refer to correct function
    ChannelDIM(chnum, value);
  }
  else if (Channel[chnum][0] == 3) {// This is a valid RGB Cahnnel
    Serial.println("RGB single ch PWM MODE");
    int PWMval = PWMconvert(value); //Convert to PWM value
    if (Channel[chnum][7] == 1) { //Check inverstion flag
      PWMval = 100 - PWMval;
    }

    switch (color) { //Select correct output based on color
      case 'R': //Red
        {
          analogWrite(Channel[chnum][1], PWMval);
          Channel[chnum][4] = value;
        }
        break;
      case 'G': //Green
        {
          analogWrite(Channel[chnum][2], PWMval);
          Channel[chnum][5] = value;
        }
        break;
      case 'B': //Blue
        {
          analogWrite(Channel[chnum][3], PWMval);
          Channel[chnum][6] = value;
        }
        break;
    }
  }

}

void Burst(int chnum, int t){
  switchON(chnum);
  delay(t);
  switchOFF(chnum);
}





//reset clock used for timeout
void resetCLOCK() {
  mSec = 0;
  seconds = 0;
  minutes = 0;
}

//check to see if clock has surpassed timeout
void checkTimeout() {
  if (timeoutperiod == 0) //if timeout period is 0, timeout function is disabled, do not check
    return;
  if (minutes > timeoutperiod)
    BLACKOUT();
  resetCLOCK();
}



//simple statup init
void initLamp() {

  FULLON();
  delay(1000);
  BLACKOUT();


}


void initChannelIO() { //iterate through Channel Array and setup all pins
  for (int i = 1; i <= numChannels; i++) {
    Serial.println("init ch" + String(i));
    switch (Channel[i][0]) {
      case 11:
        {
            Serial.println("digital input");
            if (Channel[i][7] == 1)
            {  
              pinMode(Channel[i][1], INPUT_PULLUP);   //set pin to input pullup
              Serial.println("input_pullup");
            }
            else
            {
              pinMode(Channel[i][1], INPUT);           // set pin to input
              Serial.println("input");
            }
            Channel[i][4] = 0;
        }
        break;
      case 3:
        {
          Serial.println("PWM Channel");
          pinMode(Channel[i][1], OUTPUT);
          pinMode(Channel[i][2], OUTPUT);
          pinMode(Channel[i][3], OUTPUT);
          if (Channel[i][7] == 1) {
            digitalWrite(Channel[i][1], HIGH);
            digitalWrite(Channel[i][2], HIGH);
            digitalWrite(Channel[i][3], HIGH);
          }
          else {
            digitalWrite(Channel[i][1], LOW);
            digitalWrite(Channel[i][2], LOW);
            digitalWrite(Channel[i][3], LOW);
          }
          Channel[i][4] = 0;
          Channel[i][5] = 0;
          Channel[i][6] = 0;
        }
        break;

      default: //Digital IO and PWM
        {
          Serial.println("single channel");
          pinMode(Channel[i][1], OUTPUT);
          if (Channel[i][7] == 1)
            digitalWrite(Channel[i][1], HIGH);
          else
            digitalWrite(Channel[i][1], LOW);
          Channel[i][4] = 0;
        }
        break;

    }
  }
}


///////
//Functions to perform HTTP communication with web interface and the IoSMaster
///////
//Build json pairs
String responsebuilder(String var, String value) {
  String responsestring = "\"" + var + "\":\"" + value + "\"";
  return responsestring;

}

//Build JSON string for response
String response(String var, String value) {
  //  String ip = WiFi.localIP().toString();
  String responsestring = "{\"espid\":\"" + WiFi.localIP().toString() + "\",";
  responsestring.concat(responsebuilder(var, value));
  responsestring.concat("}");

  //  client.println(responsestring);
  Serial.println(responsestring);
  return responsestring;
}





String reportstatus() {
  Serial.println("in status report");
  //  String ip = WiFi.localIP().toString();
  String responsestring = "{\"espid\":\"" + WiFi.localIP().toString() + "\",";
  responsestring.concat(responsebuilder("nodename", nodename) + ",");
  responsestring.concat(responsebuilder("desc", desc) + ",");
  //  responsestring.concat("\"channels\":\"1\",");
  responsestring.concat("\"channels\":[");

  for (int i = 1; i <= numChannels; i++) {
    Serial.println("CHTYPE:" + Channel[i][0]);

    if (Channel[i][0] == 0) //If there is no type assume its an empty channel
      continue;
    if (i > 1) //to correctly form json there needs to be a comma between elements, start on i=2
      responsestring.concat(",");

    responsestring.concat("{");
    responsestring.concat(responsebuilder("CH", String(i)) + ",");
    responsestring.concat(responsebuilder("CHdesc", Channeldesc[i]) + ",");
    switch (Channel[i][0]) {
      case 1:
        {
          responsestring.concat(responsebuilder("type", "DIGITAL") + ",");
          responsestring.concat(responsebuilder("CHVAL", String(Channel[i][4])));
        }
        break;
      case 2:
        {
          responsestring.concat(responsebuilder("type", "PWM") + ",");
          responsestring.concat(responsebuilder("CHVAL", String(Channel[i][4])));
        }
        break;
      case 3:
        {
          responsestring.concat(responsebuilder("type", "RGB") + ",");
          responsestring.concat(responsebuilder("RVAL", String(Channel[i][4])) + ",");
          responsestring.concat(responsebuilder("GVAL", String(Channel[i][5])) + ",");
          responsestring.concat(responsebuilder("BVAL", String(Channel[i][6])));
        }
        break;
      default:
        responsestring.concat(responsebuilder("type", "UNKNW")+",");
        responsestring.concat(responsebuilder("SCH1", String(Channel[i][4])) + ",");
        responsestring.concat(responsebuilder("SCH2", String(Channel[i][5])) + ",");
        responsestring.concat(responsebuilder("SCH3", String(Channel[i][6])));
        break;

    }
    responsestring.concat("}");
  }




  responsestring.concat("]");
  responsestring.concat("}");
  //  Serial.println("sending 200 header");
  //    client.println("HTTP/1.1 200 OK");
  //    client.println("Content-Type: text/html");
  //    client.println(""); // do not forget this one
  return responsestring;
}






void ticker() {
  //  Serial.println("delay length");
  //  delay(incr);
  MCLKmsec += 10;
  TMRmsec += 10;
  //  Serial.println("TICK" + TMRmsec)

  if (MCLKmsec % 1000 == 0) {
    //    MCLKmsec = 0;
    MCLKsec++;
    //    Serial.println(MCLKsec);
    //    secondFLAG = false;
  }
  if (MCLKsec > 60) {
    MCLKsec = 0;
    MCLKminutes++;
    minuteFLAG = false;
  }
  if (MCLKminutes > 60) {
    MCLKminutes = 0;
    MCLKhours++;
  }


}


void checkInputs(){
  for (int i = 1; i <= numChannels; i++) {
    if(Channel[i][0]>10){
      switch(Channel[i][0]){
        case 11:
        {
          if(digitalRead(Channel[i][1])!=Channel[i][4]){
              Serial.println("*******\nPIN CHANGE\n*******");
              Serial.println(Channeldesc[i]);
              Serial.println(digitalRead(Channel[i][1]));
//              client.println("window.console.log('PIN CHANGE');");
              Serial.println(Channel[i][4]);
              
              Channel[i][4]=digitalRead(Channel[i][1]);
              Serial.println("SET TO:");
              Serial.println(Channel[i][4]);
//              Serial.println(digitalRead(Channel[i][1]));


              if (!client.connect(host, httpPort)) {
                Serial.println("connection failed");
                return;
              }
//            
//              // This will send the request to the server
              client.print(String("GET ") + masterhost + "?mode=updstate&KEY="+Channeldesc[i]+"&VALUE="+Channel[i][4]+" HTTP/1.1\r\n" +
                           "Host: " + host + "\r\n" +
                           "Connection: close\r\n\r\n");
//
//              Serial.println(String("GET ") + url + "?mode=updstate&KEY="+Channeldesc[i]+"&VALUE="+Channel[i][4]+" HTTP/1.1\r\n" +
//                           "Host: " + host + "\r\n" +
//                           "Connection: close\r\n\r\n");             
              unsigned long timeout = millis();
              while (client.available() == 0) {
                if (millis() - timeout > 5000) {
                  Serial.println(">>> Client Timeout !");
                  client.stop();
                  return;
                }
              }
              

              while (client.available()) {
                //    Serial.print("asciR");
                String line = client.readStringUntil('\r');
                Serial.print(line);
//                SIGNcontent += line;
            }





              
              

              
            
          }
        }
        break;
        
      }
    }
  }
}


///////////
////SETUP ARDUINO ON POWERUP
//////////
void setup() {




  Serial.begin(115200); //Start debug serial
  delay(10); //wait, because things break otherwise

  Serial.println("\nSERIAL OUTPUT ACTIVE\n\n");
  delay(10); //wait, because things break otherwise

  //  initChannelArray(6);
//  initChannel(1, 3, 1, 3, 15, 0, "Right");
//  initChannel(3, 1, 10, 0, 0, 1, "Incandescent String");
//  initChannel(2, 3, 14, 12, 13, 0, "Right Flood");
  initChannel(0, 0, 0, 0, 0, 0, "CTRL");
  initChannel(1, 1, 14, 0, 0, 0, "LED BEACON LIGHT");
  initChannel(2, 0, 12, 0, 0, 0, "CHIME B");
  initChannel(3, 0, 15, 12, 14, 0, "CHIME A");
  initChannel(4, 1, 13, 12, 14, 0, "open");
  
  



  initChannelIO();

  BLACKOUT();


  startWIFI();//Connect to WIFI network and start server

  //Set IO for BLUE DEBUG LIGHT
  pinMode(BLULED, OUTPUT);
  digitalWrite(BLULED, HIGH);

//  analogWriteFreq(2700); //Set PWM clock, some ESPs seem fuckered about this
//3000 is pretty good
//  analogWriteFreq(6000);
  analogWriteFreq(10000);

  //  wdt_disable();

  MCLKmsec = 0;
  MCLKsec = 0;
  MCLKminutes = 0;
  MCLKhours = 0;

  TMRmsec = 0;
  TMRsec = 0;
  TMRminutes = 0;
  TMRhours = 0;


  Serial.println("ESP INIT COMPLETED");

  //  initLamp();//Test lights on startup

}
































void loop() {
  //  Serial.println("loop");
  //  wdt_disable();
  //if wifi connection has been lost, try to reconnect

  if (WiFi.status() != WL_CONNECTED) {
    delay(1);
    startWIFI();
    return;
  }
  delay(10);
  ticker();
  checkInputs();

  //  Serial.println("current time:");
  //  Serial.println("mSec:");
  //    Serial.print(mSec);
  //  Serial.println("seconds:");
  //  Serial.print(seconds);
  //  Serial.println("minutes:");
  //  Serial.print(minutes);





  ////////////////////////////AFTER THIS ONLY RUNS IF CLIENT CONNECTED


  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return; //no one connected exit loop
  }

  //someone must have connected
  Serial.println("new victim");




  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request); //serial debug output
  client.flush(); //trash data

  if (request.indexOf("init") != -1) {
    client.println(reportstatus());
    return;
  }


  //GET CH
  chnum = 1;
  if (request.indexOf("CH=") != 1) {
    String t = request.substring((request.indexOf("CH=") + 3), (request.indexOf("CH=") + 4));
    Serial.println("COMMAND FOR CH:" + t);
    chnum = t.toInt();
  }



  //GET ACTION
  action = "none";
  value = "none";
  if (request.indexOf("ACTION=") != -1) {
    action = request.substring((request.indexOf("ACTION=") + 7));
    Serial.println("ACTION IS " + action);
    if (action.indexOf(".") != -1) {
      value = action.substring((action.indexOf(".") + 1), (action.indexOf("HTTP/") - 1));
      Serial.println("VALUE IS " + value);
    }

  }


  //START Processing
  if (action.indexOf("SWITCHON") != -1) {
    switchON(chnum);
  }
  else if (action.indexOf("SWITCHOFF") != -1) {
    switchOFF(chnum);
  }
  else if (action.indexOf("LIGHTDIM") != -1) {
    ChannelDIM(chnum, value.toInt());
  }
  else if (action.indexOf("BLACKOUT") != -1) {
    BLACKOUT();
  }

  else if (action.indexOf("BURST") != -1) {
    Serial.println("BURST");
    Serial.println(value);
    Serial.println(value.toInt());
    Burst(chnum,value.toInt());
  }

  else if (action.indexOf("TOGGLE") != -1) {
    ChannelTOGGLE(chnum);
  }

  else if (request.indexOf("RGBSDIM") != -1) {
    Serial.println("SINGLECHANNELDIM/" + value);
    char DIMcolor = value[0];
    String t = value.substring(1, 3);
    Serial.println("coloris:");
    Serial.println(DIMcolor);
    Serial.println("valueis:" + t);
    int DIMval = t.toInt();
    RGBSDIM(chnum, DIMval, DIMcolor);
  }
  else if (request.indexOf("RGBDIM") != -1) {
    Serial.println("RGBDIM/" + value);

    String Rstr = value.substring(0, 2);
    String Gstr = value.substring(2, 4);
    String Bstr = value.substring(4, 6);
    //    Serial.println(Rstr.toInt()
    int R = Rstr.toInt();
    int G = Gstr.toInt();
    int B = Bstr.toInt();
    Serial.println("RGBcoloris:");
    Serial.println(R);
    Serial.println(G);
    Serial.println(B);
    RGBDIM(chnum, R, G, B);
  }

  else if (action.indexOf("DIM") != -1) {
    ChannelDIM(chnum, value.toInt());
  }

  else {

    // Set ledPin according to the request
    //digitalWrite(ledPin, value);

    // Return the response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); // do not forget this one
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<link rel='stylesheet' href='http://"+masterhost+"/espserve/style.css'>");
    client.println("<script type='text/javascript' src='http://"+masterhost+"/espserve/scripts/jquery-1.11.1.js'></script>");
    client.println("<body>");
    client.println("<div id='fallback'>");
    client.println("<a href='/LIGHTS=ON'>CLICK TO TURN LIGHTS ON</a>");
    client.println("<BR><BR><BR><BR><BR><BR>");
    client.println("<a href='/LIGHTS=OFF'>CLICK TO TURN LIGHTS OFF</a>");
    client.println("</div>");
    client.println("<div id='panel'>");
    client.println("</div>");
    client.println("</body>");
    client.println("<script type='text/javascript' src='http://"+masterhost+"/espserve/scripts/IOSlocal.js'></script>");

    //
    //  client.println("<br><br>");
    //  client.println("<a href=\"/LED=ON\">Click here to turn the lights on</a>");
    //  client.println("<BR><BR><BR><BR><BR><BR><BR>");
    //  client.println("<a href=\"/LED=OFF\">Click to turn lights off</a>");

    client.println("</html>");

    delay(1);
    Serial.println("Client disconnected");
    Serial.println("");
  }

//    delay(100);
}





