#include <ESP8266WiFi.h>
#include <string.h>

//WIFI ssid and password in include file
#include <passwords.h>





char* CMDCTRLaddr;
char* CMDCTRLhost;

//define channel properties
//CH# = Channel type (RGB, DIGital, DIMmer)
//CH#desc = Alphanumberic Description
//CH#IO array contain IO pin mapping
//CH#STATE array for internal tracking

//Define IoS Node
//description
const char* desc = "busbox";
//# of devices
const int numChannels = 8; //This is needed because C sucks. Should be 1 less than array size


//Channel Array Entry = {TYPE,IOa,IOb,IOc,STATEa,STATEb,STATEc,INVERTFLAG}
//TYPE: 1=Switch,2=PWM, 3=RGB PWM
//RGB uses a,b,c; otherwise only use a
//Invertflag used if HIGH is off
//STATE should be init 0, used to keep track of last values
//First array entry Channel[0] should be all zeros. Channels start at Channel[1] to align with CHnum being passed to function
static int Channel[(numChannels + 1)][8];
static String Channeldesc[(numChannels + 1)];
//static char ChannelState[6][3] = {
//  {0,0,0},
//  {0, 0, 0},
//  {0, 0, 0},
//  {0, 0, 0},
//  {0, 0, 0}
//  };

int D89 = 0;
int S89 = 0;
int S101 = 0;
int L80 = 0;
int CFE = 0;
String activeAlert="";

int slow = 2000;
int medium = 1000;
int fast = 500;
int sfast = 250;

void initChannel(int CHID, int type, int pin1, int pin2, int pin3, int flag, String desc) {
  Serial.println("initing " + String(CHID) + ": " + desc);
  Channel[CHID][0] = (char)type;
  Channel[CHID][1] = (char)pin1;
  Channel[CHID][2] = (char)pin2;
  Channel[CHID][3] = (char)pin3;
  Channel[CHID][4] = (char)0;
  Channel[CHID][5] = (char)0;
  Channel[CHID][6] = (char)0;
  Channel[CHID][7] = (char)flag;
  Channeldesc[CHID] = desc;

}


//const String CH1desc = "Tall light";
//const String CH2desc = "R Arrow";
//const String CH3desc = "L Arrow";
//const String CH4desc = "Empty";
//const String CH5desc = "L Red";
//const String CH6desc = "M Amber";
//const String CH7desc = "R Red";
//const String CH8desc = "Empty";

//configure ESP
int BLULED = 2; //ESP BLUE LED FOR DEBUGGING




//setup timeout clock
int mSec = 0;
int seconds = 0;
int minutes = 0;
bool minuteFLAG = false;
bool pausebusbotFLAG = false;
int busbotTOUT = 0;

int timeoutperiod = 20; //timeout in minutes to turn off light, no timeout if 0

//timer
int MCLKmsec, MCLKsec, MCLKminutes, MCLKhours;
int TMRmsec, TMRsec, TMRminutes, TMRhours;

int minutehold;

String SIGNcontent;

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
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

}














///IO OPERATIONS

int PWMconvert(int val) {
  int PWM = 255;
  if (val < 5)
    PWM = 0;
  else if (val > 98)
    PWM = 255;
  else
    PWM = (val / 100.0) * 255.0;
  return PWM;
}



void RGBFade(int chnum, int R, int G, int B) {
  //Get starting state

  if (chnum == 1) {

  }
}


void switchON(int chnum) {
  //  Serial.println("ON");
  //  Serial.println(chnum);
  if (Channel[chnum][0] < 3) {
    if (Channel[chnum][7] == 1) {
      //      analogWrite(Channel[chnum][1], 0);
      digitalWrite(Channel[chnum][1], LOW);
    }
    else {
      //      analogWrite(Channel[chnum][1], 255);
      digitalWrite(Channel[chnum][1], HIGH);
    }
    Channel[chnum][4] = 100;
  }
  else if (Channel[chnum][0] == 3) {
    if (Channel[chnum][7] == 1) {
      analogWrite(Channel[chnum][1], 0);
      analogWrite(Channel[chnum][2], 0);
      analogWrite(Channel[chnum][3], 0);
      digitalWrite(Channel[chnum][1], LOW);
      digitalWrite(Channel[chnum][2], LOW);
      digitalWrite(Channel[chnum][3], LOW);
    }
    else {
      analogWrite(Channel[chnum][1], 255);
      analogWrite(Channel[chnum][2], 255);
      analogWrite(Channel[chnum][3], 255);
      digitalWrite(Channel[chnum][1], HIGH);
      digitalWrite(Channel[chnum][2], HIGH);
      digitalWrite(Channel[chnum][3], HIGH);
    }
    Channel[chnum][4] = 100;
    Channel[chnum][5] = 100;
    Channel[chnum][6] = 100;

  }
}


void switchOFF(int chnum) {
  //  Serial.println("OFF");
  //  Serial.println(chnum);
  //  Serial.println("SWITCHOFF" + chnum);
  //  Serial.println(Channel[chnum][0]);
  //  Serial.println(Channel[chnum][5]);

  if (Channel[chnum][0] < 3) {
    if (Channel[chnum][7] == 1) {
      //      analogWrite(Channel[chnum][1], 255);
      digitalWrite(Channel[chnum][1], HIGH);
    }

    else {
      //      analogWrite(Channel[chnum][1], 0);
      digitalWrite(Channel[chnum][1], LOW);
    }
    Channel[chnum][4] = 0;
  }
  else if (Channel[chnum][0] == 3) {
    if (Channel[chnum][7] == 1) {
      analogWrite(Channel[chnum][1], 255);
      analogWrite(Channel[chnum][2], 255);
      analogWrite(Channel[chnum][3], 255);
      digitalWrite(Channel[chnum][1], HIGH);
      digitalWrite(Channel[chnum][2], HIGH);
      digitalWrite(Channel[chnum][3], HIGH);
    }
    else {
      analogWrite(Channel[chnum][1], 0);
      analogWrite(Channel[chnum][2], 0);
      analogWrite(Channel[chnum][3], 0);
      digitalWrite(Channel[chnum][1], LOW);
      digitalWrite(Channel[chnum][2], LOW);
      digitalWrite(Channel[chnum][3], LOW);
    }
    Channel[chnum][4] = 0;
    Channel[chnum][5] = 0;
    Channel[chnum][6] = 0;

  }
}

void BLACKOUT() {
  Serial.println("BLACKOUT MODE");
  for (int i = 1; i <= numChannels; i++) {
    switchOFF(i);
  }
}

void FULLON() {
  for (int i = 1; i <= numChannels; i++) {
    switchON(i);
  }
}

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

void RGBDIM(int chnum, int R, int G, int B) {
  if (Channel[chnum][0] == 3) {
    Serial.println("RGB PWM MODE");
    int RPWMval = PWMconvert(R);
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
  else {
    ChannelDIM(chnum, R);
  }
}

void RGBSDIM(int chnum, int value, char color) {
  Serial.println("VALUE IS" + color);
  Serial.println("VALUE IS" + value);
  if (Channel[chnum][0] == 3) {
    Serial.println("RGB single ch PWM MODE");
    int PWMval = PWMconvert(value);
    if (Channel[chnum][7] == 1) {
      PWMval = 100 - PWMval;
    }

    switch (color) {
      case 'R':
        {
          analogWrite(Channel[chnum][1], PWMval);
          Channel[chnum][4] = value;
        }
        break;
      case 'G':
        {
          analogWrite(Channel[chnum][2], PWMval);
          Channel[chnum][5] = value;
        }
        break;
      case 'B':
        {
          analogWrite(Channel[chnum][3], PWMval);
          Channel[chnum][6] = value;
        }
        break;
    }
  }
  else {
    ChannelDIM(chnum, value);
  }

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
      case 3:
        {
          Serial.println("PWM Channel");
          pinMode(Channel[i][1], OUTPUT);
          pinMode(Channel[i][2], OUTPUT);
          pinMode(Channel[i][3], OUTPUT);
          if (Channel[i][5] == 1) {
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
          if (Channel[i][5] == 1)
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
        responsestring.concat(responsebuilder("type", "UNKNW"));
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


///////////
////SETUP ARDUINO ON POWERUP
//////////
void setup() {




  Serial.begin(115200); //Start debug serial
  delay(10); //wait, because things break otherwise

  Serial.println("SERIAL OUTPUT ACTIVE");

  //  initChannelArray(6);
  initChannel(1, 1, 16, 0, 0, 1, "Arrow R");
  initChannel(2, 1, 5, 0, 0, 0, "Arrow L");
  initChannel(3, 1, 4, 0, 0, 1, "Red R");
  initChannel(4, 1, 0, 0, 0, 0, "Amber");
  initChannel(5, 1, 2, 0, 0, 1, "Red L");
  initChannel(6, 1, 14, 0, 0, 0, "Tall Light");
  initChannel(7, 1, 12, 0, 0, 1, "Empty");
  initChannel(8, 1, 13, 0, 0, 0, "Amber L");

  initChannelIO();

  BLACKOUT();


  startWIFI();//Connect to WIFI network and start server

  //Set IO for BLUE DEBUG LIGHT
  pinMode(BLULED, OUTPUT);
  digitalWrite(BLULED, HIGH);

  analogWriteFreq(2700); //Set PWM clock, some ESPs seem fuckered about this

  //  wdt_disable();

  MCLKmsec = 0;
  MCLKsec = 0;
  MCLKminutes = 0;
  MCLKhours = 0;

  TMRmsec = 0;
  TMRsec = 0;
  TMRminutes = 0;
  TMRhours = 0;

  CMDCTRLhost = "192.168.0.31";
  CMDCTRLaddr = "/espserve/CMDCTRL/buses.cmd";

  updateCMD();
  Serial.println("ESP INIT COMPLETED");

  //  initLamp();//Test lights on startup

}




































//////SPECIFIC TO BUS BOX
void updateCMDfile() {
  char* host = CMDCTRLhost;
  char* url = CMDCTRLaddr;
  Serial.println("localtion is" + String(CMDCTRLaddr));




  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  SIGNcontent = "";

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    //    Serial.print("asciR");
    String line = client.readStringUntil('\r');
    Serial.print(line);
    SIGNcontent += line;
  }

  Serial.println();
  Serial.println("closing connection");

}



void updatebuses() {
  Serial.println("BUSES");
  int index;
 
  index = SIGNcontent.indexOf("CFE:");
  CFE = SIGNcontent.substring((index + 4), (index + 7)).toInt();
  Serial.println(CFE);

  index = SIGNcontent.indexOf("AFLAG:");
//  Serial.println(index);
  activeAlert = SIGNcontent.substring((index + 6), (index + 7));
  Serial.println("Alert Type:");
  Serial.println(activeAlert);
  if (activeAlert >= "A")
    BLACKOUT();

  Serial.println("S89");
  index = SIGNcontent.indexOf("S89:");
  S89 = SIGNcontent.substring((index + 4), (index + 7)).toInt();
  Serial.println(S89);

  Serial.println("S101");
  index = SIGNcontent.indexOf("S101:");
  S101 = SIGNcontent.substring((index + 5), (index + 8)).toInt();
  Serial.println(S101);

  Serial.println("D89");
  index = SIGNcontent.indexOf("D89:");
  D89 = SIGNcontent.substring((index + 4), (index + 7)).toInt();
  Serial.println(D89);

  Serial.println("L80");
  index = SIGNcontent.indexOf("L80:");
  L80 = SIGNcontent.substring((index + 4), (index + 7)).toInt();
  Serial.println(L80);


  //  CFE = atoi(SIGNcontent.substring(strstr(SIGNcontent, "CFE:"),2));
  //  serial.println(CFE);
  //  D89 = atoi(SIGNcontent.substring(strstr(SIGNcontent, "D89:"),2));
  //  serial.println(D89);
  //  S89 = atoi(SIGNcontent.substring(strstr(SIGNcontent, "S89:"),2));
  //  serial.println(S89);
  //  S101 = atoi(SIGNcontent.substring(strstr(SIGNcontent, "S101:"),2));
  //  serial.println(S101);

}





void updateCMD() {
  updateCMDfile();
  updatebuses();
  pausebusbotFLAG = false;
}

void lightbot() {
  if (activeAlert=="A"){
      if (MCLKmsec % sfast == 0 && MCLKmsec > 0) {
        if (Channel[6][4]==100){
          switchOFF(1);
          switchOFF(2);
          switchOFF(6);
          switchON(3);
          switchON(4);
          switchON(5);
        }
        else{
          switchON(1);
          switchON(2);
          switchON(6);
          switchOFF(3);
          switchOFF(4);
          switchOFF(5);
        }
      } 
    return;
  }
  if (activeAlert=="B"){
      if (MCLKmsec % fast == 0 && MCLKmsec > 0) {
        if (Channel[6][4]==100){
          switchOFF(1);
          switchOFF(2);
          switchOFF(6);
          switchON(3);
          switchON(4);
          switchON(5);
        }
        else{
          switchON(1);
          switchON(2);
          switchON(6);
          switchOFF(3);
          switchOFF(4);
          switchOFF(5);
        }
      } 
    return;
  }
    if (activeAlert=="C"){
      if (MCLKmsec % medium == 0 && MCLKmsec > 0) {
        if (Channel[6][4]==100){
          switchOFF(1);
          switchOFF(2);
          switchOFF(6);
          switchON(3);
          switchON(4);
          switchON(5);
        }
        else{
          switchON(1);
          switchON(2);
          switchON(6);
          switchOFF(3);
          switchOFF(4);
          switchOFF(5);
        }
      } 
    return;
  }
    if (activeAlert=="D"){
      if (MCLKmsec % slow == 0 && MCLKmsec > 0) {
        if (Channel[6][4]==100){
          switchOFF(1);
          switchOFF(2);
          switchOFF(6);
          switchON(3);
          switchON(4);
          switchON(5);
        }
        else{
          switchON(1);
          switchON(2);
          switchON(6);
          switchOFF(3);
          switchOFF(4);
          switchOFF(5);
        }
      } 
    return;
  }  

  //check coffee
  if (CFE > 0) {
    if (CFE < 15) {
      if (MCLKmsec % fast == 0 && MCLKmsec > 0) {
        ChannelTOGGLE(6);
      }
    }
    else if (CFE < 45) {
      if (MCLKmsec % medium == 0 && MCLKmsec > 0) {
        ChannelTOGGLE(6);
      }
    }
    else if (CFE < 90) {
      if (MCLKmsec % slow == 0 && MCLKmsec > 0) {
        ChannelTOGGLE(6);
      }


    }
  }
  else {
    //    Serial.println("COFFEE OFF");
    switchOFF(6);
  }

  //  turn off arrows
  //  switchOFF(1);
  //  switchOFF(2);

  //turn off route lights
  switchOFF(3);
  switchOFF(4);
  switchOFF(5);


  if (MCLKsec < 15) { //Davis 89
    switchON(4);
    switchOFF(2);
    if (D89 > 0) {
      if (D89 < 6) {
        if (MCLKmsec % sfast == 0 && MCLKmsec > 0)
          ChannelTOGGLE(1);
      }
      else if (D89 < 11) {
        if (MCLKmsec % fast == 0 && MCLKmsec > 0)
          ChannelTOGGLE(1);
      }
      else if (D89 < 16) {
        if (MCLKmsec % medium == 0 && MCLKmsec > 0)
          ChannelTOGGLE(1);
      }
      else if (D89 < 31) {
        if (MCLKmsec % slow == 0 && MCLKmsec > 0)
          ChannelTOGGLE(1);
      }
      else {
        switchOFF(2);
        switchOFF(1);
      }
    }
    else {
      switchOFF(2);
      switchOFF(1);
    }



  }
  else if (MCLKsec < 30) { //Sulivan 89
    switchON(4);
    switchOFF(1);
    if (S89 > 0) {
      if (S89 < 6) {
        if (MCLKmsec % sfast == 0 && MCLKmsec > 0)
          ChannelTOGGLE(2);
      }
      else if (S89 < 11) {
        if (MCLKmsec % fast == 0 && MCLKmsec > 0)
          ChannelTOGGLE(2);
      }
      else if (S89 < 16) {
        if (MCLKmsec % medium == 0 && MCLKmsec > 0)
          ChannelTOGGLE(2);
      }
      else if (S89 < 31) {
        if (MCLKmsec % slow == 0 && MCLKmsec > 0)
          ChannelTOGGLE(2);
      }
      else {
        switchOFF(1);
        switchOFF(2);
      }
    }
    else {
      switchOFF(1);
      switchOFF(2);
    }

  }
  else if (MCLKsec < 45) { //Sullivan 101
    switchON(5);
    switchOFF(1);
    if (S101 > 0) {
      if (S101 < 6) {
        if (MCLKmsec % sfast == 0 && MCLKmsec > 0)
          ChannelTOGGLE(2);
      }
      else if (S101 < 11) {
        if (MCLKmsec % fast == 0 && MCLKmsec > 0)
          ChannelTOGGLE(2);
      }
      else if (S101 < 16) {
        if (MCLKmsec % medium == 0 && MCLKmsec > 0)
          ChannelTOGGLE(2);
      }
      else if (S101 < 31) {
        if (MCLKmsec % slow == 0 && MCLKmsec > 0)
          ChannelTOGGLE(2);
      }
      else {
        switchOFF(1);
        switchOFF(2);
      }
    }
    else {
      switchOFF(1);
      switchOFF(2);
    }
  }
  else {            //Lechmere 80
    switchON(3);
    switchOFF(1);
    if (L80 > 0) {
      if (L80 < 3) {
        if (MCLKmsec % sfast == 0 && MCLKmsec > 0)
          ChannelTOGGLE(2);
      }
      else if (L80 < 6) {
        if (MCLKmsec % fast == 0 && MCLKmsec > 0)
          ChannelTOGGLE(2);
      }
      else if (L80 < 11) {
        if (MCLKmsec % medium == 0 && MCLKmsec > 0)
          ChannelTOGGLE(2);
      }
      else if (L80 < 25) {
        if (MCLKmsec % slow == 0 && MCLKmsec > 0)
          ChannelTOGGLE(2);
      }
      else {
        switchOFF(1);
        switchOFF(2);
      }
    }
    else {
      switchOFF(1);
      switchOFF(2);
    }
  }

}








void pausebusbot(int TMOUT) {
  TMRsec = 0;
  //  BLACKOUT();
  pausebusbotFLAG = true;
  busbotTOUT = TMOUT;

}

void checkbusbotpause() {
  if (TMRsec > busbotTOUT) {
    pausebusbotFLAG = false;
    BLACKOUT();
  }
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

  //  Serial.println("current time:");
  //  Serial.println("mSec:");
  //    Serial.print(mSec);
  //  Serial.println("seconds:");
  //  Serial.print(seconds);
  //  Serial.println("minutes:");
  //  Serial.print(minutes);

  if (!pausebusbotFLAG) {
    lightbot();
    if (minutehold != MCLKminutes && !minuteFLAG) {
      Serial.println("minute");
      updateCMD();
      minuteFLAG = true;
    }
  }








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
  else if (action.indexOf("CMDLOAD") != -1) {
    updateCMD();
    pausebusbotFLAG = false;
    return;
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
  else if (action.indexOf("PAUSEBUSBOT") != -1) {
    //pause botbot
    pausebusbot(60);
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
    client.println("<link rel='stylesheet' href='http://192.168.0.31/espserve/style.css'>");
    client.println("<script type='text/javascript' src='http://192.168.0.31/espserve/scripts/jquery-1.11.1.js'></script>");
    client.println("<body>");
    client.println("<div id='fallback'>");
    client.println("<a href='/LIGHTS=ON'>CLICK TO TURN LIGHTS ON</a>");
    client.println("<BR><BR><BR><BR><BR><BR>");
    client.println("<a href='/LIGHTS=OFF'>CLICK TO TURN LIGHTS OFF</a>");
    client.println("</div>");
    client.println("<div id='panel'>");
    client.println("</div>");
    client.println("</body>");
    client.println("<script type='text/javascript' src='http://192.168.0.31/espserve/scripts/IOSlocal.js'></script>");

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

  //  delay(500);
}





