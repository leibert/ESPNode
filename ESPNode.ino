#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h> // Include the SPIFFS library
#include "espDMX.h"

//GLOBAL CONFIG VARIABLES (filled out by SPIFF read of config.dat)
char nodename[] = "DevMEMTEST";
// Node Description
char nodedesc[] = "Development Memory Test";

// WIFI Credentials. Set default as values in passwords.h
const char *WIFIssid;
const char *WIFIpassword;

//remote location of IOS Files
char IOSResources[] = "ioshit.net";

ESP8266WiFiMulti wifiMulti; // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80); // Create a webserver object that listens for HTTP request on port 80

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);		// send the right file to the client (if it exists)

//read config file
void processconfigfile(File f)
{
	char line[128];
	char *key, *value;
	int charcount;

	// read file line by line
	while (f.available())
	{
		//strcpy(line, "");
		// Serial.println("***************");
		// Serial.println(line);
		charcount = f.readBytesUntil('\n', line, 128);
		// Serial.println(line);
		// Serial.println(charcount);
		line[charcount + 1] = '\0';
		// Serial.println(line);

		//Serial.println(strncmp("WIFI-SSID", line, strlen("WIFI-SSID")));
		if (strncmp("WIFI-SSID", line, strlen("WIFI-SSID")) == 0)
		{
			Serial.println("SSID GRAB");
			Serial.println(line);
			key = strtok(line, "=");
			value = strtok(NULL, "\n");

			Serial.println("KV SSID");
			Serial.println(key);
			Serial.println(value);
			memcpy(WIFIssid, value, sizeof(value));
		}
		else if (strncmp("WIFI-PW", line, strlen("WIFI-PW")) == 0)
		{
			Serial.println("password GRAB");
			Serial.println(line);
			key = strtok(line, "=");
			value = strtok(NULL, "\n");

			Serial.println("KV password");
			Serial.println(key);
			Serial.println(value);
			memcpy(WIFIpassword, value, sizeof(value));
		}

		// char *key = strtok(line, '=')

		// 				 Serial.println(line + (line.indexOf("=") + 1));

		// strcpy(WIFIssid, line + (line.indexOf("=") + 1));

		// else if (line.startsWith("WIFI-password"))
		// {
		// 	Serial.println("wifipassword");
		// 	// Serial.println(line.substring(line.indexOf("=") + 1));
		// 	// strcpy(WIFIpassword,line.substring(line.indexOf("=") + 1));
		// }
		// else if (line.startsWith("IOSRESOURCES"))
		// {
		// 	Serial.println("iosresources");

		// 	// Serial.println(line.substring(line.indexOf("=") + 1));
		// 	// strcpy(IOSResources,line.substring(line.indexOf("=") + 1));
		// }
		// else if (line.startsWith("NODENAME"))
		// {
		// 	Serial.println("nodename");

		// 	// Serial.println(line.substring(line.indexOf("=") + 1));
		// 	// strcpy(nodename,line.substring(line.indexOf("=") + 1));
		// }
		// else if (line.startsWith("DESCRIPTION"))
		// {
		// 	Serial.println("nodedesc");

		// 	// Serial.println(line.substring(line.indexOf("=") + 1));
		// 	// strcpy(nodedesc,line.substring(line.indexOf("=") + 1));
		// }
		// // else if (line.startsWith("!!CHANNELS-START!!"))
		// // {
		// // 	processconfigchannels(f);
		// // }
	}
}

void setup()
{
	Serial.begin(115200); // Start the Serial communication to send messages to the computer
	delay(10);
	Serial.println('\n');

	SPIFFS.begin(); // Start the SPI Flash Files System

	// open config file
	File f = SPIFFS.open("/config.dat", "r");
	if (!f)
	{
		Serial.println("file open failed");
	}
	else
	{
		Serial.println("====== Reading from SPIFFS file =======");
		//process config file
		processconfigfile(f);
	}
	Serial.println("trying to setup wifi");
	Serial.println("SSID:");
	Serial.println(WIFIssid);
	Serial.println("PW:");
	Serial.println(WIFIpassword);

	wifiMulti.addAP(WIFIssid, WIFIpassword); // add Wi-Fi networks you want to connect to
	// wifiMulti.addAP("walrus", "woodchuck");  // add Wi-Fi networks you want to connect to

	Serial.println("Connecting ...");
	int i = 0;
	while (wifiMulti.run() != WL_CONNECTED)
	{ // Wait for the Wi-Fi to connect
		delay(250);
		Serial.print('.');
	}
	Serial.println('\n');
	Serial.print("Connected to ");
	Serial.println(WiFi.SSID()); // Tell us what network we're connected to
	Serial.print("IP address:\t");
	Serial.println(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer

	if (MDNS.begin("esp8266"))
	{ // Start the mDNS responder for esp8266.local
		Serial.println("mDNS responder started");
	}
	else
	{
		Serial.println("Error setting up MDNS responder!");
	}

	server.onNotFound([]() {								  // If the client requests any URI
		if (!handleFileRead(server.uri()))					  // send it if it exists
			server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
	});

	server.begin(); // Actually start the server
	Serial.println("HTTP server started");
}

void loop(void)
{
	server.handleClient();
}

String getContentType(String filename)
{ // convert the file extension to the MIME type
	if (filename.endsWith(".html"))
		return "text/html";
	else if (filename.endsWith(".css"))
		return "text/css";
	else if (filename.endsWith(".js"))
		return "application/javascript";
	else if (filename.endsWith(".ico"))
		return "image/x-icon";
	return "text/plain";
}

bool handleFileRead(String path)
{ // send the right file to the client (if it exists)
	Serial.println("handleFileRead: " + path);
	if (path.endsWith("/"))
		path += "index.html";				   // If a folder is requested, send the index file
	String contentType = getContentType(path); // Get the MIME type
	if (SPIFFS.exists(path))
	{														// If the file exists
		File file = SPIFFS.open(path, "r");					// Open it
		size_t sent = server.streamFile(file, contentType); // And send it to the client
		file.close();										// Then close the file again
		return true;
	}
	Serial.println("\tFile Not Found");
	return false; // If the file doesn't exist, return false
}