
var testjson = `{
    "CHANNELS": {
        "1": {
            "ID": "1",
                "NAME": "Sample DMX",
                    "TYPE": "DMX",
                        "PROFILE": "Amazon1",
                            "ADDRESS": "15"
        },
        "2": {
            "ID": "2",
                "NAME": "Sample DIGITAL",
                    "TYPE": "DIGITAL",
                        "PIN": "2"
        },
        "3": {
            "ID": "3",
                "NAME": "Sample analog",
                    "TYPE": "ANALOG",
                        "PIN": "22"
        },
        "4": {
            "ID": "4",
                "NAME": "SAMPLE RGB DIM",
                    "TYPE": "RGB",
                        "PIN": "14,12,33"
        }
    }
}`;








function loadChannelsPanel(configMode = false) {

    //get channel config
    ajax = $.get(configEndpoint + "?getChannelConfig");

    //fail condition, ask for help
    ajax.fail(function (data) {
        channelConfigData = null;
        $('#message').html("UNABLE TO RETRIEVE CONFIGURATION");
    });

    //message recieved, assume it's correctly formatted
    ajax.done(function (data) {
        channelConfigData = JSON.parse(data);

        jQuery.each(channelConfigData, function (i, channel) {
            console.log("working on", channel);
            addChannelPanel(channel, configMode);
        });
    });
}






function addChannelPanel(channel, configMode = false, showValue = false) {
    var channelDOM = $('<div></div>').addClass('channelObject');
    channelDOM.attr('channelID', channel.ID);
    var header = $('<h3></h3>').addClass('channelName');
    header.html(channel.NAME);

    var channelType = $('<span>').addClass('channelType');
    switch (channel.TYPE) {
        case 'DMX':
            channelType.html('DMX')
            break;
        case 'Digital':
            channelType.html('DIGITAL')
            break;
        case 'Analog':
            channelType.html('ANALOG')
            break;
    }



    //build the dom element
    channelDOM.append(header);
    channelDOM.append(channelType);



    if (configMode) {
        channelDOM.addClass('editableChannel');
        header.addClass('editableField')
    }

    $('#channelPanel').append(channelDOM);
    // $('#channelPanel').append("asasd");
}











$('#numCH').on('input', function () {
    console.log("number of channels changed");
    changeCHvisbility();
});
