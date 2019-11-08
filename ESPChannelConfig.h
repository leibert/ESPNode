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
