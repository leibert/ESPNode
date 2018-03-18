//function changenumCH(){
//  numCH = getElementbyId("numCHinput").value;
//  console.log("number of channels changed to"+numCH);
//}

var config = "NO DATA RECIEVED";



$('#numCH').on('input', function() {
  console.log("number of channels changed");
  changeCHvisbility();
});


function changeCHtype(chID) {
  var chselector = "#" + chID;

  //Configure Values

  //1 CH Switch
  if ($(chselector + "type").val() == 1) {

    $(chselector + "valA" + ".IOlabel").html("Output Pin: ");
    $(chselector + "valA" + ".IOlabel").css("display", "inline");
    $(chselector + "valA" + ".IOinput").css("display", "inline");

    $(chselector + "valB" + ".IOlabel").css("display", "none");
    $(chselector + "valB" + ".IOinput").css("display", "none");

    $(chselector + "valC" + ".IOlabel").css("display", "none");
    $(chselector + "valC" + ".IOinput").css("display", "none");

    $(chselector + "inverted" + ".IOinput").css("display", "inline");
    $(chselector + "inverted" + ".IOlabel").css("display", "inline");

  }
  //1 CH PWM
  else if ($(chselector + "type").val() == 2) {

    $(chselector + "valA" + ".IOlabel").html("Output Pin: ");
    $(chselector + "valA" + ".IOlabel").css("display", "inline");
    $(chselector + "valA" + ".IOinput").css("display", "inline");

    $(chselector + "valB" + ".IOlabel").css("display", "none");
    $(chselector + "valB" + ".IOinput").css("display", "none");

    $(chselector + "valC" + ".IOlabel").css("display", "none");
    $(chselector + "valC" + ".IOinput").css("display", "none");

    $(chselector + "inverted" + ".IOinput").css("display", "inline");
    $(chselector + "inverted" + ".IOlabel").css("display", "inline");

  }
  //RGB PWM
  else if ($(chselector + "type").val() == 3) {

    $(chselector + "valA" + ".IOlabel").html("Output Pin (Red):&nbsp;&nbsp; ");
    $(chselector + "valA" + ".IOlabel").css("display", "inline");
    $(chselector + "valA" + ".IOinput").css("display", "inline");

    $(chselector + "valB" + ".IOlabel").html("Output Pin (Green): ");
    $(chselector + "valB" + ".IOlabel").css("display", "inline");
    $(chselector + "valB" + ".IOinput").css("display", "inline");

    $(chselector + "valC" + ".IOlabel").html("Output Pin (Blue): &nbsp;&nbsp;");
    $(chselector + "valC" + ".IOlabel").css("display", "inline");
    $(chselector + "valC" + ".IOinput").css("display", "inline");

    $(chselector + "inverted" + ".IOinput").css("display", "inline");
    $(chselector + "inverted" + ".IOlabel").css("display", "inline");
}
    //DMX
    else if ($(chselector + "type").val() == 21) {

      $(chselector + "inverted" + ".IOinput").css("display", "none");
      $(chselector + "inverted" + ".IOlabel").css("display", "none");

      $(chselector + "valA" + ".IOlabel").html("Profile: ");
      $(chselector + "valA" + ".IOlabel").css("display", "inline");
      $(chselector + "valA" + ".IOinput").css("display", "inline");

      $(chselector + "valB" + ".IOlabel").html("DMX Start Channel: ");
      $(chselector + "valB" + ".IOlabel").css("display", "inline");
      $(chselector + "valB" + ".IOinput").css("display", "inline");

      $(chselector + "valC" + ".IOlabel").css("display", "none");
      $(chselector + "valC" + ".IOinput").css("display", "none");



  } else {

    $(chselector + "ValA" + ".IOlabel").css("display", "none");
    $(chselector + "ValA" + ".IOinput").css("display", "none");

    $(chselector + "ValB" + ".IOlabel").css("display", "none");
    $(chselector + "ValB" + ".IOinput").css("display", "none");

    $(chselector + "ValC" + ".IOlabel").css("display", "none");
    $(chselector + "ValC" + ".IOinput").css("display", "none");

  }
}



$(document).on('change', '.CHtype', function() {
  console.log("input type changed to to" + $(this).val() + "--" + $(this).find(':selected').text());
  console.log($(this).parent().parent().prop('id'));
  var chID = $(this).parent().parent().prop('id');
  changeCHtype(chID);
});


$(document).on('change', '.CHdescinput', function() {
  console.log("description changed to to" + $(this).val());
  var selectorstring = "#" + $(this).parent().parent().prop('id') + "DescLabel";
  console.log(selectorstring);
  if ($(this).val() != "") {
    $(selectorstring).html(": " + $(this).val());
  } else {
    $(selectorstring).html("");
  }

});


$("form").submit(function(event) {
      event.preventDefault();
      //var configstring ="/updconfig&configsecret="+$("#configsecret").val();
      var configstring = "CONFIGSTRING##\n";
      configstring += "WIFI-SSID=" + $("#WIFI-SSID").val() + "\n";
      configstring += "WIFI-PW=" + $("#WIFI-PW").val() + "\n";
      configstring += "NODENAME=" + $("#nodename").val() + "\n";
      configstring += "DESCRIPTION=" + $("#nodedesc").val() + "\n";
      configstring += "IOSRESOURCES=" + $("#iosresources").val() + "\n";
      configstring += "CONFIGSECRET=" + $("#configsecret").val() + "\n";
      configstring += "!!CHANNELS-START!!\n";

      //startassembling channels
      for (var i = 1; i <= $("#numCH").val(); i++) {
        var channelstring = i + ", " + $("#CH" + i + "type").val() + ", ";
        channelstring += $("#CH" + i + "valA" + ".IOinput ").val() + ", " + $("#CH" + i + "valB" + ".IOinput ").val() + ", " + $("#CH" + i + "valC" + ".IOinput ").val();

        if($("#CH" + i + "inverted" + ".IOinput ").is(':checked'))
          channelstring += ", 1";
        else
          channelstring += ", 0";

        channelstring += " #" + $("#CH" + i + "desc" + ".CHdescinput ").val();

        console.log(channelstring);
        channelstring+="\n";


        if ($("#CH" + i + "type").val()!=0)
          configstring += channelstring;
      }

      configstring += "##CHANNELS-END##\n";

      //configstring+="CONFIGSTRINGEND!!\n";
      console.log("configsecret is:" + $("#configsecret").val())

      console.log(configstring);

// configstring = "CONFIGSTRING##\
// WIFI-SSID=66ADAMS\n\
// WIFI-PW=3ng1n33rs_mans1on\n\
// IOSRESOURCES=ioshit.net\n\
// NODENAME=DEVBRD\n\
// DESCRIPTION=Development Board\n\
// CONFIGSECRET=defaulterd\n\
// !!CHANNELS-START!!\n\
// 1, 3, 14, 12, 13, 0 #Right RGB Flood\n\
// 2, 3, 4, 5, 16, 0 #Left RGB Flood\n\
// 3, 20, 301, 1, 0, 0 #DMXA\n\
// 4, 20, 301, 16, 0, 0 #DMXB\n\
// 5, 20, 301, 32, 0, 0 #DMXC\n\
// ##CHANNELS-END##\n";


      $.post("http://192.168.0.90/updconfig", configstring, null, "text");

      parseconfig();
      changeCHvisbility();
    });




    function changeCHvisbility() {
      var panelstring = "";
      console.log("make " + $('#numCH').val() + " channels visible");
      $('.chdetails').each(function() {
        $(this).css('display', 'none')
      });

      for (i = 1; i <= $('#numCH').val(); i++) {
        var selectorstring = "#CH" + i + ".chdetails";
        console.log(selectorstring);
        $(selectorstring).css('display', 'block');
      }

    }


    function buildCHpanel(maxCH = 128) {
      console.log("building a " + maxCH + "channel panel");
      var panelstring = "";
      for (i = 1; i <= maxCH; i++) {
        panelstring +=
          "<div class='chdetails' id='CH" + i + "' style='display:none'>\
    <h3><a class='expandlnk' href='javascript:;' onclick='expandCH(" + i + ")'>+</a> Channel " + i + "<span id='CH" + i + "DescLabel'></span></h3>\
    <div class='chconfiguration' style='display: none'>\
    Description: <input type='text' class='CHdescinput' id='CH" + i + "desc' /><br /> \
    Type: <select class='CHtype' id='CH" + i + "type'> \
    <option value='0'>IO Disabled</option>\
    <option value='1'>Binary Switch</option>\
    <option value='2'>1 CH PWM</option>\
    <option value='3'>RGB PWM</option>\
    <option value='21'>DMX</option>\
    <select/><br> \
    <span class = 'IOlabel' id='CH" + i + "inverted' style='display:none'>Inverted Output:</span><input type='checkbox' class='IOinput' id='CH" + i + "inverted' style='display:none'/><br> \
    <span class = 'IOlabel' id='CH" + i + "valA' style='display:none'>Value A:</span><input type='text' class='IOinput' id='CH" + i + "valA' style='display:none'/><br> \
    <span class = 'IOlabel' id='CH" + i + "valB' style='display:none'>Value B:</span><input type='text' class='IOinput' id='CH" + i + "valB' style='display:none'/><br> \
    <span class = 'IOlabel' id='CH" + i + "valC' style='display:none'>Value C:</span><input type='text' class='IOinput' id='CH" + i + "valC' style='display:none'/><br> \
    </div>\
    <hr />\
    </div>\
    ";
      }

      $('#chdefinitions').html(panelstring);
    }


    function populatechannels() {
      console.log("in populatechannels");
    }

    function expandCH(chnum) {
      //console.log("expanding a Channel");
      var selectorstring = "#CH" + chnum + " .chconfiguration";
      //console.log(selectorstring);
      $(selectorstring).css('display', 'block');
      var selectorstring = "#CH" + chnum + " .expandlnk";
      $(selectorstring).attr('onclick', 'collapseCH(' + chnum + ')');
      $(selectorstring).html('-');
      $(selectorstring).attr('class', 'collapselnk');
    }

    function collapseCH(chnum) {
      //console.log("expanding a Channel");
      var selectorstring = "#CH" + chnum + " .chconfiguration";
      //console.log(selectorstring);
      $(selectorstring).css('display', 'none');
      var selectorstring = "#CH" + chnum + " .collapselnk";
      $(selectorstring).attr('onclick', 'expandCH(' + chnum + ')');
      $(selectorstring).html('+');
      $(selectorstring).attr('class', 'expandlnk');
    }

    function parsechannel(chconfig) {
      console.log(chconfig);
      var chdesc = chconfig.substring(chconfig.indexOf("#") + 1);
      console.log(chdesc);
      chconfig = chconfig.split(",");
      console.log(chconfig)
      var chselector = "#CH" + chconfig[0];
      $(chselector + "desc").val(chdesc);
      $(chselector + "type").val(chconfig[1].trim());
      $(chselector + "valA" + ".IOinput ").val(chconfig[2].trim());
      $(chselector + "valB" + ".IOinput ").val(chconfig[3].trim());
      $(chselector + "valC" + ".IOinput ").val(chconfig[4].trim());
      if(chconfig[5].trim().substring(0,1)==1)
        $(chselector + "inverted" + ".IOinput ").prop( "checked", true );
      else
        $(chselector + "inverted" + ".IOinput ").prop( "checked", false );


      changeCHtype("CH" + chconfig[0]);



    }

    function parsechannelblock(config) {
      console.log("parsing channels");
      for (var i = 1; i < config.length; i++) {
        if (config[i] == "##CHANNELS-END##") {
          console.log("done parsing channels there are " + (i - 1));
          $("#numCH").val(i - 1);
          return config.slice(i);
        } else {
          parsechannel(config[i]);
        }
      }
      return null;
    }

    function parseline(line) {
      if (line.startsWith("WIFI-SSID=")) {
        $("#WIFI-SSID").val(line.substring(line.indexOf("=") + 1));
        console.log("ssid");
      } else if (line.startsWith("WIFI-PW=")) {
        $("#WIFI-PW").val(line.substring(line.indexOf("=") + 1));
      } else if (line.startsWith("NODENAME=")) {
        $("#nodename").val(line.substring(line.indexOf("=") + 1));
      } else if (line.startsWith("DESCRIPTION=")) {
        $("#nodedesc").val(line.substring(line.indexOf("=") + 1));
      } else if (line.startsWith("IOSRESOURCES=")) {
        $("#iosresources").val(line.substring(line.indexOf("=") + 1));
      }
    }

    function parseconfig() {
      console.log(config);
      console.log("parsing config");
      config=config.toString();
      config = config.split('\n');
      console.log(config[2]);
      for (var i = 0; i < config.length; i++) {
        if (config[i] == "!!CHANNELS-START!!") {
          config = parsechannelblock(config.slice(i));
          i = 0;
        } else {
          parseline(config[i]);
        }

      }



    }

    function getconfig() {
      console.log("Getting initial config");
      $.get("http://192.168.0.90/cfginit", dataType = "text", function(data) {

        config = data.toString();
        parseconfig();
        changeCHvisbility();
      });

    }

    function init() {
      console.log("init function");
      getconfig();
      buildCHpanel();


    }


    init(); console.log("configpanel.js loaded")
