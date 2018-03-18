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

// setup timeout clock
int MCLKmsec, MCLKsec, MCLKminutes, MCLKhours;
int TMRmsec, TMRsec, TMRminutes, TMRhours;

int mSec = 0;
int seconds = 0;
int minutes = 0;
bool minuteFLAG = false;
int minutehold;

// fade holds
int fadetimeremaining = 0;
String fadeCHs = "";
float fadeRstep = 0;
float fadeGstep = 0;
float fadeBstep = 0;
float fadingR = 0;
float fadingG = 0;
float fadingB = 0;
unsigned long fadestart = millis();

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
	//  Serial.println("ON");
	//  Serial.println(chnum);
	if (Channel[chnum][0] < 3) {
		if (Channel[chnum][7] == 1) {
			//      analogWrite(Channel[chnum][1], 0);
			digitalWrite(Channel[chnum][1], LOW);
		} else {
			//      analogWrite(Channel[chnum][1], 255);
			digitalWrite(Channel[chnum][1], HIGH);
		}
		Channel[chnum][4] = 100;
	} else if (Channel[chnum][0] == 3) {
		if (Channel[chnum][7] == 1) {
			analogWrite(Channel[chnum][1], 0);
			analogWrite(Channel[chnum][2], 0);
			analogWrite(Channel[chnum][3], 0);
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
		Channel[chnum][4] = 100;
		Channel[chnum][5] = 100;
		Channel[chnum][6] = 100;

	} else if (Channel[chnum][0] == 21) {

		dmx.write(Channel[chnum][1], 255);
		dmx.write(Channel[chnum][2], 255);
		dmx.write(Channel[chnum][3], 255);

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
	} else if (Channel[chnum][0] == 3) {
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

	} else if (Channel[chnum][0] == 21) {

		dmx.write(Channel[chnum][1], 0);
		dmx.write(Channel[chnum][2], 0);
		dmx.write(Channel[chnum][3], 0);

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

void FADEmaintainer() {
	if (fadetimeremaining > 0) {
    float fadeadvance = (millis()-fadestart);
    fadestart=millis();
    fadetimeremaining=fadetimeremaining-fadeadvance;


    Serial.println("fade advancing");
    //Serial.println(millis());
    //Serial.println(fadestart);
    //Serial.println(fadeadvance);
    //Serial.println(fadetimeremaining);

		fadingR = fadingR + (fadeadvance * fadeRstep);
		fadingG = fadingG + (fadeadvance * fadeGstep);
		fadingB = fadingB + (fadeadvance * fadeBstep);

    Channel[chnum][4] =(int)fadingR;
    Channel[chnum][5] =(int)fadingG;
    Channel[chnum][6] =(int)fadingB;

		String maintainfadeCHs = fadeCHs;
		while (maintainfadeCHs.length() > 0) {
			// Serial.println(MaintainDMXCTRLChs);
			int CC = maintainfadeCHs.substring(0, maintainfadeCHs.indexOf(",")).toInt();
			// Serial.println(DMXCTRLChs.indexOf("," + 1));
			maintainfadeCHs = maintainfadeCHs.substring((maintainfadeCHs.indexOf(",") + 1));
//			Serial.println("Dimming " + String(CC));
			Serial.println((int)fadingR);
			//Serial.println((int)fadingG);
			//Serial.println((int)fadingB);
			RGBDIM(CC, (int)fadingR, (int)fadingG, (int)fadingB);
		}
	}
  else
  {
		fadeRstep = 0;
		fadeGstep = 0;
		fadeBstep = 0;
		fadeCHs = "";
	}
}

void ChannelFADEset(int chnum, int finalRval, int finalGval, int finalBval,
                    int fadetime) {
  Serial.println("Fade time is:"+String(fadetime));

	fadeRstep = finalRval - Channel[chnum][4];
	fadeGstep = finalGval - Channel[chnum][5];
	fadeBstep = finalBval - Channel[chnum][6];
	Serial.println("fade difference is: " + String(fadeRstep,4));

	fadeRstep = fadeRstep / (float)fadetime;
	fadeGstep = fadeGstep / (float)fadetime;
	fadeBstep = fadeBstep / (float)fadetime;
	Serial.println("fade step is: " + String(fadeRstep,4));

	fadetimeremaining = fadetime;
  fadestart=millis();
	fadeCHs += String(chnum) + ",";

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
	// Serial.println("RGB PWM MODE");
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

void serveHTMLfromSPIFFS(String filename,
                         String passthrough = "NO DATA PASSED") {
	File f = SPIFFS.open("/" + filename, "r");
	if (!f) {
		Serial.println("opening " + filename + " failed");
		return;
	} else {
		Serial.println("====== Reading " + filename + " from SPIFFS =======");
	}
	// START SENDING PAGE
	// DEFAULT HTTP HEADER
	client.println("HTTP/1.1 200 OK");
	client.println("Content-Type: text/html");
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
	}

	delay(5);
	Serial.println("Client disconnected / Page Sent");
}

void serve404() {
	client.println("HTTP/1.1 404 Not Found");
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
		serveHTMLfromSPIFFS(page + ".html");
	} else if (request.indexOf(".js") > 0) {
		String script =
			request.substring(request.indexOf("/") + 1, request.indexOf(".js"));
		serveHTMLfromSPIFFS(script + ".js");
	} else if (request.indexOf("/ HTTP/1.1") > 0) {
		serveHTMLfromSPIFFS("index.html");
	} else {
		serve404();
	}
}

void processNodeRequest(String action = "", int chnum = 0, String value = "") {
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


	if (DMXpresent and (millis()%100==0)) //helps to slow down to avoid overwhelming DMX devices
    DMXmaintenance();

  if (fadeCHs != "")
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
	String request = client.readStringUntil('\r');

	Serial.println(request); // serial debug output

	bool moreRequests = true;

	yield();

	while (moreRequests) {

		// Extract any parameters for a node request
		// Get Channel
		int chnum = 1;
		if (request.indexOf("CH=") != 1) {
			String t = request.substring((request.indexOf("CH=") + 3),
			                             (request.indexOf("CH=") + 4));
			// Serial.println("COMMAND FOR CH:" + t);
			chnum = t.toInt();
		}

		if (request.indexOf("updconfig") != -1) {
			client.println(request);
			client.setTimeout(1000);
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

		if (request.indexOf("cfginit") != -1) {
			client.println(sendConfigfile());
			break;
		}

		if (request.indexOf("init") != -1 || request.indexOf("status") != -1) {
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

		// if there is an node action process it
		if (action != "none") {
			processNodeRequest(action, chnum, value);
			if (request.indexOf("+") != -1) { // there is a command following
				request = request.substring(request.indexOf("+") + 1);
				// Serial.println("NEXT COMMAND");
				// Serial.println(request);
				moreRequests = true;
			} else
				moreRequests = false; // nothing else to do, exit
		}

		// otherwise it may be a webrequest
		else {
			webserver(request);
			moreRequests = false;
		}

		yield();
	}
	client.flush(); // trash data
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

	MCLKmsec = 0;
	MCLKsec = 0;
	MCLKminutes = 0;
	MCLKhours = 0;

	TMRmsec = 0;
	TMRsec = 0;
	TMRminutes = 0;
	TMRhours = 0;

	Serial.println("ESP LOAD COMPLETE");

	if (DMXpresent) {
		Serial.println("DMX Present");
		// Serial.end();
		dmx.init();
	} else {
		Serial.println("NO DMX");
	}
}
