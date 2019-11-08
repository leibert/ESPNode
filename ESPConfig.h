//GLOBAL CONFIG VARIABLES (filled out by SPIFF read of config.dat)
char nodename[64] = "DEFESPNode";
// Node Description
char nodedesc[64] = "DEFUnconfigured ESPNode";
// WIFI Credentials. Set default as values in passwords.h
char WIFIssid[64];
char WIFIpassword[64];
//remote location of IOS Files
char IOSResources[] = "DEFioshit.net";

String buffer = "";
char bufferB[1024];

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