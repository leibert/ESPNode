#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "ArduinoJson.h"
#include <FS.h> // Include the SPIFFS library
#include "espDMX.h"

//Max number of channels or devices controlled
#define NUM_OF_CHANNELS 10

//predefine buffer variables
String buffer = "";
char bufferB[1024];
//holds milliseconds from start to power fade and time based routines
unsigned long lastFadeMaintain = millis();

//GLOBAL CONFIG VARIABLES, with default values (filled out by SPIFF read of config.dat)
char nodename[64] = "DEFESPNode";
// Node Description
char nodedesc[64] = "DEFUnconfigured ESPNode";

// WIFI Credentials.
char WIFIssid[64];
char WIFIpassword[64];
//is this a valid wifi connection, if not node will revert to AP mode to allow configuration
bool validWIFI = false;

//setup for softAP if needed. This provides for configuration of the node when not sucessfully connected to WIFI
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;

//Webserver setup
ESP8266WebServer server(80);			// Create a webserver object that listens for HTTP request on port 80
String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);		// send the right file to the client (if it exists)

//remote location of IOS Files
char IOSResources[] = "ioshit.net";

//DMX
//provide dmx class
DMXESPSerial dmx;
//DMX maintenance routine only needs to be run if dmx devices are configured
bool DMXpresent = false;

//keep DMX channels in a buffer to provide to dmx maintainer function
String DMXCTRLChs = "";


//CHANNEL structure
//Each device connected to the node will require a defined channel.
//Subchannels provide support for RGB and other multi-channel fixtures
struct channel
{
	//Channel name
	char name[30];

	//if inverted logic low is ON
	bool inverted;

	//Channel type
	//ANALOG - Dimmable channel connected directly to ESP
	//SWITCH - On/off channel connected directly to ESP
	//RGB - 3 channel device connected directly to ESP. Each channel dimmable
	//DMXSWITCH - On/off channel connected via DMX
	//DMXANALOG - Dimmable channel connected via DMX
	//DMXRGB - 3 channel device connected via DMX. Each channel dimmable
	//EMPTY - Undefined channel
	char type[10];

	//***************is this still needed
	
	//Mapping of channels
	//01=Single Channel
	//03=RGB

	//11=Single Channel w/ Control
	//13=RGB w/ Control
	char chmapping[2];

	//DMX address of control/fade function (only needed for certain fixtures)
	unsigned short int CTRLAddress;
	//DMX address or GPIO pin of subchannels
	unsigned short int address1;
	unsigned short int address2;
	unsigned short int address3;

	//Value of all subchannels. For fades, this is the destination value
	unsigned short int CTRLValue;
	unsigned short int address1Value;
	unsigned short int address2Value;
	unsigned short int address3Value;
	unsigned short int valueSetTime;

	//if non-zero fixture is in a fade. This is the time remaining
	unsigned short int fadeTime;

	//contains current value and fade step for each 1/10 of a second. CTRLval,addr1val,addr2val,addr3val/CTRLstep,addr1step,addr2step,addr3step
	char fadeScratch[10];
};

//array of channels, not zero indexed
struct channel channels[(NUM_OF_CHANNELS + 1)];


///////////////////////////
///////////////////////////
/////CONFIG FUNCTIONS//////
///////////////////////////
///////////////////////////

//read config file
void loadConfigJSONSettings(File f)
{	
	//buffer for line
	char line[512];

	//KV pair being processed
	char *key, *value;

	//line chancount
	int charcount;

	// read file line by line
	while (f.available())
	{
		//get number of characters in line to add string terminator
		charcount = f.readBytesUntil(',', line, 128);
		//add string terminator to end
		line[charcount] = '\0';

		//find first key in JSON

		//if there is a " from the http string
		if (line[0] == '"' && line != NULL)
		{
			//shift line over by 1
			memmove(line + 1, line, strlen(line) + 1);
			//replace first character with a space
			line[0] = ' ';
		}

		//start looking for key
		//trash any characters before " 
		key = strtok(line, "\"");

		//if the key is not null (because no opening " was found) and the string terminator hasn't been reached
		while (key != NULL && line[0] != '\0')
		{
			//key is text up until "
			key = strtok(NULL, "\"");

			//if there is no key
			if (!key)
			{
				//break out of loop
				break;
			}

			//check to see if this is a key we want a value for

			//config header can be ignored
			if (strcmp(key, "ESPconfig") == 0)
			{
			}
			//WIFI SSID
			else if (strcmp(key, "WIFISSID") == 0)
			{
				//value is text between " "					
				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
				//copy into memory
				strcpy(WIFIssid, value);
			}
			//WIFI Password
			else if (strcmp(key, "WIFIPW") == 0)
			{
				// Serial.println("wifipwset");
				value = strtok(NULL, "\"");
				value = strtok(NULL, "\"");
				//copy into memory
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

//build configuration JSON to send to browser
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

//write configuration buffer to SD card
void saveConfigJSON(String configBuffer)
{
	//open (and create if needed) SPIFF file in write mode
	File f = SPIFFS.open("/config.dat", "w+");

	//if file can't be open because of a SPIFFS error
	if (!f)
	{
		//print message to debug serial
		Serial.println("file open failed");
	}
	else
	{
		//write buffer string to file
		f.print(configBuffer);
		//confirm config written
		Serial.println("config written");
		Serial.println("resetting node");
		f.close();
		ESP.reset();
	}
}

///////////////////////////
///////////////////////////
///////////////////////////
///////////////////////////
/////CH SETUP FUNCTIONS//////
///////////////////////////
///////////////////////////
///////////////////////////
///////////////////////////
///////////////////////////
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
	if (channelJSON["NAME"])
	{
		holder = strdup(channelJSON["NAME"]);
		Serial.println(holder);
		strncpy(channels[channelID].name, holder, 30);
	}
	else
	{
		strncpy(channels[channelID].name, "", 1);
	}

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

	//COPY CHANNEL TYPE FROM CONFIG JSON
	holder = strdup(channelJSON["TYPE"]);
	Serial.println(holder);
	//Check if it's a dmx channel
	Serial.println(strstr(channelJSON["TYPE"], "DMX"));

	if (strstr(channelJSON["TYPE"], "DMX"))
	{
		Serial.println("DMX CHANNEL IS PRESENT");
		DMXpresent = true;
		DMXCTRLChs += (String(channelID) + ",");
	}
	else if (strstr(channelJSON["TYPE"], "SWITCH"))
	{
		pinMode(channels[channelID].address1, OUTPUT);
		digitalWrite(channels[channelID].address1, HIGH);
		Serial.println("CONFIGURE AS SWITCH");
		Serial.println(channels[channelID].address1);
	}
	else if (strstr(channelJSON["TYPE"], "ANALOG"))
	{
		pinMode(channels[channelID].address1, OUTPUT);
		digitalWrite(channels[channelID].address1, HIGH);
		Serial.println("CONFIGURE AS ANALOG");
		Serial.println(channels[channelID].address1);
	}
	else if (strstr(channelJSON["TYPE"], "RGB"))
	{
		pinMode(channels[channelID].address1, OUTPUT);
		digitalWrite(channels[channelID].address1, HIGH);

		pinMode(channels[channelID].address2, OUTPUT);
		digitalWrite(channels[channelID].address2, HIGH);

		pinMode(channels[channelID].address3, OUTPUT);
		digitalWrite(channels[channelID].address3, HIGH);
	}

	strncpy(channels[channelID].type, channelJSON["TYPE"], 10);

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

		snprintf(charString, 50, "\"INVERSION\":\"%s\",", channels[i].inverted);
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
void initializeChannels()
{
	///ensure clean node state
	DMXpresent = false;

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

	if (DMXpresent)
	{
		Serial.println("DMX Present");
		Serial.end();
		dmx.init();
	}
	else
	{
		Serial.println("NO DMX");
		// dmx.end();
	}
}

////////////////
////////////////
////////////////
////////////////
////////////////
////CHANNEL VALUES
////////////////
////////////////
////////////////
////////////////

String getChannelStatusJSON(int channelID)
{

	StaticJsonDocument<100> channelStatusJSON;
	String jsonString;
	jsonString = "";
	channelStatusJSON["CHANNELID"] = channelID;
	channelStatusJSON["CTRLValue"] = channels[channelID].CTRLValue;
	channelStatusJSON["AValue"] = channels[channelID].address1Value;
	channelStatusJSON["BValue"] = channels[channelID].address2Value;
	channelStatusJSON["CValue"] = channels[channelID].address3Value;

	serializeJsonPretty(channelStatusJSON, jsonString);
	channelStatusJSON.clear();
	return jsonString;
}

///CHANNEL STATUS
void sendAllChannelStatusJSON()
{

	String jsonString;

	jsonString = "{\"CHANNELS\":[";

	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, "text / html", jsonString);

	for (int i = 1; i <= NUM_OF_CHANNELS; i++)
	{
		server.sendContent(getChannelStatusJSON(i));
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

void processChannelUpdates(String updateBuffer)
{
	int updateEndPos;
	String channelUpdate;

	//cut off header
	updateBuffer = updateBuffer.substring((updateBuffer.indexOf('[') + 1));

	updateBuffer = updateBuffer.substring(0, updateBuffer.lastIndexOf('}'));

	updateBuffer = updateBuffer.substring(0, updateBuffer.lastIndexOf(']'));

	while (updateBuffer.length() > 2)
	{
		// Serial.println(updateBuffer.length());
		// Serial.println("Read char$");
		// Serial.println(channelDefinition);
		// Serial.println(updateBuffer[0]);
		switch (updateBuffer[0])
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

			channelUpdate = updateBuffer.substring(0, (updateBuffer.indexOf('}') + 1));

			updateChannelfromJSON(channelUpdate);
			break;
		//end of Channel Defintion, ignor look for next one
		case '}':
			break;
		default:
			break;
		}
		updateBuffer = updateBuffer.substring(1);
	}
}

void updateChannelfromJSON(String channelUpdate)
{

	Serial.println("parsing updates");
	Serial.println(channelUpdate);

	StaticJsonDocument<512> channelUpdateJSON;
	char *holder;

	// Deserialize the JSON document
	DeserializationError error = deserializeJson(channelUpdateJSON, channelUpdate);

	// Test if parsing succeeds.
	if (error)
	{
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}

	//GET CHANNEL ID
	int channelID = channelUpdateJSON["channelID"];
	Serial.println("channel ID is:");
	Serial.println(channelID);

	Serial.println("CTRLValue");
	Serial.println(channelUpdateJSON["CTRLValue"].as<char *>());
	channels[channelID].CTRLValue = channelUpdateJSON["CTRLValue"];

	channels[channelID].address1Value = channelUpdateJSON["AValue"];
	Serial.println("saved value");
	Serial.println(channels[channelID].address1Value);

	channels[channelID].address2Value = channelUpdateJSON["BValue"];

	channels[channelID].address3Value = channelUpdateJSON["CValue"];

	Serial.println("ch parse done");
	channelUpdateJSON.clear();
}

void maintainFades()
{
	Serial.println("in maintain fades");
	for (int i = 0; i < NUM_OF_CHANNELS; i++)
	{
		Serial.println("*****************");
		Serial.println("CHANNEL");
		Serial.println(i);
		Serial.println(channels[i].fadeTime);
		Serial.println("*****************");
	}
	lastFadeMaintain = millis()
		delay(1000);
}

void maintainLocalChannels()
{
	Serial.println("in maintain");
	for (int i = 0; i < NUM_OF_CHANNELS; i++)
	{
		Serial.println("*****************");
		Serial.println("CHANNEL");
		Serial.println(i);
		Serial.println(channels[i].type);
		Serial.println("*****************");
		if (strstr(channels[i].type, "ANALOG"))
		{
			Serial.println("analog write");
			Serial.println(channels[i].address1);
			Serial.println(channels[i].address1Value);
			Serial.println("COMPUTED");
			Serial.println((int)((channels[i].address1Value / 100.0) * 1024));
			analogWrite(channels[i].address1, (int)((channels[i].address1Value / 100.0) * 1024));
		}
		else if (strstr(channels[i].type, "SWITCH"))
		{
			Serial.println("digital write");
			Serial.println(channels[i].address1);
			Serial.println(channels[i].address1Value);
			if (channels[i].address1Value > 50)
			{
				digitalWrite(channels[i].address1, HIGH);
				Serial.println("high");
			}
			else
			{
				digitalWrite(channels[i].address1, LOW);
				Serial.println("low");
			}
		}
		else if (strstr(channels[i].type, "RGB"))
		{
			analogWrite(channels[i].address1, channels[i].address1Value);
			analogWrite(channels[i].address2, channels[i].address2Value);
			analogWrite(channels[i].address3, channels[i].address3Value);
		}
	}
}

///////////////////////////
///////////////////////////
///////////////////////////
///////////////////////////
/////DMX FUNCTIONS//////
///////////////////////////
///////////////////////////
///////////////////////////
///////////////////////////

void DMXmaintenance()
{
	Serial.println("Control Channel Maintenance");
	Serial.println(DMXCTRLChs);
	String MaintainDMXCTRLChs = DMXCTRLChs;
	while (MaintainDMXCTRLChs.length() > 0)
	{
		// Serial.println(MaintainDMXCTRLChs);
		int CC = MaintainDMXCTRLChs.substring(0, MaintainDMXCTRLChs.indexOf(","))
					 .toInt();
		// Serial.println(DMXCTRLChs.indexOf("," + 1));
		MaintainDMXCTRLChs =
			MaintainDMXCTRLChs.substring((MaintainDMXCTRLChs.indexOf(",") + 1));
		// Serial.println("A DMX CH is " + String(CC));

		// Serial.println("channels");
		// Serial.println(channels[CC].CTRLAddress);
		// Serial.println(channels[CC].address1);
		// Serial.println(channels[CC].address2);
		// Serial.println(channels[CC].address3);

		// Serial.println("value");
		// Serial.println(channels[CC].CTRLValue);
		// Serial.println(channels[CC].address1Value);
		// Serial.println(channels[CC].address2Value);
		// Serial.println(channels[CC].address3Value);

		// Serial.println("OUTPUTS");
		if (channels[CC].CTRLAddress > 0)
			dmx.write(channels[CC].CTRLAddress, (int)((channels[CC].CTRLValue / 100.0) * 255));

		// Serial.println((int)((channels[CC].CTRLValue / 100.0) * 255));
		if (channels[CC].address1 > 0)
			dmx.write(channels[CC].address1, (int)((channels[CC].address1Value / 100.0) * 255));
		// Serial.println((int)((channels[CC].address1Value / 100.0) * 255));
		if (channels[CC].address2 > 0)
			dmx.write(channels[CC].address2, (int)((channels[CC].address2Value / 100.0) * 255));
		// Serial.println((int)((channels[CC].address2Value / 100.0) * 255));
		if (channels[CC].address3 > 0)
			dmx.write(channels[CC].address3, (int)((channels[CC].address3Value / 100.0) * 255));
		// Serial.println((int)((channels[CC].address3Value / 100.0) * 255));
	}
	// Update DMX Universe
	dmx.update();
	delay(10);
}
void DMXtest()
{

	dmx.write(1, 0);
	dmx.write(2, 200);
	dmx.write(3, 0);
	// Update DMX Universe
	dmx.update();
	delay(300);
}

///////////////////////////
///////////////////////////
///////////////////////////
///////////////////////////
/////WEB SERVER FUNCTIONS//////
///////////////////////////
///////////////////////////
///////////////////////////
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
	for (int i = 0; i < server.args(); i++)
	{

		Serial.print(server.argName(i));
		Serial.println(server.arg(i));
	}

	if (server.method() == HTTP_POST)
	{
		Serial.println("updates POST");
		buffer = server.arg("plain");
		Serial.println("inputis");
		Serial.println(buffer);
		if (buffer.startsWith("{\"channelUpdates\":"))
		{
			Serial.println("recieving channel updates");
			processChannelUpdates(buffer);
			sendAllChannelStatusJSON();
		}
	}
	else if (server.method() == HTTP_GET)
	{
		Serial.println("updates GET");
		Serial.println(server.argName(0));
		// Serial.println(server.argName(1));

		if (server.args() && server.argName(0) == "getCHStatus")
		{
			Serial.println("get CH Status");
			sendAllChannelStatusJSON();
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

///////////////////////////
/////ESP FUNCTIONS//////
///////////////////////////

void setup()
{
	Serial.begin(250000); // Start the Serial communication to send messages to the computer
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
	// f = SPIFFS.open("/channelSetup.dat", "w+");
	// f.print("");
	// f.close();
	// 	// remove("/channelSetup.dat");
	// }
	// else
	// {
	// 	Serial.printf("INCREMENT CORRUPTION CHECK FROM %c to %c", corruptionCheck, corruptionCheck + 1);
	// 	updateLiferaft('C', corruptionCheck + 1);
	// }

	initializeChannels();
	analogWriteFreq(3000);
	// updateLiferaft('C', '0');
	Serial.println("load completed");
}

void loop(void)
{
	if (DMXpresent) //helps to slow down to avoid overwhelming DMX devices
		DMXmaintenance();
	// DMXtest();

	maintainLocalChannels();
	maintainFades();

	if (!validWIFI)
	{
		// Serial.println("BBBBDDDDD");
		dnsServer.processNextRequest();
	}
	// Serial.println("333AAA");
	server.handleClient();
	delay(10);
}
