var config = "NO DATA RECIEVED";



$('#numpresets').on('input', function() {
  console.log("number of presets changed");
  changepresetvisbility();
});





$("form").submit(function(event) {
  event.preventDefault();
  //var configstring ="/updconfig&configsecret="+$("#configsecret").val();
  var configstring = "PRESETUPDATE##\n";
  configstring += "CONFIGSECRET=" + $("#configsecret").val() + "\n";
  configstring += "!!PRESETS-START!!\n";
  var lastfilledpreset=$('#numpresets').val();
    $('#numpresets').val("1");


  //startassembling channels
  for (var i = 1; i <= lastfilledpreset; i++) {
    //if it is a valid preset, add it to the list
    if($("#preset"+i+"desc").val()!=""||$("#preset"+i+"details").val()!=""){
      var presetstring ="~~~~~~"+parseInt($('#numpresets').val())+"\n";
      presetstring += $("#preset"+i+"desc").val()+"\n";
      presetstring += $("#preset"+i+"details").val();
      if (presetstring.slice(-1)!="\n"){
        presetstring +="\n";
      }
      presetstring +="######\n";
      $('#numpresets').val(parseInt($('#numpresets').val())+1);
      configstring += presetstring;
    }

  }

  configstring += "##PRESET-END##\n";

  //configstring+="CONFIGSTRINGEND!!\n";
  console.log("configsecret is:" + $("#configsecret").val())
  console.log(configstring);

  $.post("/updpresets", configstring, null, "text");
  buildpresetpanel();
  parsepresets(configstring);
  changepresetvisbility();
});


$(document).on('change', '.presetdescinput', function() {
  console.log("description changed to to" + $(this).val());
  var selectorstring = "#" + $(this).parent().parent().prop('id') + "desclabel";
  console.log(selectorstring);
  if ($(this).val() != "") {
    $(selectorstring).html(": " + $(this).val());
  } else {
    $(selectorstring).html("");
  }

});

function delPreset(presetID){
  $("#preset"+presetID+"desc").val("");
  $("#preset"+presetID+"desclabel").html("--TO BE DELETED");
  $("#preset"+presetID+"details").val("");
  collapsepreset(presetID);
}

function addPreset(){
  $('#numpresets').val(parseInt($('#numpresets').val())+1);
  changepresetvisbility();
}



function changepresetvisbility() {
  var panelstring = "";
  console.log("make " + $('#numpresets').val() + " presets visible");
  $('.presetcfg').each(function() {
    $(this).css('display', 'none')
  });

  for (i = 1; i <= $('#numpresets').val(); i++) {
    var selectorstring = "#preset" + i + ".presetcfg";
    console.log(selectorstring);
    $(selectorstring).css('display', 'block');
  }
}


function buildpresetpanel(maxpresets = "128") {
  console.log("building a " + maxpresets + "channel panel");
  var panelstring = "";
  for (i = 1; i <= maxpresets; i++) {
    panelstring +=
      "<div class='presetcfg' id='preset" + i + "' style='display:none'>\
          <h3><a class='expandlnk' href='javascript:;' onclick='expandpreset(" + i + ")'>+</a> PRESET " + i + "<span id='preset" + i + "desclabel'></span></h3>\
          <div class='presetdetails' style='display: none'>\
          Description: <input type='text' class='presetdescinput' id='preset" + i + "desc' /><br /> \
          <textarea class='presetdetailsinput' id='preset" + i + "details' rows='4' cols='50'></textarea>\
          <button type='button' onclick='delPreset("+i+")'>Delete</button>\
          </div>\
          <hr />\
          </div>\
          ";
  }

  $('#presets').html(panelstring);
}


function populatechannels() {
  console.log("in populatechannels");
}

function expandpreset(presetnum) {
  //console.log("expanding a Channel");
  var selectorstring = "#preset" + presetnum + " .presetdetails";
  //console.log(selectorstring);
  $(selectorstring).css('display', 'block');
  var selectorstring = "#preset" + presetnum + " .expandlnk";
  $(selectorstring).attr('onclick', 'collapsepreset(' + presetnum + ')');
  $(selectorstring).html('-');
  $(selectorstring).attr('class', 'collapselnk');
}

function collapsepreset(presetnum) {
  //console.log("expanding a Channel");
  var selectorstring = "#preset" + presetnum + " .presetdetails";
  //console.log(selectorstring);
  $(selectorstring).css('display', 'none');
  var selectorstring = "#preset" + presetnum + " .collapselnk";
  $(selectorstring).attr('onclick', 'expandpreset(' + presetnum + ')');
  $(selectorstring).html('+');
  $(selectorstring).attr('class', 'expandlnk');
}

function parsepresets(presets) {
  console.log("PRESET PARSE");
  console.log(presets)

  presets = presets.toString();
  presets = presets.split('\n');
  console.log(presets[2]);
  preset = "";
  for (var i = 0; i < presets.length; i++) {
    if (presets[i].startsWith("//")){
      //pass;
    }

    //is this the start of a new preset?
    else if (presets[i].startsWith("~~~~~~")) {
      //yes, get ID
      presetID = presets[i].substring(6);
      //presetID = presetID.trim();
      i=i+1;
      presetDESC=presets[i];
      console.log("ID# "+presetID);
      console.log("DESC: "+presetDESC);
      $("#preset"+parseInt(presetID)+"desc").val(presetDESC);
      $("#preset"+parseInt(presetID)+"desclabel").html(": "+presetDESC);
      preset="";

    }
    //end of preset add to page
    else if (presets[i].startsWith("######")) {
      console.log("PRESET IS:");
      console.log(preset);
      console.log("#preset"+i+"details")
      $("#preset"+parseInt(presetID)+"details").val(preset);
      $("#numpresets").val(presetID);
      //pass;
    }
    //must be part of the preset
    else {
      preset += presets[i]+"\n";
    }

  }
}





function getpresets() {
  console.log("Getting presets");
  $.get("/presetinit", dataType = "text", function(data) {

  presets = data.toString();
  console.log(data.toString());
  parsepresets(presets);
  changepresetvisbility();
});
}



function init() {
  console.log("init function");
  buildpresetpanel();
  getpresets();



}


init();
console.log("preset.js loaded");
