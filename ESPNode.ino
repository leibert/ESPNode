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
String fading = "";
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
String commandbuffer = "";

//Position in command buffer, 0=empty or not active (completion without loop)
int commandbufferpos = 0;

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

int cmdwaitms = 0;

void servefilefromSPIFFS(String filename,
						 String passthrough = "NO DATA PASSED")
{
	File f = SPIFFS.open("/" + filename, "r");
	if (!f)
	{
		Serial.println("opening " + filename + " failed");
		return;
	}
	else
	{
		Serial.println("====== Reading " + filename + " from SPIFFS =======");
	}

	Serial.println("file size is :" + String(f.size()));
	// START SENDING PAGE
	// DEFAULT HTTP HEADER
	client.println("HTTP/1.1 200 OK");
	//client.println("Content-Length: "+String(f.size()));
	client.println("Content-Type: text/html");
	client.println("Connection: close");
	client.println(""); // do not forget this one

	// read out HTML file
	String line = "";
	while (f.available())
	{
		line = f.readStringUntil('\r');
		// Serial.print(line);
		if (line.indexOf("~!@#$PASSTHROUGH~!@#$") > 0)
		{
			Serial.println("PASSING DATA THROUGH");
			Serial.println(passthrough);
			client.print(passthrough);
		}
		else
			client.print(line);
		yield();
		client.flush();
	}

	delay(10);
	client.stop();
	Serial.println("Client disconnected / Page Sent");
}

void serve404()
{
	Serial.println("404 Error");
	client.println("HTTP/1.1 404 Not Found");
	client.stop();
}

void serve200()
{
	Serial.println("200 Repsonse");
	client.println("HTTP/1.1 200 OK");
	client.stop();
}

// connect to wifi
void startWIFI()
{
	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.persistent(false);
	// connect to network using cons credentials

	WiFi.begin(ssid, password);
	// WiFi.persistent(false);

	while (WiFi.status() != WL_CONNECTED)
	{ // wait for connection
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

void webserver(String request)
{
	if (request.indexOf(".html") > 0)
	{
		String page =
			request.substring(request.indexOf("/") + 1, request.indexOf(".html"));
		Serial.println("looking for page" + page);
		servefilefromSPIFFS(page + ".html");
	}
	else if (request.indexOf(".js") > 0)
	{
		String script =
			request.substring(request.indexOf("/") + 1, request.indexOf(".js"));
		servefilefromSPIFFS(script + ".js");
	}
	else if (request.indexOf("/ HTTP/1.1") > 0)
	{
		servefilefromSPIFFS("index.html");
	}
	else
	{
		serve404();
	}
}

void loop()
{
	// if wifi connection has been lost, try to reconnect

	if (WiFi.status() != WL_CONNECTED)
	{
		delay(1);
		startWIFI();
		return;
	}
	yield();

	// Check if a client has connected
	client = server.available();
	client.setNoDelay(1);
	yield();
	if (!client)
	{
		yield();
		return; // no one connected exit loop
	}

	////////////////////////////AFTER THIS ONLY RUNS IF CLIENT CONNECTED
	// Read the first line of the request
	Serial.println("connection");
	client.setTimeout(100);
	String request = client.readStringUntil('\r');

	Serial.println(request); // serial debug output
	Serial.println("Service webpage");
	Serial.println(request);
	webserver(request);

	delay(1);
	client.stop();
}

// Setup/Init on Startup
///////////
////SETUP ARDUINO ON POWERUP
//////////
void setup()
{

	Serial.begin(250000); // Start debug serial
	delay(10);			  // wait, because things break otherwise

	Serial.println("SERIAL OUTPUT ACTIVE");

	Serial.println("Start reading file");
	SPIFFS.begin();

	// open file for reading
	File f = SPIFFS.open("/config.dat", "r");
	if (!f)
	{
		Serial.println("file open failed");
		Serial.println("using default config");
		// initChannel(1, 1, 5, 0, 0, 1, "CH1");
	}
	else
	{
		Serial.println("====== Reading from SPIFFS file =======");
		// processconfigfile(f);
	}

	Serial.println("initChannelIO");
	// initChannelIO();

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
}