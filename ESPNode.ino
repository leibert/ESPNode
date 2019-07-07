#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <FS.h> // Include the SPIFFS library
#include "espDMX.h"

//GLOBAL CONFIG VARIABLES (filled out by SPIFF read of config.dat)
char nodename[64] = "ESPNode";
// Node Description
char nodedesc[64] = "Unconfigured ESPNode";

// WIFI Credentials. Set default as values in passwords.h
char WIFIssid[64];
char WIFIpassword[64];

//remote location of IOS Files
char IOSResources[] = "ioshit.net";

bool validWIFI = false;

//setup for softAP if needed
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);

DNSServer dnsServer;
ESP8266WebServer server(80); // Create a webserver object that listens for HTTP request on port 80

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);		// send the right file to the client (if it exists)

String buffer = "";

//read config file
void loadConfigJSONSettings(File f)
{
	char line[128];
	char *key, *value;
	int charcount;

	// read file line by line
	while (f.available())
	{
		charcount = f.readBytesUntil(',', line, 128);
		// Serial.println(line);
		// Serial.println(charcount);
		line[charcount] = '\0';
		Serial.println("!!!!!!!!!!!!!!!!!");
		Serial.println(line);
		Serial.println("!!!!!!!!!!!!!!!!!");
		//make sure we're in the ESPconfig JSON object
		// key = strtok(line, "\"ESPCONFIG\"");
		Serial.print(line[0]);
		if (line[0] == '"' && line != NULL)
		{
			memmove(line + 1, line, strlen(line) + 1);
			line[0] = ' ';
			Serial.println("line shifted");
			Serial.println(line);
		}

		key = strtok(line, "\"");

		while (key != NULL && line[0] != '\0')
		{
			Serial.println("***************");
			Serial.println(line);
			key = strtok(NULL, "\"");
			Serial.println("KEY:");
			if (!key)
			{
				Serial.println("loop break");
				break;
			}
			// Serial.println(key);

			//check to see if this is a key we want a value for
			if (strcmp(key, "ESPconfig") == 0)
			{
				Serial.println("header--ignore");
			}
			else if (strcmp(key, "WIFISSID") == 0)
			{
				Serial.println("wifissidset");
				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
				strcpy(WIFIssid, value);
			}
			else if (strcmp(key, "WIFIPW") == 0)
			{
				Serial.println("wifipwset");

				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
				Serial.print("wifi pw");
				Serial.print(value);
				strcpy(WIFIpassword, value);
			}
			else if (strcmp(key, "NODENAME") == 0)
			{
				Serial.println("nodenameset");

				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
				strcpy(nodename, value);
			}
			else if (strcmp(key, "NODEDESC") == 0)
			{
				Serial.println("nodedescset");

				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
				strcpy(nodedesc, value);
			}
			else if (strcmp(key, "IOSRESOURCES") == 0)
			{
				Serial.println("IOSRESOURCESset");

				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
				strcpy(IOSResources, value);
			}
			else
			{
				Serial.println("trash this value...unknown key");
				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
			}

			//done processing this key...advance to next
			// Serial.println("line now");
			// Serial.print(line);
			// if (line[0])
			// {
			key = strtok(NULL, "\"");
			// }
		}

		// //Serial.println(strncmp("WIFI-SSID", line, strlen("WIFI-SSID")));
		// if (strstr(line, "WIFISSID"))
		// {
		// 	Serial.println("SSID GRAB");
		// 	Serial.println(line);
		// 	key = strtok(line, "\"");
		// 	key = strtok(NULL, "\"");
		// 	value = strtok(NULL, "\"");
		// 	value = strtok(NULL, "\"");

		// 	Serial.println("KV SSID");
		// 	Serial.println(key);
		// 	Serial.println(value);
		// 	strcpy(WIFIssid, value);
		// }
		// else if (strstr(line, "WIFIPW"))
		// {
		// 	Serial.println("password GRAB");
		// 	Serial.println(line);
		// 	key = strtok(line, "\"");
		// 	key = strtok(NULL, "\"");
		// 	value = strtok(NULL, "\"");
		// 	value = strtok(NULL, "\"");

		// 	Serial.println("KV password");
		// 	Serial.println(key);
		// 	Serial.println(value);
		// 	strcpy(WIFIpassword, value);
		// }

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

void setupWebServer(bool configMode)
{
	if (MDNS.begin("esp8266"))
	{ // Start the mDNS responder for esp8266.local
		Serial.println("mDNS responder started");
	}
	else
	{
		Serial.println("Error setting up MDNS responder!");
	}

	if (configMode)
	{
		Serial.println("switch to config index");
		server.on("/", HTTP_GET, [&]() {
			const char *metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"0; url=http://192.168.1.1/configpanel.html\" /></head><body><p>redirecting...</p></body>";
			server.send(200, "text/html", metaRefreshStr);
		});
	}

	server.on("/CFG", HTTP_ANY, handleWEBConfig); // Call the 'handleLED'

	server.onNotFound([&]() { // If the client requests any URI
		if (!handleFileRead(server.uri()))
		{ // send it if it exists
			Serial.println("captive catch");
			const char *metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"0; url=/\" /></head><body><p>redirecting...</p></body>";
			server.send(200, "text/html", metaRefreshStr);
		}
	});

	server.begin(); // Actually start the server
	Serial.println("HTTP server started");
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
		loadConfigJSONSettings(f);
	}
	Serial.println("trying to setup wifi");
	Serial.println("SSID:");
	Serial.println(WIFIssid);
	Serial.println("PW:");
	Serial.println(WIFIpassword);
	WiFi.persistent(false);
	// WiFi.begin(WIFIssid, WIFIpassword); // add Wi-Fi networks you want to connect to
	WiFi.begin("badap", "badpw"); // add Wi-Fi networks you want to connect to

	Serial.println("Connecting ...");
	int i = 0;
	while (WiFi.status() != WL_CONNECTED)
	{ // Wait for the Wi-Fi to connect
		//assume there will be a valid wifi connection
		validWIFI = true;
		delay(250);
		Serial.print('.');
		i++;
		if (i > 10)
		{
			Serial.println("WIFI FAIL");
			validWIFI = false;
			break;
		}
	}

	if (validWIFI)
	{
		Serial.println('\n');
		Serial.print("Connected to ");
		Serial.println(WiFi.SSID()); // Tell us what network we're connected to
		Serial.print("IP address:\t");
		Serial.println(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer
		setupWebServer(false);
	}
	else
	{
		WiFi.disconnect();
		WiFi.mode(WIFI_AP);
		WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
		WiFi.softAP("ESPCONFIG");
		dnsServer.start(DNS_PORT, "*", apIP);
		setupWebServer(true);
	}
}

void loop(void)
{
	if (!validWIFI)
	{
		dnsServer.processNextRequest();
	}
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
	// path = "html" + path;
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

void handleWEBConfig()
{
	Serial.println("in handle WEB config");
	Serial.println("recieved ARGS are");
	// for (int i = 0; i < server.args(); i++)
	// {

	// 	Serial.print(server.argName(i));
	// 	Serial.println(server.arg(i));
	// }

	if (server.method() == HTTP_POST)
	{
		Serial.println("CONFIG POST");
		buffer = server.argName(0);
		Serial.println("inputis");
		Serial.println(buffer);
		if (buffer.startsWith("{\"ESPconfig\":"))
		{
			saveConfigJSON(buffer);
		}
	}
	else if (server.method() == HTTP_GET)
	{
		Serial.println("CONFIG GET");
		Serial.println(server.argName(0));
		// Serial.println(server.argName(1));

		if (server.args() && server.argName(0) == "getConfig")
		{
			Serial.println("createConfigJSON");
			server.send(200, "text/html", getConfigJSON());
		}
		Serial.println();
	}
	else
	{
		Serial.println("OTHER METHOD");
	}

	// Serial.println(server.arg);
}

String getConfigJSON()
{
	buffer = "";
	buffer += "{";

	buffer += "\"ESPconfig\":{";
	buffer += "\"WIFISSID\":\"" + String(WIFIssid) + "\",";
	buffer += "\"WIFIPW\":\"" + String(WIFIpassword) + "\",";
	buffer += "\"WIFISTATUS\":\"" + String(validWIFI) + "\",";
	buffer += "\"NODENAME\":\"" + String(nodename) + "\",";
	buffer += "\"NODEDESC\":\"" + String(nodedesc) + "\",";
	buffer += "\"IOSRESOURCES\":\"" + String(IOSResources) + "\",";
	buffer += "\"NUMCH\":\"TEST\"";
	buffer += "}";

	buffer += "}";
	return buffer;
}

void saveConfigJSON(String configBuffer)
{
	File f = SPIFFS.open("/config.dat", "w+");
	if (!f)
	{
		Serial.println("file open failed");
	}
	else
	{
		f.print(configBuffer);
		Serial.println("config written");
		Serial.println("resetting node");
		f.close();
		ESP.reset();
	}
}