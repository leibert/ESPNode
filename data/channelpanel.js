var numberOfChannels = 0;

var channelTypes = {
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
        channelConfigData = JSON.parse(data);

        jQuery.each(channelConfigData.channels, function (i, channel) {
            // console.log("working on", channel);
            addChannelPanel(channelID = i, channel, configMode);
        });
    });
}


function addNewChannel() {

    // debugger

    channel = {};
    channel["ADDRESSING"] = "0,0,0,0";
    channel["CHMAPPING"] = "0";
    channel["NAME"] = "New Channel";
    channel["TYPE"] = "ANALOG";

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
    if (channel.TYPE == '' || channel.TYPE === undefined || channel.TYPE == null) {
        console.log("EMPTY CHANNEL");
        return
    }


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






    var faderElement = $('<span>').html('fader goes here');
    var rgbSelectorElement = $('<span>').html('RGB goes here');

    var switchElement = $('<span>').addClass("custom-control custom-switch");
    switchElement.append('<input type="checkbox" class="custom-control-input channelEditSwitch" id="channelEditSwitch' + channelID + '">');
    switchElement.append('<label class="custom-control-label" for="channelEditSwitch' + channelID + '">Edit this channel</label>');
    



    // var controllerDOM = $('');
    var controllerDOM = $('<div>');

    controllerDOM.addClass('channelController');
    controllerDOM.append(channelTypePanel);
    controllerDOM.append('<br>');



    switch (channelType) {
        case "ANALOG":
            controllerDOM.append(faderElement.attr('channelAddress', '1'));
            break;

        case "SWITCH":
            controllerDOM.append(switchElement.attr('channelAddress', '1'));
            break;

        case "RGB":
            controllerDOM.append(rgbSelectorElement.attr('channelAddress', '1'));
            break;

        case "DMXSWITCH":
            controllerDOM.append(switchElement.attr('channelAddress', '1'));
            break;

        case "DMXANALOG":
            controllerDOM.append(faderElement.attr('channelAddress', '1'));
            break;

        case "DMXRGB":
            controllerDOM.append(rgbSelectorElement.attr('channelAddress', '1'));
            break;

    }

    return controllerDOM
}




function channelEditorDOM(channelID) {

    var channel = channelConfigData[channelID];

    var channelType = channel.TYPE;

    var channelTypePanel = $('<span>').addClass('channelTypePanel');
    var typeSelector = $('<select></select>');
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





    var faderElement = $('<span>').html('fader goes here');
    var rgbSelectorElement = $('<span>').html('RGB goes here');



    var controllerDOM = $('<div>');
    // var controllerDOM = $('');

    controllerDOM.addClass('channelController');
    controllerDOM.append(channelTypePanel);
    controllerDOM.append('<br>');



    switch (channelType) {
        case "ANALOG":
            controllerDOM.append(faderElement.attr('channelAddress', '1'));
            break;

        case "SWITCH":
            controllerDOM.append(switchElement.attr('channelAddress', '1'));
            break;

        case "RGB":
            controllerDOM.append(rgbSelectorElement.attr('channelAddress', '1'));
            break;

        case "DMXSWITCH":
            controllerDOM.append(switchElement.attr('channelAddress', '1'));
            break;

        case "DMXANALOG":
            controllerDOM.append(faderElement.attr('channelAddress', '1'));
            break;

        case "DMXRGB":
            controllerDOM.append(rgbSelectorElement.attr('channelAddress', '1'));
            break;

    }

    controllerDOM.append($('<div class="saveChannelConfigButton">').html('<i class="fas fa-check-circle"></i>Save Configuration'));

    return controllerDOM
}


function saveChannelConfig(channelID){
    
    console.log('new value is',$('.channelObject[channelid=' + channelID + '] .channelTypeSelector').val());
    
    updatedChannel = {};
    updatedChannel["ADDRESSING"] = "0,0,0,0";
    updatedChannel["CHMAPPING"] = "0";
    updatedChannel["NAME"] = "New Channel";
    updatedChannel["TYPE"] = $('.channelObject[channelid=' + channelID + '] .channelTypeSelector').val();


    channelConfigData[channelID] = updatedChannel;


}

$(document).on('click', '.saveChannelConfigButton', function () {
    var channelID = $(this).parent().parent().parent().attr('channelID');
    $('.channelObject[channelid=' + channelID + '] .channelEditSwitch').prop('checked',false).trigger("change");

    console.log('save button:',channelID);

});


// $(document).on('change', '.channelTypeSelector', function () {
//     var channelID = $(this).attr('channelID');
//     console.log('type changed for channel', channelID, $(this));
//     // $('.channelObject[channelID=' + channelID + '] .channelController').html(channelControllerDOM($(this).children("option:selected").val()));
//     // $('.channelObject[channelID=' + channelID + '] .channelController').html(channelControllerDOM(channelID));

// });

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







