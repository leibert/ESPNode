function initConfigPage() {
  //get configuration from ESP
  ajax = $.get(configEndpoint + "?getConfig");

  //fail condition, ask for help
  ajax.fail(function (data) {
    configData = null;
    $('#message').html("UNABLE TO RETRIEVE CONFIGURATION");
  });

  //message recieved, assume it's correctly formatted
  ajax.done(function (data) {
    //clear load message
    $('#message').html("");

    //parse into JSON
    configData = JSON.parse(data).ESPconfig;

    //log recieved JSON
    console.log("recieved config data", data);
    if (configData.WIFISTATUS) {
      if (configData.WIFISTATUS == "0") {
        $('#WIFIstatus').html("Not Connected");
      }
      else if (configData.WIFISTATUS == "1") {
        $('#WIFIstatus').html("Connected to " + configData.WIFISSID);
      }

      //fill out all the form fields
      $('#WIFISSID').val(configData.WIFISSID);
      $('#WIFIPW').val(configData.WIFIPW);
      $('#NODENAME').val(configData.NODENAME);
      $('#NODEDESC').val(configData.NODEDESC);
      $('#IOSRESOURCES').val(configData.IOSRESOURCES);



    }
    //there is something wrong with this JSON
    else {
      $('#message').html("CONFIGURATION MALFORMED. SET NEW CONFIGURATION");
    }

    //check WIFI status



  });

}
function sendNewConfig() {
  configDict = {};
  configDict['ESPconfig'] = {}
  configDict['ESPconfig']['WIFISSID'] = $('#WIFISSID').val();
  configDict['ESPconfig']['WIFIPW'] = $('#WIFIPW').val();
  configDict['ESPconfig']['NODENAME'] = $('#NODENAME').val();
  configDict['ESPconfig']['NODEDESC'] = $('#NODEDESC').val();

  ajax = $.ajax(
    {
      url: configEndpoint,
      type: "POST",
      data: JSON.stringify(configDict)
    });

  ajax.done(function (data) {
    console.log("dict sent", configDict);
    $('#message').html("NEED TO RELOAD");
  });

  ajax.fail(function (data) {
    console.log("dict send fail", configDict);
    $('#message').html("SEND FAILED");
  });

}