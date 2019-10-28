var numberOfChannels = 0;

var channelTypes = {
    "EMPTY": "Open Channel",
    "ANALOG": "Analog Dimmer",
    "SWITCH": "On/Off Switch",
    "RGB": "3 Channel RGB",
    "DMXSWITCH": "DMX On/Off Switch",
    "DMXANALOG": "DMX DIMMER",
    "DMXRGB": "DMX 3-Channel RGB"
};

function loadChannelsPanel(configMode = false) {

    //get channel config
    ajax = $.get(configEndpoint + "?getChannelConfig");

    //fail condition, ask for help
    ajax.fail(function (data) {
        //THIS CREATES A RACE CONDITON
        //!!!!!!!!!!!!!!!FIX!!!!!!!!!!!!!!!!!!!!!!!! 
        //if(chann)
        // channelConfigData = {};
        $('#message').html("UNABLE TO RETRIEVE CONFIGURATION");
    });

    //message recieved, assume it's correctly formatted
    ajax.done(function (data) {
        channelConfigData = JSON.parse(data).CHANNELS;

        //get channel count
        jQuery.each(channelConfigData, function (i, channel) {
            if (channel.TYPE == '' || channel.TYPE === undefined || channel.TYPE == null) {

            }
            else {
                console.log("channel found", i);
                numberOfChannels = parseInt(channel.CHANNELID);
            }


        });


        jQuery.each(channelConfigData, function (i, channel) {

            // console.log("working on", i, (numberOfChannels);
            if (i >= (numberOfChannels)) {
                console.log("bail out of extra channels");
                return
            }


            if (channel.TYPE == '' || channel.TYPE === undefined || channel.TYPE == null) {
                console.log("EMPTY CHANNEL");
                channel.TYPE = 'EMPTY'
                channel.NAME = 'OPEN CHANNEL'
            }


            if (!(channel.TYPE in channelTypes)) {
                channel.TYPE = "NOSUPPORT";
                channel.NAME = 'CHANNEL TYPE NOT SUPPORTED'

            }




            addChannelPanel(channelID = channel.CHANNELID, channel, configMode);
        });
    });
}


function addNewChannel() {


    channel = {};
    channel["ADDRESSING"] = "0,0,0,0";
    channel["CHMAPPING"] = "0";
    channel["NAME"] = "New Channel";
    channel["TYPE"] = "ANALOG";

    debugger


    var channelID = numberOfChannels + 1;
    numberOfChannels += 1;

    channelConfigData[channelID] = channel;
    addChannelPanel(channelID, channel, configMode = 1);




}





function addChannelPanel(channelID, channel, configMode = false, showValue = false) {

    //build a header that will also serve as the collapser for the channel details
    var channelHeader = $('<div></div>').addClass('collapsingSectionHeader');
    channelHeader.attr('headerFor', "#channel" + channelID);



    //create the dom for the channel details
    var channelDOM = $('<div></div>').addClass('channelObject');
    channelDOM.attr('ID', "channel" + channelID);
    channelDOM.attr('channelID', channelID);

    var channelDescription = $('<h3></h3>').addClass('channelName');
    channelDescription.append(channel.NAME);


    //build the header
    channelHeader.append('<i class="fas fa-chevron-circle-down collapserIcon"></i> ');
    channelHeader.append('#' + channelID + ': ');
    channelHeader.append(channelDescription);
    //get everything on the same line
    channelDescription.css('display', 'inline');










    console.log("channel parse", channelID, channel.TYPE, channel);



    if (configMode) {
        var switchElement = $('<span>').addClass("custom-control custom-switch");
        switchElement.append('<input type="checkbox" class="custom-control-input channelEditSwitch" id="channelEditSwitch' + channelID + '">');
        switchElement.append('<label class="custom-control-label" for="channelEditSwitch' + channelID + '">Edit this channel</label>');
        channelDOM.append(switchElement);
        channelDOM.append('<br>');
    }

    if (configMode) {


    }
    else {

    }

    var channelController = $('<div class="channelControllerContainer">').html(channelControllerDOM(channelID))
    channelDOM.append(channelController);
    // numberOfChannels += 1;

    //build the dom element





    $('#channelPanel').append('<hr />');
    $('#channelPanel').append(channelHeader);
    $('#channelPanel').append(channelDOM);
    // $('#channelPanel').append("asasd");


    if (configMode) {
        channelDOM.addClass('editableChannel');
        channelDescription.addClass('editableItem');
        channelDescription.attr('AJAX_fieldName', 'channelName');
        channelDescription.attr('AJAX_targetObject', 'channel');
        channelDescription.attr('AJAX_targetObjectID', channelID);


    }

    editableItemPageSetup();
}



function createFaderElement(channelID, subchannel = 1, value = 0, label = 'Output') {
    var faderElement = $('<span>').addClass("channelFaderContainer");;
    faderElement.append($('<label for="channelFader' + channelID + 's' + subchannel + '" subchannel="' + subchannel + '">' + label + ': ' + value + '</label>'));
    faderElement.append($('<input type="range" class="custom-range channelFader" id="channelFader' + channelID + 's' + subchannel + '" subchannel="' + subchannel + '" value="' + value + '">'));
    return faderElement
}
function createRGBElement(channelID, subchannel = 0, valueR = 0, valueG = 0, valueB = 0, label = '') {
    var rgbSelectorElement = $('<span>').addClass("channelRGBContainer");;
    rgbSelectorElement.append(createFaderElement(channelID, (subchannel + 1), valueR, 'Red'));
    rgbSelectorElement.append(createFaderElement(channelID, (subchannel + 2), valueG, 'Green'));
    rgbSelectorElement.append(createFaderElement(channelID, (subchannel + 3), valueB, 'Blue'));
    return rgbSelectorElement
}
function createSwitchElement(channelID, subchannel = 1, value = 0, label = 'Output') {
    var switchElement = $('<span>').addClass("custom-control custom-switch");
    if (value) {
        switchElement.append('<input type="checkbox" class="custom-control-input channelSwitch" id="channelSwitch' + channelID + 's' + subchannel + '" subchannel="' + subchannel + '" checked="true">');
        switchElement.append('<label class="custom-control-label" for="channelSwitch' + channelID + 's' + subchannel + '"subchannel="' + subchannel + '">' + label + ': ON</label>');
    }
    else {
        switchElement.append('<input type="checkbox" class="custom-control-input channelSwitch" id="channelSwitch' + channelID + 's' + subchannel + '" subchannel="' + subchannel + '">');
        switchElement.append('<label class="custom-control-label" for="channelSwitch' + channelID + 's' + subchannel + '"subchannel="' + subchannel + '">' + label + ': OFF</label>');
    }
    return switchElement
}


function channelControllerDOM(channelID) {
    var channel = channelConfigData[channelID];
    var channelType = channel.TYPE;

    var channelTypePanel = $('<span>').addClass('channelTypePanel');
    if (channel.TYPE == 'NEW') {
        console.log("NEW CHANNEL");
        channelTypePanel.html('Undefined');
    }
    else {
        $.each(channelTypes, function (key, value) {
            if (key == channel.TYPE) {
                channelTypePanel.html(value);


            }
        });
    }






    // var controllerDOM = $('');
    var controllerDOM = $('<div>');

    controllerDOM.addClass('channelController');
    controllerDOM.append(channelTypePanel);
    controllerDOM.append('<br>');
    controllerDOM.append('<br>');



    switch (channelType) {
        case "ANALOG":
            controllerDOM.append(createFaderElement(channelID, 1, 0));
            break;

        case "SWITCH":
            controllerDOM.append(createSwitchElement(channelID, 1, 0));
            break;

        case "RGB":
            controllerDOM.append(createRGBElement(channelID, 1, 0, 0, 0));
            break;

        case "DMXSWITCH":
            controllerDOM.append(createSwitchElement(channelID, 1, 0));
            break;

        case "DMXANALOG":
            controllerDOM.append(createFaderElement(channelID, 1, 0));
            break;

        case "DMXRGB":
            controllerDOM.append(createRGBElement(channelID, 1, 0, 0, 0));
            break;
        case "EMPTY":
            controllerDOM.append("Select a channel type to define this channel");
            break;

    }

    return controllerDOM
}


function createChannelEditField(channelID, subchannel, pinout = 0, label = 'Output Pin') {
    var channelEditField = $('<tr>').addClass('channelEditField');
    channelEditField.append('<td><label class="channelEditField-label" for="channelEditField' + channelID + 's' + subchannel + '"subchannel="' + subchannel + '">' + label + ': </label></td>');
    channelEditField.append('<td><input type="number" class="channelEditField" id="channelEditField' + channelID + 's' + subchannel + '" subchannel="' + subchannel + '" value="' + pinout + '" size="5"></td>');

    return channelEditField
}

function channelEditorDOM(channelID) {

    var channel = channelConfigData[channelID];


    var channelType = channel.TYPE;


    var channelTypePanel = $('<span>').addClass('channelTypePanel');
    var typeSelector = $('<select></select>');

    var addressing = channel['ADDRESSING'].split(',');

    typeSelector.addClass('channelTypeSelector');
    typeSelector.attr('channelID', channelID);
    $.each(channelTypes, function (key, value) {
        typeSelector.attr('channelID', channelID).append($('<option/>', {
            value: key,
            text: value
        }));
    });
    typeSelector.val(channelType);
    channelTypePanel.append(typeSelector);








    var editorDOM = $('<div>');
    // var editorDOM = $('');

    editorDOM.addClass('channelEditor');
    editorDOM.append(channelTypePanel);
    editorDOM.append('<br>');
    editorDOM.append('<br>');


    editTable = $('<table>');



    switch (channelType) {
        case "ANALOG":

            editTable.append(createChannelEditField(channelID, 1, addressing[0]));
            break;

        case "SWITCH":
            editTable.append(createChannelEditField(channelID, 1, addressing[0]));
            break;

        case "RGB":
            editTable.append(createChannelEditField(channelID, 1, addressing[0], "Red Output"));
            editTable.append('<BR>');
            editTable.append(createChannelEditField(channelID, 2, addressing[1], "Green Output"));
            editTable.append('<BR>');
            editTable.append(createChannelEditField(channelID, 3, addressing[2], "Blue Output"));
            editTable.append('<BR>');
            break;

        case "DMXSWITCH":
            editTable.append(createChannelEditField(channelID, 1, addressing[0]));
            break;

        case "DMXANALOG":
            editTable.append(createChannelEditField(channelID, 1, addressing[0]));
            break;

        case "DMXRGB":
            editTable.append(createChannelEditField(channelID, 1, addressing[0]));
            editTable.append('<BR>');
            editTable.append(createChannelEditField(channelID, 1, addressing[1]));
            editTable.append('<BR>');
            editTable.append(createChannelEditField(channelID, 1, addressing[2]));
            editTable.append('<BR>');
            break;

    }

    editorDOM.append(editTable);
    editorDOM.append('<BR>');
    editorDOM.append($('<div class="saveChannelConfigButton">').html('<i class="fas fa-check-circle"></i>Save Configuration'));
    editorDOM.append($('<div class="deleteChannelConfigButton">').html('<i class="fas fa-check-circle"></i>DELETE CHANNEL'));

    return editorDOM
}


function saveChannelConfig(channelID, mode = "SAVE") {

    console.log('new value is', $('.channelObject[channelid=' + channelID + '] .channelTypeSelector').val());

    updatedChannel = {};
    if (mode == "SAVE") {

        var addressing = [0, 0, 0, 0];

        if ($('.channelObject[channelid=' + channelID + '] .channelEditField[subchannel=1]').val() > 0) {
            addressing[0] = $('.channelObject[channelid=' + channelID + '] .channelEditField[subchannel=1]').val();
        }
        else {
            addressing[0] = 0;
        }

        if ($('.channelObject[channelid=' + channelID + '] .channelEditField[subchannel=2]').val() > 0) {
            addressing[1] = $('.channelObject[channelid=' + channelID + '] .channelEditField[subchannel=2]').val();
        }
        else {
            addressing[1] = 0;
        }

        if ($('.channelObject[channelid=' + channelID + '] .channelEditField[subchannel=3]').val() > 0) {
            addressing[2] = $('.channelObject[channelid=' + channelID + '] .channelEditField[subchannel=3]').val();
        }
        else {
            addressing[2] = 0;
        }

        if ($('.channelObject[channelid=' + channelID + '] .channelEditField[subchannel=4]').val() > 0) {
            addressing[3] = $('.channelObject[channelid=' + channelID + '] .channelEditField[subchannel=4]').val();
        }
        else {
            addressing[3] = 0;
        }



        updatedChannel["ADDRESSING"] = addressing.join('/');
        updatedChannel["CHMAPPING"] = "0";
        updatedChannel["NAME"] = $('.channelName[ajax_targetobjectid=' + channelID + '] .editableItemValue').html();
        updatedChannel["TYPE"] = $('.channelObject[channelid=' + channelID + '] .channelTypeSelector').val();
    }
    else if (mode == "DELETE") {
        if (channelID == numberOfChannels) {
            console.log("this is the last channel, just remove")

        }
        else {
            console.log("this channel is in the middle upshift up")
        }
        updatedChannel["TYPE"] = "DELETE"

    }



    channelConfigData[channelID] = updatedChannel;


}





function buildChannelConfigJSON() {
    var jsonString = '';
    // jsonString += '{';
    // jsonString += '"CHANNELS": {';

    // for (i = 1; i <= Object.keys(channelConfigData).length; i++) {
    //     jsonString += '"' + i + '": {';
    //     Object.keys(channelConfigData[i]).forEach(function (key, index) {
    //         jsonString += '"' + key + '":"' + channelConfigData[i][key] + '"';
    //         if (index < Object.keys(channelConfigData[i]).length - 1) {
    //             jsonString += ',';
    //         }

    //     });
    //     jsonString += '}';
    //     if (i < Object.keys(channelConfigData).length) {
    //         jsonString += ',';
    //     }


    // }


    // jsonString += '}';
    // jsonString += '}';



    jsonDict = {}
    jsonDict['CHANNELS'] = {}

    for (i = 1; i <= Object.keys(channelConfigData).length; i++) {

        if (!channelConfigData[i] || !channelConfigData[i].TYPE || channelConfigData[i].TYPE == '' || channelConfigData[i].TYPE == 'EMPTY') {
            continue
        }
        else {
            jsonDict['CHANNELS'][i] = channelConfigData[i]
        }

    }

    jsonString = JSON.stringify(jsonDict)


    console.log(jsonString);
    return jsonString

}


function pushChannelConfigJSONtoNode() {

    ajax = $.ajax(
        {
            url: configEndpoint,
            type: "POST",
            data: buildChannelConfigJSON()
        });

    ajax.done(function (data) {
        console.log("chconfig sent", channelConfigData);
        $('#message').html("NEED TO RELOAD");
    });

    ajax.fail(function (data) {
        console.log("chconfig send fail", channelConfigData);
        $('#message').html("SEND FAILED");
    });


}


$(document).on('click', '.saveChannelConfigButton', function () {
    var channelID = $(this).parent().parent().parent().attr('channelID');
    $('.channelObject[channelid=' + channelID + '] .channelEditSwitch').prop('checked', false).trigger("change");

    console.log('save button:', channelID);

});

$(document).on('click', '.deleteChannelConfigButton', function () {
    var channelID = $(this).parent().parent().parent().attr('channelID');
    // $('.channelObject[channelid=' + channelID + ']').parent().remove();

    console.log('delete channel :', channelID);
    saveChannelConfig(channelID, 'DELETE');

});

$(document).on('change', '.channelTypeSelector', function () {
    var channelID = $(this).attr('channelID');
    console.log('type changed for channel', channelID, $(this));
    saveChannelConfig(channelID);
    $('.channelObject[channelID=' + channelID + '] .channelControllerContainer').html(channelEditorDOM(channelID));
    // $('.channelObject[channelID=' + channelID + '] .channelController').html(channelControllerDOM(channelID));

});

$(document).on('change', '.channelEditSwitch', function () {
    var channelID = $(this).parent().parent().attr('channelID');
    console.log('edit channel', channelID, $(this).value, $(this));
    if ($(this)[0].checked) {
        $('.channelObject[channelid=' + channelID + '] .channelControllerContainer').html(channelEditorDOM(channelID));
    }
    else {
        saveChannelConfig(channelID);
        $('.channelObject[channelid=' + channelID + '] .channelControllerContainer').html(channelControllerDOM(channelID));

    }


    // $('.channelObject[channelID=' + channelID + '] .channelController').html(channelControllerDOM($(this).children("option:selected").val()));
});







