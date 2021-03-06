#include "espDMX.h"
#include <ESP8266WiFi.h>
#include <FS.h>
#include <passwords.h>

#define PWMRANGE 255

DMXESPSerial dmx;

// Default values, may be overwritten by SPIFF config.dat
// Node Name, will be set as hostname
String nodename = "DevMEMTEST";

// Node Description
String nodedesc = "Development Memory Test";

// WIFI Credentials. Set default as values in passwords.h
String WIFIssid = ssid;
String WIFIpassword = password;

// init WiFi propoerties
WiFiClient client;
WiFiServer server(80);

String IOSResources = "ioshit.net";

//# of devices
const byte numChannels = 128;
byte numActiveChannels = 0;

// // setup timeout clock
// int MCLKmsec, MCLKsec, MCLKminutes, MCLKhours;
// int TMRmsec, TMRsec, TMRminutes, TMRhours;
//
// int mSec = 0;
// int seconds = 0;
// int minutes = 0;
// bool minuteFLAG = false;
// int minutehold;

// fade holds
String fading="";
// int fadetimeremaining = 0;
// String fadeCHs = "";
// float fadeRstep = 0;
// float fadeGstep = 0;
// float fadeBstep = 0;
// float fadingR = 0;
// // float fadingG = 0;
// // float fadingB = 0;
unsigned long fadestart = millis();

//entire buffer has to be maintened in case there is a looping command
String commandbuffer="";

//Position in command buffer, 0=empty or not active (completion without loop)
int commandbufferpos=0;

// configure ESP
int BLULED = 2; // ESP BLUE LED FOR DEBUGGING

// setup for command processing
int chnum;
String action, value;

// Channel Array, to be populated from configuration file
int Channel[numChannels][8];
String Channeldesc[numChannels];

bool DMXpresent = false;
String DMXCTRLChs = "";

int cmdwaitms=0;

// initChannel - Store config data into 2D array
//
// Channel Array Entry = {TYPE,IOa,IOb,IOc,STATEa,STATEb,STATEc,INVERTFLAG}
// TYPE: 1=Switch,2=PWM, 3=RGB PWM, 2 = DMX,
// RGB uses a,b,c; otherwise only use a
// Invertflag used if HIGH is off
// STATE should be init 0, used to keep track of last values
// First array entry Channel[0] should be all zeros. Channels start at
// Channel[1] to align with CHnum being passed to function
void initChannel(int CHID, int type, int pin1, int pin2, int pin3, int flag,
                 String desc) {
	Serial.println("initing " + String(CHID) + ": " + desc);
	Channel[CHID][0] = (char)type;
	Channel[CHID][1] = (char)pin1;
	Channel[CHID][2] = (char)pin2;
	Channel[CHID][3] = (char)pin3;
	Channel[CHID][4] = (char)0;
	Channel[CHID][5] = (char)0;
	Channel[CHID][6] = (char)0;
	Channel[CHID][7] = (char)flag;
	// Channeldesc[CHID] = desc;
}

/// IO OPERATIONS
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

void switchON(int chnum) {
	//check to see if it is a digital or analog output
	if (Channel[chnum][0] < 3) {
    //is digital...check inversion
		if (Channel[chnum][7] == 1) {
			digitalWrite(Channel[chnum][1], LOW);
		} else {
			digitalWrite(Channel[chnum][1], HIGH);
		}
		Channel[chnum][4] = 100;
	} else if (Channel[chnum][0] == 3) {
    //is analog check inversion and set PWM duty cycle to 0
		if (Channel[chnum][7] == 1) {
			analogWrite(Channel[chnum][1], 0);
			analogWrite(Channel[chnum][2], 0);
			analogWrite(Channel[chnum][3], 0);
      //fixes bug on nodemcu
			digitalWrite(Channel[chnum][1], LOW);
			digitalWrite(Channel[chnum][2], LOW);
			digitalWrite(Channel[chnum][3], LOW);
		} else {
			analogWrite(Channel[chnum][1], 255);
			analogWrite(Channel[chnum][2], 255);
			analogWrite(Channel[chnum][3], 255);
			digitalWrite(Channel[chnum][1], HIGH);
			digitalWrite(Channel[chnum][2], HIGH);
			digitalWrite(Channel[chnum][3], HIGH);
		}
    //set status variable for tracking
		Channel[chnum][4] = 100;
		Channel[chnum][5] = 100;
		Channel[chnum][6] = 100;

	} else if (Channel[chnum][0] == 21) {
    //is a DMX

		dmx.write(Channel[chnum][1], 255);
		dmx.write(Channel[chnum][2], 255);
		dmx.write(Channel[chnum][3], 255);

		Channel[chnum][4] = 100;
		Channel[chnum][5] = 100;
		Channel[chnum][6] = 100;
	}
}

//switch Channel off
void switchOFF(int chnum) {


  //check device type and process off command accordingly
  if (Channel[chnum][0] < 3) { //single output channel
		if (Channel[chnum][7] == 1) { //inverted output
			//      analogWrite(Channel[chnum][1], 255);
			digitalWrite(Channel[chnum][1], HIGH);
		}

		else { //normal output
			//      analogWrite(Channel[chnum][1], 0);
			digitalWrite(Channel[chnum][1], LOW);
		}
		Channel[chnum][4] = 0;
	} else if (Channel[chnum][0] == 3) { //RGB device
		if (Channel[chnum][7] == 1) {
			analogWrite(Channel[chnum][1], 255);
			analogWrite(Channel[chnum][2], 255);
			analogWrite(Channel[chnum][3], 255);
			digitalWrite(Channel[chnum][1], HIGH);
			digitalWrite(Channel[chnum][2], HIGH);
			digitalWrite(Channel[chnum][3], HIGH);
		} else {
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

	} else if (Channel[chnum][0] == 21) { //dmx device

		dmx.write(Channel[chnum][1], 0);
		dmx.write(Channel[chnum][2], 0);
		dmx.write(Channel[chnum][3], 0);

		Channel[chnum][4] = 0;
		Channel[chnum][5] = 0;
		Channel[chnum][6] = 0;
	}
}

//blackout command turns off all channels
void BLACKOUT() {
	Serial.println("BLACKOUT MODE");
	for (int i = 1; i <= numChannels; i++) { //iterate through activated channels
		switchOFF(i);
	}
}

//turn all channels on to full
void FULLON() {
	for (int i = 1; i <= numChannels; i++) { //iterate through activated channels
		switchON(i);
	}
}

void ChannelTOGGLE(int chnum) { // Toggle Channel from OFF to ON (or ON to OFF)
	if (Channel[chnum][0] < 3) { // Single Pin Channel, set pin to MAX or MIN
		if (Channel[chnum][4] < 30) { // PIN is mostly off, turn on
			switchON(chnum); // turn channel on
		} else {          // otherwise turn off
			switchOFF(chnum); // turn channel off
		}
	} else if (Channel[chnum][0] == 3) { // RGB Channel, sum values and round to
		                             // tell if its mostly on or off
		if ((Channel[chnum][4] + Channel[chnum][5] + Channel[chnum][6]) <
		    150) // Mostly off, turn on all pins
			switchON(chnum); // turn channel on
		else    // otherwise turn off channel
			switchOFF(chnum); // turn channel off
	}
}



//retrieve a preset (presetID) from SPIFFS memory
void readPreset(int presetID){
  File f = SPIFFS.open("/presets.dat", "r");
  //clear command buffer
  commandbuffer="";
  while (f.available()){
    String line = f.readStringUntil('\n') + '\n';
    //found reqeusted presets
    if (line.startsWith("~~~~~~"+String(presetID))){
      Serial.println("PRESET FOUND");
      while (f.available()){
        line = f.readStringUntil('\n') + '\n';
        //read until end of preset
        if (!line.startsWith("######")){
          //load preset into command buffer
          commandbuffer += f.readStringUntil('\n') + '\n';
        }
      }
    }
  }
  //set pointer to first line of buffer
  commandbufferpos=1;
}


void setCommandWait(int wait){
  Serial.println("setting wait to "+String(wait)+" ms");
  cmdwaitms=wait;
}


void processCMDbuffer(){
  Serial.println("Command Buffer");
  String line = commandbuffer;
  while(line!=""){
    for(int i=0;i<commandbufferpos;i++){
      line=line.substring((line.indexOf("\n")+2));
      Serial.println("new line"+String(i));
      Serial.println(line);
    }
    //check if there are following lines, if so trim
    if(line.indexOf("\n")>-1){
      line=line.substring(0,line.indexOf("\n"));
    }

    if(line.startsWith("WAIT")){
      setCommandWait(line.substring(4).toInt());
    }

  }
}

void readCommands(String commands){
  //read passed commandstring into buffer
  commandbuffer=commands;

  //set buffer point to 1
  commandbufferpos=1;
}







void FADEmaintainer() {
	String fademaintainer=fading;
	fading = "";
	// Serial.println("FADEmaintainer");



	while (fademaintainer.length()>0) {
		// Serial.print(fademaintainer);
		String fadeaction = fademaintainer.substring(0,fademaintainer.indexOf("\n"));
		fademaintainer = fademaintainer.substring((fademaintainer.indexOf("\n")+1));

		//#CH,time,finalR,finalG,finalB,Rstep,Gstep,fadeBstep
		int fadeCH = fadeaction.substring(1,fadeaction.indexOf(",")).toInt();
		fadeaction = fadeaction.substring((fadeaction.indexOf(",")+1));
		// Serial.println("Maintaining CH");
		// Serial.println(fadeCH);

		// Serial.println(fadeaction);
		// Serial.println(fadeaction.substring(0,fadeaction.indexOf(",")).toInt());
		int fadetime = fadeaction.substring(0,fadeaction.indexOf(",")).toInt();
		fadeaction =  fadeaction.substring(fadeaction.indexOf(",")+1);

		unsigned long fadetimestarted = fadeaction.substring(0,fadeaction.indexOf(",")).toInt();
		fadeaction =  fadeaction.substring(fadeaction.indexOf(",")+1);

		float fadeadvance = (millis()-fadetimestarted);


		// Serial.println(fadetimeremaining);
		// Serial.println("next parse");
		// Serial.println(fadeaction);

		int finalR = fadeaction.substring(0,fadeaction.indexOf(",")).toInt();
		fadeaction = fadeaction.substring((fadeaction.indexOf(",")+1));
		// Serial.println("finalR"+String(finalR));

		int finalG = fadeaction.substring(0,fadeaction.indexOf(",")).toInt();
		fadeaction = fadeaction.substring((fadeaction.indexOf(",")+1));

    int finalB = fadeaction.substring(0,fadeaction.indexOf(",")).toInt();
		fadeaction = fadeaction.substring((fadeaction.indexOf(",")+1));
		// Serial.println("finalG"+String(finalG));

		float Rstep = fadeaction.substring(0,fadeaction.indexOf(",")).toFloat();
		fadeaction = fadeaction.substring((fadeaction.indexOf(",")+1));

		float Gstep = fadeaction.substring(0,fadeaction.indexOf(",")).toFloat();
		fadeaction = fadeaction.substring((fadeaction.indexOf(",")+1));

		float Bstep = fadeaction.substring(0,fadeaction.indexOf(",")).toFloat();
		fadeaction = fadeaction.substring((fadeaction.indexOf(",")+1));

		int Rstart = fadeaction.substring(0,fadeaction.indexOf(",")).toInt();
		fadeaction = fadeaction.substring((fadeaction.indexOf(",")+1));

		int Gstart = fadeaction.substring(0,fadeaction.indexOf(",")).toInt();
		fadeaction = fadeaction.substring((fadeaction.indexOf(",")+1));

		int Bstart = fadeaction.substring(0,fadeaction.indexOf(",")).toInt();
		fadeaction = fadeaction.substring((fadeaction.indexOf(",")+1));

		if (fadeadvance < fadetime) {

			// Serial.println("fadeadvance");
			// Serial.println(fadeadvance);
      //
			// Serial.println(Channel[fadeCH][4]);
			// Serial.println(fadeadvance * Rstep);
			// Serial.println(fadeadvance * Gstep);
			// Serial.println(fadeadvance * Bstep);
			// Serial.println(Channel[fadeCH][4] + (fadeadvance * Rstep));

			Channel[fadeCH][4] =Rstart + (fadeadvance * Rstep);
			Channel[fadeCH][5] =Gstart + (fadeadvance * Gstep);
			Channel[fadeCH][6] =Bstart + (fadeadvance * Bstep);
			RGBDIM(fadeCH, Channel[fadeCH][4], Channel[fadeCH][5], Channel[fadeCH][6]);

			fading += "#"+String(fadeCH)+","+String(fadetime)+","+String(fadetimestarted)+","+String(finalR)+","+String(finalG)+","+String(finalB)+",";
      fading += String(Rstep,4)+","+String(Gstep,4)+","+String(Bstep,4)+","+String(Rstart)+","+String(Gstart)+","+String(Bstart)+"\n";
			// Serial.println(fading);

		}
		else{
			Serial.println("FADE OVER");
			RGBDIM(fadeCH, finalR, finalG, finalB);
		}


	}


	// if (fadetimeremaining > 0) {

	//
	//
	//   // Serial.println("fade advancing");
	//   //Serial.println(millis());
	//   //Serial.println(fadestart);
	//   //Serial.println(fadeadvance);
	//   //Serial.println(fadetimeremaining);
	//
	//      fadingR = fadingR + (fadeadvance * fadeRstep);
	//      fadingG = fadingG + (fadeadvance * fadeGstep);
	//      fadingB = fadingB + (fadeadvance * fadeBstep);
	//
	//   Channel[chnum][4] =(int)fadingR;
	//   Channel[chnum][5] =(int)fadingG;
	//   Channel[chnum][6] =(int)fadingB;
	//
	//      String maintainfadeCHs = fadeCHs;
	//      while (maintainfadeCHs.length() > 0) {
	//              //Serial.println(MaintainDMXCTRLChs);
	//              int CC = maintainfadeCHs.substring(0, maintainfadeCHs.indexOf(",")).toInt();
	//              //Serial.println(DMXCTRLChs.indexOf("," + 1));
	//              maintainfadeCHs = maintainfadeCHs.substring((maintainfadeCHs.indexOf(",") + 1));
	//              // Serial.println("Dimming " + String(CC));
	//              // Serial.println((int)fadingR);
	//              // Serial.println((int)fadingG);
	//              // Serial.println((int)fadingB);
	//              RGBDIM(CC, (int)fadingR, (int)fadingG, (int)fadingB);
	//      }
	// }
	// else
	// {
	//      fadeRstep = 0;
	//      fadeGstep = 0;
	//      fadeBstep = 0;
	//      fadeCHs = "";
	// }
}

void ChannelFADEset(int chnum, int finalRval, int finalGval, int finalBval,
                    int fadetime) {

	Serial.println("fade buffers");
	Serial.println(fading);
	//check to see if the channel is already fading and remove it
	Serial.println(removeCHfromFade(chnum));
	Serial.println(fading);

	//Calculate fade parameters
	Serial.println("Fade time is:"+String(fadetime));

	float fadeRstep = finalRval - Channel[chnum][4];
	float fadeGstep = finalGval - Channel[chnum][5];
	float fadeBstep = finalBval - Channel[chnum][6];
	Serial.println("fade difference is: " + String(fadeRstep,4));
	Serial.println("fade difference is: " + String(fadeGstep,4));
	Serial.println("fade difference is: " + String(fadeBstep,4));

	fadeRstep = fadeRstep / (float)fadetime;
	fadeGstep = fadeGstep / (float)fadetime;
	fadeBstep = fadeBstep / (float)fadetime;
	Serial.println("fade step is: " + String(fadeRstep,4));
	Serial.println("fade step is: " + String(fadeGstep,4));
	Serial.println("fade step is: " + String(fadeBstep,4));


	//fadeCHs += String(chnum) + ",";


	fading += "#"+String(chnum)+","+String(fadetime)+","+String(millis())+","+String(finalRval)+","+String(finalGval)+","+String(finalBval)+",";
  fading += String(fadeRstep,4)+","+String(fadeGstep,4)+","+String(fadeBstep,4)+","+String(Channel[chnum][4])+","+String(Channel[chnum][5])+","+String(Channel[chnum][6])+"\n";
	Serial.println(fading);

//#CH,fadetime,timestarted, finalR,finalG,finalB,Rstep,Gstep,fadeBstep,Rstart,Gstart,Bstart





}


int removeCHfromFade(int CH){
	Serial.println("Cheking if channel is already fading");
	Serial.println(fading);
	Serial.println("#"+String(CH)+",");
	int CHcmdpos = fading.indexOf(("#"+String(CH)+","));
	Serial.println(CHcmdpos);
	if(fading.indexOf(("#"+String(CH)+","))>=0) {
		Serial.println("Channel in Fade");
		Serial.println(fading);
		String fadingA=fading.substring(0,fading.indexOf(("#"+String(CH)+",")));
		Serial.println("FADINGA"+fadingA);
		String fadingB=fading.substring(fading.indexOf(("#"+String(CH)+",")));
		Serial.println("FADINGB"+fadingB);
		fadingB = fadingB.substring(fadingB.indexOf("\n"));
		Serial.println("FADINGB2"+fadingB);
		fading = fadingA+fadingB;
		Serial.println(fading);
		return 1;
	}


	Serial.println("Channel not present");
	return 0;
}

// Dim all outputs on Channel
void ChannelDIM(int chnum, int value) {
	switch (Channel[chnum][0]) { // check type of channel
	case 1:                // Digital IO, turn on and off
	{
		if (value > 50) // if value more than 50%
			switchON(chnum); // turn channel full on
		else
			switchOFF(chnum); // otherwise turn off
	}

	case 2: // Single Channel PWMable. Convert to Analog Write and set pin
	{
		int PWMval = (1024 * (value / 100.0));
		// Serial.println("single PWM MODE");
		// Serial.println(PWMval);
		if (Channel[chnum][7] == 1)           // check inversion flag
			analogWrite(Channel[chnum][1], (255 - PWMval)); // HIGH is off
		else
			analogWrite(Channel[chnum][1], PWMval); // LOW is OFF
		Channel[chnum][4] = value;    // set state tracker
	}

	case 3: // RGB PWMable. Convert to Analog Write values and set all three pins
		// at same value
	{
		int PWMval = (1024 * (value / 100.0));
		// Serial.println("RGB PWM as one MODE");
		// Serial.println(PWMval);
		if (Channel[chnum][7] == 1) { // check inversion flag
			analogWrite(Channel[chnum][1],
			            (255 - PWMval)); // set PWM on each pin assuming HIGH is off
			analogWrite(Channel[chnum][2], (255 - PWMval));
			analogWrite(Channel[chnum][3], (255 - PWMval));
		} else {
			analogWrite(Channel[chnum][1],
			            0); // set PWM on each pin assuming HIGH is on
			analogWrite(Channel[chnum][2], 0);
			analogWrite(Channel[chnum][3], 0);
		}
		Channel[chnum][4] = value; // set state tracker for each pin
		Channel[chnum][5] = value;
		Channel[chnum][6] = value;
		break;
	}
	case 21: {
		dmx.write(Channel[chnum][1], 255);
		dmx.write(Channel[chnum][2], 255);
		dmx.write(Channel[chnum][3], 255);
		dmx.update();
		Channel[chnum][4] = 100;
		Channel[chnum][5] = 100;
		Channel[chnum][6] = 100;
	}
	}
}

void RGBDIM(int chnum, int R, int G, int B) {
	//Serial.println("RGB PWM MODE"+String(R)+"/"+String(G)+"/"+String(B));
	int RPWMval = PWMconvert(R);
	int GPWMval = PWMconvert(G);
	int BPWMval = PWMconvert(B);

	if (Channel[chnum][0] == 3) {
		// Serial.println("RGB PWM MODE");
		if (Channel[chnum][7] == 1) { // check inversion flag
			analogWrite(Channel[chnum][1],
			            (255 - RPWMval)); // set PWM on each pin assuming HIGH is off
			analogWrite(Channel[chnum][2], (255 - GPWMval));
			analogWrite(Channel[chnum][3], (255 - BPWMval));
		} else {
			analogWrite(Channel[chnum][1],
			            RPWMval); // set PWM on each pin assuming HIGH is on
			analogWrite(Channel[chnum][2], GPWMval);
			analogWrite(Channel[chnum][3], BPWMval);
		}
		Channel[chnum][4] = R; // set state tracker for each pin
		Channel[chnum][5] = G;
		Channel[chnum][6] = B;
	} else if (Channel[chnum][0] == 21) {
		dmx.write(Channel[chnum][1], RPWMval);
		dmx.write(Channel[chnum][2], GPWMval);
		dmx.write(Channel[chnum][3], BPWMval);
		dmx.update();
		Channel[chnum][4] = R;
		Channel[chnum][5] = G;
		Channel[chnum][6] = B;
	} else {
		ChannelDIM(chnum, R);
	}
}

void RGBSDIM(int chnum, int value, char color) {
	// Serial.println("VALUE IS" + color);
	// Serial.println("VALUE IS" + value);
	if (Channel[chnum][0] == 3) {
		// Serial.println("RGB single ch PWM MODE");
		int PWMval = PWMconvert(value);
		if (Channel[chnum][7] == 1) {
			PWMval = 255 - PWMval;
		}
		// Serial.println("PMWval-");
		// Serial.println(PWMval);
		switch (color) {
		case 'R': {
			analogWrite(Channel[chnum][1], PWMval);
			Channel[chnum][4] = value;
		} break;
		case 'G': {
			analogWrite(Channel[chnum][2], PWMval);
			Channel[chnum][5] = value;
		} break;
		case 'B': {
			analogWrite(Channel[chnum][3], PWMval);
			Channel[chnum][6] = value;
		} break;
		}
	} else if (Channel[chnum][0] == 21) {
		int PWMval = PWMconvert(value);
		switch (color) {
		case 'R': {
			dmx.write(Channel[chnum][1], PWMval);
			Channel[chnum][4] = value;
		} break;
		case 'G': {
			dmx.write(Channel[chnum][2], PWMval);
			Channel[chnum][5] = value;
		} break;
		case 'B': {
			dmx.write(Channel[chnum][3], PWMval);
			Channel[chnum][6] = value;
		} break;
		}
	} else {
		ChannelDIM(chnum, value);
	}
}

///////
// Functions to perform HTTP communication with web interface and the IoSMaster
///////
// Build json pairs
String responsebuilder(String var, String value) {
	String responsestring = "\"" + var + "\":\"" + value + "\"";
	return responsestring;
}

// Build JSON string for response
String response(String var, String value) {
	//  String ip = WiFi.localIP().toString();
	String responsestring = "{\"espid\":\"" + WiFi.localIP().toString() + "\",";
	responsestring.concat(responsebuilder(var, value));
	responsestring.concat("}");

	//  client.println(responsestring);
	// Serial.println(responsestring);
	return responsestring;
}

String reportstatus() {
	// Serial.println("in status report");
	//  String ip = WiFi.localIP().toString();
	String responsestring = "{\"espid\":\"" + WiFi.localIP().toString() + "\",";
	responsestring.concat(responsebuilder("desc", nodedesc) + ",");
	responsestring.concat(responsebuilder("hostname", WiFi.hostname()) + ",");
	//  responsestring.concat("\"channels\":\"1\",");
	responsestring.concat("\"channels\":[");

	for (int i = 1; i <= numActiveChannels; i++) {
		// Serial.println("CHTYPE:" + Channel[i][0]);

		if (Channel[i][0] == 0) // If there is no type assume its an empty channel
			continue;
		if (i > 1) // to correctly form json there needs to be a comma between
			   // elements, start on i=2
			responsestring.concat(",");

		responsestring.concat("{");
		responsestring.concat(responsebuilder("CH", String(i)) + ",");
		responsestring.concat(responsebuilder("CHdesc", Channeldesc[i]) + ",");
		switch (Channel[i][0]) {
		case 1: {
			responsestring.concat(responsebuilder("type", "DIGITAL") + ",");
			responsestring.concat(responsebuilder("CHVAL", String(Channel[i][4])));
		} break;
		case 2: {
			responsestring.concat(responsebuilder("type", "PWM") + ",");
			responsestring.concat(responsebuilder("CHVAL", String(Channel[i][4])));
		} break;
		case 3: {
			responsestring.concat(responsebuilder("type", "RGB") + ",");
			responsestring.concat(responsebuilder("RVAL", String(Channel[i][4])) +
			                      ",");
			responsestring.concat(responsebuilder("GVAL", String(Channel[i][5])) +
			                      ",");
			responsestring.concat(responsebuilder("BVAL", String(Channel[i][6])));
		} break;
		case 21: {
			responsestring.concat(responsebuilder("type", "RGB") + ",");
			responsestring.concat(responsebuilder("RVAL", String(Channel[i][4])) +
			                      ",");
			responsestring.concat(responsebuilder("GVAL", String(Channel[i][5])) +
			                      ",");
			responsestring.concat(responsebuilder("BVAL", String(Channel[i][6])));
		} break;
		default:
			responsestring.concat(responsebuilder("type", "UNKNW"));
			break;
		}
		responsestring.concat("}");
	}

	responsestring.concat("]");
	responsestring.concat("}");
	return responsestring;
}

void writeConfigfile(String newconfig) {
	File f = SPIFFS.open("/config.dat", "w+");
	if (!f) {
		Serial.println("file open failed");
	} else {
		f.print(newconfig);
		Serial.println("file updated with:");
		Serial.println(newconfig);
	}
}

String sendConfigfile() {
	File f = SPIFFS.open("/config.dat", "r");
	if (!f) {
		Serial.println("file open failed");
		return "FILE OPEN FAILED";
	} else {
		String configfile = "";
		Serial.println("====== Reading from SPIFFS file =======");
		while (f.available())
			configfile += f.readStringUntil('\n') + '\n';
		return configfile;
	}
}


String sendPresets() {
	File f = SPIFFS.open("/presets.dat", "r");
	if (!f) {
		Serial.println("file open failed");
		return "FILE OPEN FAILED";
	} else {
		String configfile = "";
		Serial.println("====== Reading from SPIFFS file =======");
		while (f.available())
			configfile += f.readStringUntil('\n') + '\n';
		return configfile;
	}
}

void writePresetsfile(String newpresets) {
	File f = SPIFFS.open("/presets.dat", "w+");
	if (!f) {
		Serial.println("file open failed");
	} else {
		f.print(newpresets);
		Serial.println("file updated with:");
		Serial.println(newpresets);
	}
}


void initDMX(int CH, int Profile, int Start) {
	DMXpresent = true;

	Serial.println("initing DMX on CH: " + String(CH) + " with profile: " +
	               String(Profile) + " starting at: " + String(Start));

	// get DMX Profile

	switch (Profile) {
	case 1: // Generic Amazon Lights
	{
		Serial.println("init small amazon style light#" + String(CH));
		Channel[CH][1] = Start + 1;
		Channel[CH][2] = Start + 3;
		Channel[CH][3] = Start + 2;
		Serial.println("adding control channel " + String(Start));
		DMXCTRLChs += (String(Start) + ",");
	} break;
	case 2: // LED PAR LIGHT
	{
		Serial.println("init small amazon style light#" + String(CH));
		Channel[CH][1] = Start + 1;
		Channel[CH][2] = Start + 2;
		Channel[CH][3] = Start + 3;
		Serial.println("adding control channel " + String(Start));
		DMXCTRLChs += (String(Start) + ",");
	} break;
	default: { Serial.println("INVALID PROFILE"); }
	}
}

void initChannelIO() { // iterate through Channel Array and setup all pins
	Serial.println("in init chIO");
	Serial.println(numActiveChannels + "/" + numChannels);
	delay(100);
	for (int i = 1; i <= numChannels; i++) {
		// Serial.println("init ch" + String(i));
		numActiveChannels++;
		switch (Channel[i][0]) {
		// DMX Profiles
		case 21: // DMX
		{
			Serial.println("DMX Channel");
			initDMX(i, Channel[i][1], Channel[i][2]);
			Serial.println(DMXCTRLChs);
		} break;

		case 3: {
			// Serial.println("PWM Channel");
			pinMode(Channel[i][1], OUTPUT);
			pinMode(Channel[i][2], OUTPUT);
			pinMode(Channel[i][3], OUTPUT);
			if (Channel[i][5] == 1) {
				digitalWrite(Channel[i][1], HIGH);
				digitalWrite(Channel[i][2], HIGH);
				digitalWrite(Channel[i][3], HIGH);
			} else {
				digitalWrite(Channel[i][1], LOW);
				digitalWrite(Channel[i][2], LOW);
				digitalWrite(Channel[i][3], LOW);
			}
			Channel[i][4] = 0;
			Channel[i][5] = 0;
			Channel[i][6] = 0;
		} break;

		case 2: // Digital PWM
		{
			// Serial.println("single channel PWM");
			pinMode(Channel[i][1], OUTPUT);
			if (Channel[i][5] == 1)
				digitalWrite(Channel[i][1], HIGH);
			else
				digitalWrite(Channel[i][1], LOW);
			Channel[i][4] = 0;
		} break;

		case 1: // Digital IO
		{
			// Serial.println("single channel");
			pinMode(Channel[i][1], OUTPUT);
			if (Channel[i][5] == 1)
				digitalWrite(Channel[i][1], HIGH);
			else
				digitalWrite(Channel[i][1], LOW);
			Channel[i][4] = 0;
		} break;
		default: {
			// Serial.println("Empty Channel");
			numActiveChannels--;
		}
		}
	}
}

void processconfigchannels(File f) {
	Serial.println("~~~~Configuring Channels from file~~~~");
	while (f.available()) {
		String line = f.readStringUntil('\n');
		Serial.println(line);
		if (line.startsWith("##CHANNELS-END##")) {
			return;
		} else {
			Serial.println("channel line is" + line);
			String chdesc = line.substring(line.indexOf("#"));
			Serial.println(chdesc);
			line = line.substring(0, line.indexOf("#"));

			line.replace(" ", "");

			int channel = line.substring(0, line.indexOf(",")).toInt();
			Serial.println(channel);
			line = line.substring(line.indexOf(",") + 1);

			int chtype = line.substring(0, line.indexOf(",")).toInt();
			// Serial.println(chtype);

			line = line.substring(line.indexOf(",") + 1);

			int chcfgA = line.substring(0, line.indexOf(",")).toInt();
			// Serial.println(chcfgA);
			line = line.substring(line.indexOf(",") + 1);

			int chcfgB = line.substring(0, line.indexOf(",")).toInt();
			// Serial.println(chcfgB);
			line = line.substring(line.indexOf(",") + 1);

			int chcfgC = line.substring(0, line.indexOf(",")).toInt();
			// Serial.println(chcfgC);
			line = line.substring(line.indexOf(",") + 1);

			int invert = line.substring(0, line.indexOf(",")).toInt();
			// Serial.println(invert);
			line = line.substring(line.indexOf(",") + 1);

			initChannel(channel, chtype, chcfgA, chcfgB, chcfgC, invert, chdesc);
		}
	}
}

void processconfigfile(File f) {
	// read file line by line
	while (f.available()) {
		String line = f.readStringUntil('\n');
		Serial.println(line);

		if (line.startsWith("WIFI-SSID")) {
			Serial.println(line.substring(line.indexOf("=") + 1));
			WIFIssid = line.substring(line.indexOf("=") + 1);
		} else if (line.startsWith("WIFI-password")) {
			Serial.println(line.substring(line.indexOf("=") + 1));
			WIFIpassword = line.substring(line.indexOf("=") + 1);
		} else if (line.startsWith("IOSRESOURCES")) {
			Serial.println(line.substring(line.indexOf("=") + 1));
			IOSResources = line.substring(line.indexOf("=") + 1);
		} else if (line.startsWith("NODENAME")) {
			Serial.println(line.substring(line.indexOf("=") + 1));
			nodename = line.substring(line.indexOf("=") + 1);
		} else if (line.startsWith("DESCRIPTION")) {
			Serial.println(line.substring(line.indexOf("=") + 1));
			nodedesc = line.substring(line.indexOf("=") + 1);
		} else if (line.startsWith("!!CHANNELS-START!!")) {
			processconfigchannels(f);
		}
	}
}

void servefilefromSPIFFS(String filename,
                         String passthrough = "NO DATA PASSED") {
	File f = SPIFFS.open("/" + filename, "r");
	if (!f) {
		Serial.println("opening " + filename + " failed");
		return;
	} else {
		Serial.println("====== Reading " + filename + " from SPIFFS =======");
	}


  Serial.println("file size is :"+String(f.size()));
	// START SENDING PAGE
	// DEFAULT HTTP HEADER
	client.println("HTTP/1.1 200 OK");
  //client.println("Content-Length: "+String(f.size()));
	client.println("Content-Type: text/html");
  client.println("Connection: close");
	client.println(""); // do not forget this one

	// read out HTML file
	String line = "";
	while (f.available()) {
		line = f.readStringUntil('\r');
		// Serial.print(line);
		if (line.indexOf("~!@#$PASSTHROUGH~!@#$") > 0) {
			Serial.println("PASSING DATA THROUGH");
			Serial.println(passthrough);
			client.print(passthrough);
		} else
			client.print(line);
      yield();
      client.flush();
	}

	delay(10);
  client.stop();
	Serial.println("Client disconnected / Page Sent");
}

void serve404() {
  Serial.println("404 Error");
	client.println("HTTP/1.1 404 Not Found");
  client.stop();
}

void serve200() {
  Serial.println("200 Repsonse");
	client.println("HTTP/1.1 200 OK");
  client.stop();
}


// connect to wifi
void startWIFI() {
	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.persistent(false);
	// connect to network using cons credentials

	WiFi.begin(ssid, password);
	// WiFi.persistent(false);

	while (WiFi.status() != WL_CONNECTED) { // wait for connection
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

void webserver(String request) {
	if (request.indexOf(".html") > 0) {
		String page =
			request.substring(request.indexOf("/") + 1, request.indexOf(".html"));
		Serial.println("looking for page" + page);
		servefilefromSPIFFS(page + ".html");
	} else if (request.indexOf(".js") > 0) {
		String script =
			request.substring(request.indexOf("/") + 1, request.indexOf(".js"));
		servefilefromSPIFFS(script + ".js");
	} else if (request.indexOf("/ HTTP/1.1") > 0) {
		servefilefromSPIFFS("index.html");
	} else {
		serve404();
	}
}

void processCHRequest(String action = "", int chnum = 0, String value = "") {
	Serial.println("processing node request");
	Serial.println(chnum);
	Serial.println(action);
	Serial.println(value);

	// START Processing
	if (action.indexOf("SWITCHON") != -1) {
		switchON(chnum);
	} else if (action.indexOf("SWITCHOFF") != -1) {
		switchOFF(chnum);
	} else if (action.indexOf("LIGHTDIM") != -1) {
		ChannelDIM(chnum, value.toInt());
	} else if (action.indexOf("BLACKOUT") != -1) {
		BLACKOUT();
	}

	else if (action.indexOf("TOGGLE") != -1) {
		ChannelTOGGLE(chnum);
	}

	else if (action.indexOf("RGBSDIM") != -1) {
		// Serial.println("SINGLECHANNELDIM/" + value);
		char DIMcolor = value[0];
		String t = value.substring(1, 3);
		// Serial.println("coloris:");
		// Serial.println(DIMcolor);
		// Serial.println("valueis:" + t);
		int DIMval = t.toInt();
		RGBSDIM(chnum, DIMval, DIMcolor);
	} else if (action.indexOf("RGBDIM") != -1) {
		// Serial.println("RGBDIM/" + value);

		String Rstr = value.substring(0, 2);
		String Gstr = value.substring(2, 4);
		String Bstr = value.substring(4, 6);
		//    Serial.println(Rstr.toInt()
		int R = Rstr.toInt();
		int G = Gstr.toInt();
		int B = Bstr.toInt();
		// Serial.println("RGBcoloris:");
		// Serial.println(R);
		// Serial.println(G);
		// Serial.println(B);
		RGBDIM(chnum, R, G, B);
	}

	else if (action.indexOf("DIM") != -1) {
		ChannelDIM(chnum, value.toInt());
	}

	else if (action.indexOf("RGBFADE") != -1) {
		String finalRval = value.substring(0, 2);
		String finalGval = value.substring(2, 4);
		String finalBval = value.substring(4, 6);
		String timestep = value.substring(6);

		ChannelFADEset(chnum, finalRval.toInt(), finalGval.toInt(),
		               finalBval.toInt(), timestep.toInt());
	}
	else if (action.indexOf("FADE") != -1) {
		String finalval = value.substring(0, 2);
		String timestep = value.substring(6);
		ChannelFADEset(chnum, finalval.toInt(), 0, 0, timestep.toInt());
	}
}

void DMXmaintenance() {
	// Serial.println("Control Channel Maintenance");
	// Serial.println(DMXCTRLChs);
	String MaintainDMXCTRLChs = DMXCTRLChs;
	while (MaintainDMXCTRLChs.length() > 0) {
		// Serial.println(MaintainDMXCTRLChs);
		int CC = MaintainDMXCTRLChs.substring(0, MaintainDMXCTRLChs.indexOf(","))
		         .toInt();
		// Serial.println(DMXCTRLChs.indexOf("," + 1));
		MaintainDMXCTRLChs =
			MaintainDMXCTRLChs.substring((MaintainDMXCTRLChs.indexOf(",") + 1));
		// Serial.println("A Control CH is "+String(CC));
		// Serial.println(MaintainDMXCTRLChs);
		dmx.write(CC, 255);
	}
	// Update DMX Universe
	dmx.update();
}

void loop() {
	// if wifi connection has been lost, try to reconnect

	if (WiFi.status() != WL_CONNECTED) {
		delay(1);
		startWIFI();
		return;
	}
	yield();

	//delayMicroseconds(500); // mS tick -- ASSUMES 200uS execution time
	// delay(100); //to debug with serial
	// process DMX control channels


	if (DMXpresent) //helps to slow down to avoid overwhelming DMX devices
		DMXmaintenance();

	if (fading != "")
		FADEmaintainer();

	// Check if a client has connected
	client = server.available();
	client.setNoDelay(1);
	yield();
	if (!client) {
		yield();
		return; // no one connected exit loop
	}

	////////////////////////////AFTER THIS ONLY RUNS IF CLIENT CONNECTED
	// Read the first line of the request
	Serial.println("connection");
	client.setTimeout(100);
	String request = client.readStringUntil('\r');


	Serial.println(request); // serial debug output

  bool moreRequests = true;

  if(request == "")
	 moreRequests = false; // just kidding

	yield();

	while (moreRequests) {

		// Extract any parameters for a node request
		// Get Channel
    Serial.println("REQ:");
    Serial.println(request);
		int chnum = 1;
		if (request.indexOf("CH=") != 1) {
			String t = request.substring((request.indexOf("CH=") + 3),
			                             (request.indexOf("CH=") + 4));
			Serial.println("COMMAND FOR CH:" + t);
			chnum = t.toInt();
		}

		if (request.indexOf("updconfig") != -1) {
			client.println(request);
		//	client.setTimeout(1000);
			String newconfig = client.readString();
			if (newconfig.indexOf("CONFIGSTRING")) {
				newconfig =
					newconfig.substring(newconfig.indexOf("CONFIGSTRING##") + 14);
				Serial.println("New Config Recieved");
				Serial.println(newconfig);
				writeConfigfile(newconfig);
				ESP.restart();

			} else {
				client.println("NO CONFIG UPDATES PRESENT");
				Serial.println("NO CONFIG UPDATES PRESENT");
			}

			break;
		}
    else if (request.indexOf("updpresets") != -1) {
			client.println(request);
		//	client.setTimeout(1000);
			String newpresets = client.readString();
			if (newpresets.indexOf("PRESETUPDATE##")) {
				newpresets = newpresets.substring(newpresets.indexOf("PRESETUPDATE##") + 14);
				Serial.println("New Presets Recieved");
				Serial.println(newpresets);
				writePresetsfile(newpresets);

			} else {
				client.println("NO PRESET UPDATES PRESENT");
				Serial.println("NO PRESET UPDATES PRESENT");
			}

			break;
		}

		else if (request.indexOf("cfginit") != -1) {
			client.println(sendConfigfile());
			break;
		}

    else if (request.indexOf("presetinit") != -1) {
      client.println(sendPresets());
      break;
    }

		else if (request.indexOf("init") != -1 || request.indexOf("status") != -1) {
			client.println(reportstatus());
			break;
		}

		// GET ACTION
		String action = "none";
		String value = "none";
		if (request.indexOf("ACTION=") != -1) {
			action = request.substring((request.indexOf("ACTION=") + 7));
			if ((action.indexOf("+") != -1)) { // there is a following command
				action = action.substring(
					0, action.indexOf("+")); // trim off the following command
			}
			// Serial.println("ACTION IS " + action);
			if (action.indexOf(".") != -1) {
				value = action.substring((action.indexOf(".") + 1),
				                         (action.indexOf("HTTP/") - 1));
				// Serial.println("VALUE IS " + value);
			}
		}

    if (action == "CMDS"){
      readCommands(value);
    }
    // if there is an node action process it
		else if (action != "none") {
			processCHRequest(action, chnum, value);
			if (request.indexOf("+") != -1) { // there is a command following
				request = request.substring(request.indexOf("+") + 1);
				Serial.println("NEXT COMMAND");
				Serial.println(request);
				moreRequests = true;
			} else{
        //nothing else to do
        Serial.println("Actions done");
        serve200(); //serve 200
        moreRequests = false; //and exit
        client.stop();
      }

		}
		// otherwise it may be a webrequest
		else {
      Serial.println("Service webpage");
      Serial.println(request);
			webserver(request);
      moreRequests = false;
      delay(1);
      client.stop();
		}
	}
}

// Setup/Init on Startup
///////////
////SETUP ARDUINO ON POWERUP
//////////
void setup() {


	Serial.begin(500000); // Start debug serial
	delay(10);      // wait, because things break otherwise

	Serial.println("SERIAL OUTPUT ACTIVE");

	Serial.println("Start reading file");
	SPIFFS.begin();

	// open file for reading
	File f = SPIFFS.open("/config.dat", "r");
	if (!f) {
		Serial.println("file open failed");
		Serial.println("using default config");
		initChannel(1, 1, 5, 0, 0, 1, "CH1");
	} else {
		Serial.println("====== Reading from SPIFFS file =======");
		processconfigfile(f);
	}

	Serial.println("initChannelIO");
	initChannelIO();

	startWIFI(); // Connect to WIFI network and start server

	//  analogWriteFreq(2700); //Set PWM clock, some ESPs seem fuckered about this
	analogWriteFreq(3000);

	// MCLKmsec = 0;
	// MCLKsec = 0;
	// MCLKminutes = 0;
	// MCLKhours = 0;
  //
	// TMRmsec = 0;
	// TMRsec = 0;
	// TMRminutes = 0;
	// TMRhours = 0;

	Serial.println("ESP LOAD COMPLETE");

	if (DMXpresent) {
		Serial.println("DMX Present");
		// Serial.end();
		dmx.init();
	} else {
		Serial.println("NO DMX");
	}
}
