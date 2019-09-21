//CODE TO COLLAPSE SECTIONS
$(document).on('click', '.collapsingSectionHeader', function (e) {
    e.stopPropagation();
    elementToChange = $(this).attr('headerFor')
    console.log('collapsing', elementToChange)
    if ($(this).attr('collapsed') == 'true') {
        expandSectionHeader(elementToChange);
    }
    else {
        collapseSectionHeader(elementToChange);
    }
});


function collapseSectionHeader(elementToChange) {
    $(elementToChange).css('display', 'none')
    $('.collapsingSectionHeader[headerfor="' + elementToChange + '"]').find('.collapserIcon').replaceWith('<i class="fas fa-chevron-circle-right collapserIcon"></i>');
    $('.collapsingSectionHeader[headerfor="' + elementToChange + '"]').attr('collapsed', 'true')
}
function expandSectionHeader(elementToChange) {
    $(elementToChange).css('display', '');
    $('.collapsingSectionHeader[headerfor="' + elementToChange + '"]').find('.collapserIcon').replaceWith('<i class="fas fa-chevron-circle-down collapserIcon"></i>');
    $('.collapsingSectionHeader[headerfor="' + elementToChange + '"]').attr('collapsed', 'false');
}
function prepareCollapsableSectionHeaders() {
    $('.collapsingSectionHeader').each(function () {
        elementToChange = $(this).attr('headerFor')
        if ($(this).attr('collapsed') == 'true') {
            collapseSectionHeader(elementToChange);
        }
        else {
            expandSectionHeader(elementToChange);
        }
    });
}



var oldSpinContent = {};

function spinnerStart(location) {
    oldSpinContent[location] = $(location).html();
    htmlString = `
    <div class="sk-folding-cube">
        <div class="sk-cube1 sk-cube"></div>
        <div class="sk-cube2 sk-cube"></div>
        <div class="sk-cube4 sk-cube"></div>
        <div class="sk-cube3 sk-cube"></div>
    </div>`;
    $(location).html(htmlString);
}

function spinnerStop(location) {
    $(location).html(oldSpinContent[location]);
    oldSpinContent[location] = '';
}




// function interceptClickEvent(e) {
//     var href;
//     var target = e.target || e.srcElement;
//     if (target.tagName === 'A') {
//         href = target.getAttribute('href');



//         // function jiraLink(resource) {
//         //     window.console.log(window.location)
//         //     baseURL = window.location.hostname
//         //     atlassianInfo = window.location.search

//         //     newUrl = resource + atlassianInfo
//         //     window.console.log("redirecting to:" + newUrl)
//         //     window.location.href = newUrl
//         // }



//         //put your logic here...
//         if (true) {

//             //tell the browser not to respond to the link click
//             e.preventDefault();
//         }
//     }
// }



// function getUrlParameter(name) {
//     name = name.replace(/[\[]/, '\\[').replace(/[\]]/, '\\]');
//     var regex = new RegExp('[\\?&]' + name + '=([^&#]*)');
//     var results = regex.exec(location.search);
//     return results === null ? '' : decodeURIComponent(results[1].replace(/\+/g, ' '));
// };

// //listen for link click events at the document level
// if (document.addEventListener) {
//     document.addEventListener('click', interceptClickEvent);
// } else if (document.attachEvent) {
//     document.attachEvent('onclick', interceptClickEvent);
// }


///////////////NAVIGATION LINKS///////////////////
$(document).on('click', '.activityPopupLink', function (e) {
    e.stopPropagation();
    openActivityDetails($(this).attr("activityid"), true, false);
});
$(document).on('click', '.activityEditPopupLink', function (e) {
    e.stopPropagation();
    openActivityDetails($(this).attr("activityid"), true, true);
});
$(document).on('click', '.reportLink', function (e) {
    e.stopPropagation();
    reportID = $(this).attr("reportID") || $(this).parent().attr("reportID")
    window.location = "/pm/reportViewer/" + reportID + "";
});
$(document).on('click', '.supressNAV', function (e) {
    e.stopPropagation();
    console.log("NAV SUPRESSED");
});



$(document).on("click", ".projectLink", function (e) {
    e.stopPropagation();
    console.log($(this).parent().attr("projectID"));
    window.location = "/pm/map/" + $(this).parent().attr("projectID") + "";
});
$(document).on("click", ".projectSettingsLink", function (e) {
    e.stopPropagation();
    console.log($(this).parent().attr("projectID"));
    window.location = "/pm/map/" + $(this).parent().attr("projectID") + "#projectSettings";
});
$(document).on("click", ".projectAdminsLink", function (e) {
    e.stopPropagation();
    console.log($(this).parent().attr("projectID"));
    window.location = "/pm/map/" + $(this).parent().attr("projectID") + "#projectAdmins";
});

