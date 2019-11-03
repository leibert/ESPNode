#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
// #include "cjson/cJSON.h"
// #include "cjson/cJSON.c"
#include "ArduinoJson.h"
// #include "cJSON.h"
#include <FS.h> // Include the SPIFFS library
#include "espDMX.h"

#define NUM_OF_CHANNELS 10

String buffer = "";

char bufferB[1024];

//GLOBAL CONFIG VARIABLES (filled out by SPIFF read of config.dat)
char nodename[64] = "DEFESPNode";
// Node Description
char nodedesc[64] = "DEFUnconfigured ESPNode";
// WIFI Credentials. Set default as values in passwords.h
char WIFIssid[64];
char WIFIpassword[64];
//remote location of IOS Files
char IOSResources[] = "DEFioshit.net";

//variables for WIFI
bool validWIFI = false;

//setup for softAP if needed
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;

//Webserver setup
ESP8266WebServer server(80);			// Create a webserver object that listens for HTTP request on port 80
String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);		// send the right file to the client (if it exists)

//CHANNEL struct
struct channel
{
	//Channel name
	char name[30];

	//Channel type
	//0=UNSET
	//1=Analog
	//2=PWM
	//5=DMX
	char type[10];

	//Mapping of channels
	//01=Single Channel
	//03=RGB

	//11=Single Channel w/ Control
	//13=RGB w/ Control
	char chmapping[2];

	//
	unsigned short int CTRLAddress;
	unsigned short int address1;
	unsigned short int address2;
	unsigned short int address3;

	unsigned short int CTRLValue;
	unsigned short int address1Value;
	unsigned short int address2Value;
	unsigned short int address3Value;
	unsigned short int valueSetTime;

	unsigned short int fadeTime;
	char fadeScratch[10];
};

//array of channels, not zero indexed
struct channel channels[(NUM_OF_CHANNELS + 1)];

///////////////////////////
/////CONFIG FUNCTIONS//////
///////////////////////////

//read config file
void loadConfigJSONSettings(File f)
{

	char line[512];
	char *key, *value;
	int charcount;

	// read file line by line
	while (f.available())
	{
		// memset(&line[0], 0, sizeof(line));
		charcount = f.readBytesUntil(',', line, 128);
		line[charcount] = '\0';
		// Serial.println("!!!!!!!!!!!!!!!!!");
		// Serial.println(line);
		// Serial.println("!!!!!!!!!!!!!!!!!");
		// Serial.print(line[0]);
		if (line[0] == '"' && line != NULL)
		{
			memmove(line + 1, line, strlen(line) + 1);
			line[0] = ' ';
			// Serial.println("line shifted");
			// Serial.println(line);
		}

		key = strtok(line, "\"");

		while (key != NULL && line[0] != '\0')
		{
			// Serial.println("***************");
			// Serial.println(line);
			key = strtok(NULL, "\"");
			// Serial.println("KEY:");

			if (!key)
			{
				// Serial.println("loop break");
				break;
			}
			// Serial.println(key);

			//check to see if this is a key we want a value for
			if (strcmp(key, "ESPconfig") == 0)
			{
				// Serial.println("header--ignore");
			}
			else if (strcmp(key, "WIFISSID") == 0)
			{
				// Serial.println("wifissidset");
				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
				strcpy(WIFIssid, value);
			}
			else if (strcmp(key, "WIFIPW") == 0)
			{
				// Serial.println("wifipwset");

				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
				// Serial.print("wifi pw");
				// Serial.print(value);
				strcpy(WIFIpassword, value);
			}
			else if (strcmp(key, "NODENAME") == 0)
			{
				// Serial.println("nodenameset");

				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
				strcpy(nodename, value);
			}
			else if (strcmp(key, "NODEDESC") == 0)
			{
				// Serial.println("nodedescset");

				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
				strcpy(nodedesc, value);
			}
			else if (strcmp(key, "IOSRESOURCES") == 0)
			{
				// Serial.println("IOSRESOURCESset");

				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
				strcpy(IOSResources, value);
			}
			else
			{
				// Serial.println("trash this value...unknown key");
				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
			}

			key = strtok(NULL, "\"");
		}
	}
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

void saveChannelConfigJSON(String configBuffer)
{
	File f = SPIFFS.open("/channelSetup.dat", "w+");
	if (!f)
	{
		Serial.println("file open failed");
	}
	else
	{
		f.print(configBuffer);
		Serial.println("config written");
		// Serial.println("resetting node");
		f.close();
		initializeChannels();
		// ESP.reset();
	}
}
///////////////////////////
/////CH SETUP FUNCTIONS//////
///////////////////////////

//read ChannelSetup file
void loadChannelSetupJSONSettings(File f)
{
	char line[512];
	char *key, *value;
	char channelDefinition[256];
	int charcount;

	// read file line by line looking for any KVs
	while (f.available())
	{
		//Read Key
		charcount = f.readBytesUntil(':', line, 512);
		line[charcount] = '\0';

		Serial.println("-----!!!!!!!!!!!!!!!!!");
		Serial.println(line);
		Serial.println("+++++!!!!!!!!!!!!!!!!!");

		Serial.println(line[0]);

		//Remove opening paren or space
		if (line[0] == '"' && line != NULL)
		{
			memmove(line + 1, line, strlen(line) + 1);
			line[0] = ' ';
			Serial.println("line shifted");
			Serial.println(line);
		}

		//get any text before closing paren
		key = strtok(line, "\"");

		Serial.println("key check");
		Serial.println(key);

		//if valid key and the line hasn't terminated
		while (key != NULL && line[0] != '\0')
		{
			Serial.println("***************");
			Serial.println(line);
			key = strtok(NULL, "\"");
			Serial.println("KEYLC:");
			Serial.println(key);
			Serial.println(line);
			Serial.println(f);
			if (!key)
			{
				Serial.println("loop break");
				break;
			}

			//check to see if this is a key we want a value for

			//start of channel block
			if (strcmp(key, "CHANNELS") == 0)
			{
				Serial.println("found channel block");

				//try to find start of ch array
				while (f.readBytes(channelDefinition, 1))
				{
					// Serial.println("Read Byte$");
					// Serial.println(channelDefinition);
					// Serial.println(channelDefinition[0]);
					switch (channelDefinition[0])
					{
					//start of channel dict
					case '[':
						break;
					//end of channel dict, kick out of this loop
					case ']':
						continue;
						break;
					//start of Channel Defintion, initialize on it
					case '{':
						charcount = f.readBytesUntil('}', channelDefinition, 256);
						Serial.println("channel definition found, initing");

						//make it a pretty JSON by adding {}
						memmove(channelDefinition + 1, channelDefinition, strlen(channelDefinition) + 1);
						channelDefinition[0] = '{';
						channelDefinition[charcount + 1] = '}';
						channelDefinition[charcount + 2] = '\0';

						initChannelFromJSON(channelDefinition);
						break;
					//end of Channel Defintion, ignor look for next one
					case '}':
						break;
					default:
						break;
					}
				}
			}
		}
	}
}

void initChannelFromJSON(char *channelString)
{
	Serial.println("parsing channel");
	Serial.println(channelString);
	StaticJsonDocument<512> channelJSON;
	char *holder;

	// Deserialize the JSON document
	DeserializationError error = deserializeJson(channelJSON, channelString);

	// Test if parsing succeeds.
	if (error)
	{
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}

	//GET CHANNEL ID
	int channelID = channelJSON["CHANNELID"];
	Serial.println("channel ID is:");
	Serial.println(channelID);

	//COPY CHANNEL NAME FROM CONFIG JSON
	holder = strdup(channelJSON["NAME"]);
	Serial.println(holder);
	strncpy(channels[channelID].name, holder, 30);

	//COPY CHANNEL TYPE FROM CONFIG JSON
	holder = strdup(channelJSON["TYPE"]);
	Serial.println(holder);
	strncpy(channels[channelID].type, channelJSON["TYPE"], 10);

	//SET CHANNEL MAPPING
	strncpy(channels[channelID].chmapping, channelJSON["CHMAPPING"], 2);

	//GET ADDRESSING INFO AND SPLIT

	holder = strdup(channelJSON["ADDRESSING"]);
	Serial.println("address coppied");

	Serial.println("ADDRESS IS:");
	Serial.println(holder);
	holder = strtok(holder, ",");
	Serial.println(holder);
	channels[channelID].address1 = atoi(strdup(holder));
	Serial.println(holder);
	channels[channelID].address2 = atoi(strtok(NULL, ","));
	channels[channelID].address3 = atoi(strtok(NULL, ","));
	channels[channelID].CTRLAddress = atoi(strtok(NULL, ","));

	Serial.println("SEGMENTS ARE:");
	Serial.println(channels[channelID].CTRLAddress);
	Serial.println(channels[channelID].address1);
	Serial.println(channels[channelID].address2);
	Serial.println(channels[channelID].address3);

	Serial.println(channelJSON["NAME"].as<char *>());
	Serial.println(channelJSON["ADDRESSING"].as<char *>());

	Serial.println("ch parse done");
	channelJSON.clear();
}

void initChannel(File buffer, char *channelString, char *channelJSON)
{
	char *key, *value;
	// global line;
	int channelID = atoi(channelString);
	int charcount;
	bool channelEnd = false;

	Serial.println("channel config for:");
	Serial.println(channelID);
	Serial.println("is:");
	Serial.println(channelJSON);
	Serial.println("gggggggggggggggggggggggg");
	Serial.println(buffer);

	while (buffer.available())
	{
		Serial.println("rrrrrrrrrrrrrrrrrrrrrrrrrrr***********");
		Serial.println(buffer);
		charcount = buffer.readBytesUntil(',', channelJSON, 1024);
		channelJSON[charcount] = '\0';
		Serial.println("***************************************");
		Serial.println(channelJSON);
		Serial.println("******CHECK FOR END AND START KV EXTRACT******");

		char *pPosition = strchr(channelJSON, '}');
		if (pPosition != NULL)
		{
			Serial.println("END OF CHANNEL FOUND");
			channelEnd = true;
		}

		// strcpy(testbuffer, line);
		// const char channelTerm = '}';
		// char *c = channelJSON;
		// while (*c)
		// {
		// 	Serial.printf("%s %c", *c, channelTerm);
		// 	if (strchr(*c, channelTerm))
		// 	{
		// 		Serial.println("end of Channel Found");
		// 		channelEnd=true;
		// 	}

		// 	c++;
		// }

		Serial.println("strtok sequence");
		key = strtok(channelJSON, "\"");
		// Serial.println(key);
		value = strtok(NULL, "\"");
		// Serial.println(key);
		value = strtok(NULL, "\"");
		// Serial.println(value);
		// value = strtok(NULL, "\"");
		// Serial.println(value);

		Serial.println("KEYIC:");
		Serial.println(key);
		if (!key)
		{
			Serial.println("skip iteration");
			continue;
		}
		Serial.println("VALUEa:");
		Serial.println("VALUE:");
		Serial.println(value);
		Serial.println("$$$$$$$$$$$$$$$$$$$$$$$$$$$");

		Serial.println("b4chkA");

		if (!value)
		{
			continue;
			// Serial.println("putup null string");
			// // char nullstring[8]="nullSTR";
			// // strcpy(value,nullstring);
			// value[0]='A';
			// Serial.println("putup null char1 done");
			// value[1]='\0';
			// Serial.println("putup null string done");
			// // strcpy(value, (" ").c_str());
		}

		Serial.println("chkA");
		if (strcmp(key, "NAME") == 0)
		{
			strncpy(channels[channelID].name, value, 50);
		}
		else if (strcmp(key, "TYPE") == 0)
		{
			if (value == "DELETE")
			{
				Serial.println("DELETE THIS CHANNEL");
				for (int c = channelID; c < NUM_OF_CHANNELS + 1; c++)
				{
					channels[c] = channels[c + 1];
				}

				struct channel emptyChannel;
				channels[NUM_OF_CHANNELS] = emptyChannel;

				return;
			}
			strncpy(channels[channelID].type, value, 20);
		}
		else if (strcmp(key, "CHMAPPING") == 0)
		{
			strncpy(channels[channelID].chmapping, value, 20);
		}
		else if (strcmp(key, "ADDRESSING") == 0)
		{
			Serial.println("ADDRESS IS:");
			Serial.println(value);
			channels[channelID].CTRLAddress = atoi(strtok(value, "/"));
			Serial.println(value);
			channels[channelID].address1 = atoi(strtok(NULL, "/"));
			channels[channelID].address2 = atoi(strtok(NULL, "/"));
			channels[channelID].address3 = atoi(strtok(NULL, "/"));

			Serial.println("SEGMENTS ARE:");
			Serial.println(channels[channelID].CTRLAddress);
			Serial.println(channels[channelID].address1);
			Serial.println(channels[channelID].address2);
			Serial.println(channels[channelID].address3);
		}
		Serial.println("chkB");

		if (channelEnd)
		{
			Serial.println("end of channel JSON reached");
			//the end of the channelJSON has been found
			return;
		}
	}

	//Extract key
}

void sendChannelConfigJSON()
{
	bool firstChildElement = true;
	strcpy(bufferB, "{\"CHANNELS\":[\n");
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, "text/html", bufferB);
	// Serial.println(i);

	char charString[50];

	for (int i = 1; i <= NUM_OF_CHANNELS; i++)
	{
		// Serial.print("working on channel");
		// Serial.println(i);
		strcpy(bufferB, "");

		if (channels[i].type == 0 || channels[i].type == NULL)
		{
			Serial.println("skipb");
			continue;
		}

		//if this isn't the first element, add a comma to properly format JSON
		if (!firstChildElement)
		{
			strcat(bufferB, ",\n");
		}
		firstChildElement = false;

		//Create JSON object with the channel index as ID
		// strcat(bufferB, "\"%d"");

		snprintf(charString, 50, "{\"CHANNELID\":\"%i\",", i);
		strcat(bufferB, charString);

		snprintf(charString, 30, "\"NAME\":\"%s\",", channels[i].name);
		strcat(bufferB, charString);

		// Serial.println("parsing ch type");
		// Serial.println(channels[i].type);
		snprintf(charString, 50, "\"TYPE\":\"%s\",", channels[i].type);
		strcat(bufferB, charString);

		snprintf(charString, 50, "\"CHMAPPING\":\"%s\",", channels[i].chmapping);
		strcat(bufferB, charString);

		snprintf(charString, 50, "\"ADDRESSING\":\"%i,%i,%i,%i\"}", channels[i].CTRLAddress, channels[i].address1, channels[i].address2, channels[i].address3);
		strcat(bufferB, charString);

		server.sendContent(bufferB);
	}
	Serial.println("looping done");
	server.sendContent("]\n}");

	server.sendContent("");
	Serial.println("need to close server");
}

void saveChannelSetupJSON(String buffer)
{
	File f = SPIFFS.open("/channelSetup.dat", "w+");
	if (!f)
	{
		Serial.println("file open failed");
	}
	else
	{
		f.print(buffer);
		Serial.println("channelSetup written");
		Serial.println("resetting node");
		f.close();
		ESP.reset();
	}
}

// void parseChannelValueUpdates(){
// 	StaticJsonDocument<512> channelStatusJSON;

// 	// Parse JSON object
// 	DeserializationError error = deserializeJson(doc, client);
// 	if (error) {
// 		Serial.print(F("deserializeJson() failed: "));
// 		Serial.println(error.c_str());
// 		return;

// 	channelStatusJSON.clear();

// }

///CHANNEL STATUS
void sendChannelStatusJSON()
{

	StaticJsonDocument<100> channelStatusJSON;
	String jsonString;

	jsonString = "{\"CHANNELS\":[";

	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, "text / html", jsonString);

	for (int i = 1; i <= NUM_OF_CHANNELS; i++)
	{
		Serial.println(i);
		jsonString = "";
		channelStatusJSON["CHANNELID"] = i;
		channelStatusJSON["valA"] = channels[i].address1Value;
		channelStatusJSON["valB"] = channels[i].address2Value;
		channelStatusJSON["valC"] = channels[i].address3Value;
		serializeJsonPretty(channelStatusJSON, jsonString);
		channelStatusJSON.clear();
		server.sendContent(jsonString);
		// Serial.println(jsonString);
		if (i != NUM_OF_CHANNELS)
		{
			server.sendContent(",");
		}
	}
	Serial.println("Asfasfasf");

	server.sendContent("]}");
	Serial.println("sssssssff");
	server.sendContent("");
	// server.stop();
	Serial.println("sggbbbbsgsff");

	Serial.println("ch status summary sent");
	// sendChannelStatusJSON.printTo(json);upp

	// Serial.println('gggga');

	// Serial.println(json);

	// Serial.println('a');
	// server.setContentLength(measureJsonPretty(channelStatusJSON));
	// Serial.println('B');

	// // server.send(200, "application/json", json);
	// // server.send(200, "text/plain", "this works as well");
	// // server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	// server.send(200, "text/html", "");

	// Serial.println("Ddasfsd");
	// server.sendContent(json);
	// Serial.println("Ddd");
	// server.sendContent("");

	// // serializeJson(doc, Serial);
	// // server.println(F("HTTP/1.0 200 OK"));
	// // server.println(F("Content-Type: application/json"));
	// // server.println(F("Connection: close"));
	// // server.print(F("Content-Length: "));
	// // server.println(measureJsonPretty(channelStatusJSON));
	// // server.println();

	// // Write JSON document
	// // serializeJsonPretty(channelStatusJSON, server);

	// // Disconnect
	// server.stop();
	// Serial.println("wqwe");
	// channelStatusJSON.clear();
	// Serial.println("ddfsf");
}

void saveChannelDefaultsJSON(String buffer)
{
	File f = SPIFFS.open("/channelDefaults.dat", "w+");
	if (!f)
	{
		Serial.println("file open failed");
	}
	else
	{
		f.print(buffer);
		Serial.println("channel Defaults written");
		Serial.println("resetting node");
		f.close();
		ESP.reset();
	}
}

///////////////////////////
/////WEB SERVER FUNCTIONS//////
///////////////////////////

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

	server.on("/CFG", HTTP_ANY, handleWEBConfig);  // Call the 'handleLED'
	server.on("/UPD", HTTP_ANY, handleWEBUpdates); // Call the 'handleLED'

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

///HANDLER FOR ALL CONFIG FUNCTIONS

void handleWEBUpdates()
{
	Serial.println("in handle WEB updates");
	Serial.println("recieved ARGS are");
	// for (int i = 0; i < server.args(); i++)
	// {

	// 	Serial.print(server.argName(i));
	// 	Serial.println(server.arg(i));up
	// }

	if (server.method() == HTTP_POST)
	{
		Serial.println("updates POST");
		buffer = server.argName(0);
		Serial.println("inputis");
		Serial.println(buffer);
	}
	else if (server.method() == HTTP_GET)
	{
		Serial.println("updates GET");
		Serial.println(server.argName(0));
		// Serial.println(server.argName(1));

		if (server.args() && server.argName(0) == "getCHStatus")
		{
			Serial.println("get CH Status");
			sendChannelStatusJSON();
		}
	}
	else
	{
		Serial.println("OTHER METHOD");
	}
	delay(25);
	Serial.println("end of web updates");
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
		else if (buffer.startsWith("{\"CHANNELS\":"))
		{
			saveChannelConfigJSON(buffer);
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
		else if (server.args() && server.argName(0) == "getChannelConfig")
		{
			Serial.println("sending CH setup JSON");
			sendChannelConfigJSON();
			Serial.println("asdgwdgaw");
		}
		Serial.println("wferwer");
	}
	else
	{
		Serial.println("OTHER METHOD");
	}
	delay(25);
	Serial.println("end of web config");
}

char getLiferaft(char param)
{
	char liferaft[6];

	File f = SPIFFS.open("/liferaft.dat", "r");
	if (!f)
	{
		f.close();
		f = SPIFFS.open("/liferaft.dat", "w+");
		Serial.println("no liferaft, create new one");

		liferaft[0] = '0';
		liferaft[1] = '0';
		liferaft[2] = '0';
		liferaft[3] = '\0';

		Serial.printf("%s\n", liferaft);

		Serial.println("liferaft created, resetting node");
		f.close();
	}
	else
	{
		f.readBytes(liferaft, 2);
		// Serial.println("RAW LIFERAFT");
		// Serial.println(liferaft);
		// Serial.printf("check is CFG%c CH%c\n", liferaft[0], liferaft[1]);
	}

	switch (param)
	{
	//return entire check string
	// case 'A':
	// 	return liferaft;

	//return channelcfg check string
	case 'C':
		return liferaft[2];

	//return config check string
	case 'F':
		return liferaft[1];
	}
}

void updateLiferaft(char param, char value)
{
	char liferaft[6];

	//use getLiferaft to check if the file exists and create if not
	getLiferaft('C');

	File f = SPIFFS.open("/liferaft.dat", "w+");

	f.readBytes(liferaft, 2);
	// Serial.printf("ready for write check is CFG%c CH%c\n", liferaft[0], liferaft[1]);
	Serial.printf("value: %c   param: %c", value, param);

	switch (param)
	{
	//update channelcfg check char
	case 'C':
		liferaft[2] = value;
		break;

	//update config check char
	case 'F':
		liferaft[1] = value;
		break;
	}

	liferaft[3] = '\0';
	// f.printf("%s\n", liferaft);

	Serial.println("\nnew liferaft");
	Serial.printf("%s\n", liferaft);

	f.close();
}

void initializeChannels()
{
	File f = SPIFFS.open("/channelSetup.dat", "r");
	Serial.println("channel file open");

	if (!f)
	{
		Serial.println("file open failed, create empty file");
		f.close();
		f = SPIFFS.open("/channelSetup.dat", "w+");
		f.print("");
		f.close();
		loadChannelSetupJSONSettings(f);
	}
	else
	{
		Serial.println("====== Reading channel settings from SPIFFS file =======");
		// Serial.println("File Content:");

		//process config file
		loadChannelSetupJSONSettings(f);
		f.close();
	}
}

///////////////////////////
/////ESP FUNCTIONS//////
///////////////////////////

void setup()
{
	Serial.begin(115200); // Start the Serial communication to send messages to the computer
	delay(10);
	Serial.println('\n');

	SPIFFS.begin(); // Start the SPI Flash Files System

	// char corruptionCheck = '0';

	//check to see if the configuration was valid

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
	WiFi.begin(WIFIssid, WIFIpassword); // add Wi-Fi networks you want to connect to
	// WiFi.begin("badap", "badpw"); // add Wi-Fi networks you want to connect to

	Serial.println("Connecting ...");
	int i = 0;
	while (WiFi.status() != WL_CONNECTED)
	{ // Wait for the Wi-Fi to connect
		//assume there will be a valid wifi connection
		validWIFI = true;
		delay(250);
		Serial.print('.');
		i++;
		///////////////////////////////
		///////////////////////////////
		//////////////////NEEED TO REVERT THIS TO A LONGER SERACH TIME
		///////////////////////////////
		///////////////////////////////
		if (i > 30)
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

	// //for debugging
	Serial.println("loading channels");
	// Serial.println("JSON END");

	// corruptionCheck = getLiferaft('C');
	// if (corruptionCheck > '2')
	// {
	// 	Serial.println("corruption check failed, remove file");
	// 	f = SPIFFS.open("/channelSetup.dat", "w+");
	// 	f.print("");
	// 	f.close();
	// 	// remove("/channelSetup.dat");
	// }
	// else
	// {
	// 	Serial.printf("INCREMENT CORRUPTION CHECK FROM %c to %c", corruptionCheck, corruptionCheck + 1);
	// 	updateLiferaft('C', corruptionCheck + 1);
	// }

	initializeChannels();
	// updateLiferaft('C', '0');
	Serial.println("load completed");
}

void loop(void)
{
	if (!validWIFI)
	{
		// Serial.println("BBBBDDDDD");
		dnsServer.processNextRequest();
	}
	// Serial.println("333AAA");
	server.handleClient();
	delay(20);
}
