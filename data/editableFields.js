////////////////////
///EDITABLE ITEMS///
////////////////////

//SAMPLE EDITABLE FIELD
//<span class="editableItem" AJAX_fieldName="first_name" AJAX_targetObject="userDetails" AJAX_targetObjectID="REQ">INITIAL VALUE</span>
//<span class="editableItem" fieldType="textarea" AJAX_fieldName="first_name" AJAX_targetObject="userDetails" AJAX_targetObjectID="REQ">INITIAL VALUE</span>

//Element click binding
$(document).on("click", ".editableItem", function (e) {
    e.stopImmediatePropagation();
    console.log("editable field clicked- ", $(this));

    //check to see if already being edited
    if ($(this).attr("editing") == "true") {
        console.log("Check/X not clicked...do nothing");
        return null;
    } else {
        editableItemHandler($(this), "openForEdit");
    }
});

//Confirm click binding
$(document).on("click", ".editableConfirm", function (e) {
    e.stopImmediatePropagation();
    console.log(
        "confirm edit change",
        $(this)
    );
    editableItemHandler(
        $(this).parent(),
        "submitEdit"
    );
});

//Cancel edit click binding
$(document).on("click", ".editableCancel", function (e) {
    e.stopImmediatePropagation();
    console.log(
        "cancel edit change",
        $(this)
    );
    editableItemHandler(
        $(this).parent(),
        "cancelEdit"
    );
});

//editable item handler
function editableItemHandler(element, action) {
    console.log('target element', element)
    console.log('target object type', element.attr('ajax_targetobject'))
    var jqSelector =
        ".editableItem" +
        '[AJAX_targetObject="' +
        $(element).attr('AJAX_targetObject') +
        '"]' +
        '[AJAX_targetObjectID="' +
        $(element).attr('AJAX_targetObjectID') +
        '"]' +
        '[AJAX_fieldName="' +
        $(element).attr('AJAX_fieldName') +
        '"]';
    // console.log('jqSelector', jqSelector);

    //switch case for desired action
    switch (action) {
        //replace text with input box
        case "openForEdit":
            console.log("opening for edit", jqSelector);
            //value is taken from the span within the editableItem element
            var initialValue = $(jqSelector + " .editableItemValue").html();
            if (initialValue == undefined) {
                initialValue = "";
            }
            //set edit flag for parent element
            $(jqSelector).attr("editing", true);


            //check to see if there is a special type, if not assign as inputField
            var fieldType;
            if ($(jqSelector).attr("fieldType") == undefined)
                fieldType = 'inputField'
            else
                fieldType = $(jqSelector).attr("fieldType")


            //create input field
            switch (fieldType) {
                case "inputField":
                    inputFieldHTML =
                        '<input type="text" class="editableItemInputField" value="' +
                        initialValue +
                        '">';
                    break;
                case "textarea":
                    inputFieldHTML =
                        '<textarea class="editableItemInputField" style="vertical-align:middle">' + initialValue + '</textarea>';
                    break;
            }

            $(element).attr('AJAX_initialValue', initialValue);
            //overwrite current element HTML with the input text box
            $(jqSelector).html(inputFieldHTML);

            //add check button
            $(jqSelector).append(
                '<i class="fas fa-check editableConfirm" style="padding:5px">SAVE</i>'
            );
            //add cancel button
            $(jqSelector).append(
                '<i class="fas fa-times editableCancel" style="padding:5px">CANCEL</i>'
            );
            break;

        //cancel edit and reset back to text
        case "cancelEdit":
            editableItemSetup(
                $(element).attr('AJAX_targetObject'),
                $(element).attr('AJAX_targetObjectID'),
                $(element).attr('AJAX_fieldName'),
                $(element).attr('AJAX_initialValue'),
                $(jqSelector + " .editableItemInputField").val()
            );
            break;

        //submit edit via AJAM
        case "submitEdit":
            var fieldName = $(element).attr('AJAX_fieldName') || elementID;
            var value = $(jqSelector + " .editableItemInputField").val();
            var payload = element.attr("AJAX_payload") || null;
            var targetObject = $(element).attr('AJAX_targetObject') || null;
            var targetObjectID =
                $(element).attr('AJAX_targetObjectID') || null;


            //if there is a place to send the update, send it
            if (typeof xactAddress !== 'undefined') {
                $.ajax({
                    type: "POST",
                    url: xactAddress,
                    data: {
                        command: "editableFieldUpdate",
                        fieldName: fieldName,
                        value: value,
                        payload: payload,
                        targetObject: targetObject,
                        targetObjectID: targetObjectID
                    },
                    success: function (data) {
                        // console.log("data recieved from AJAX", data)
                        //use standard handler
                        if (data.startsWith('EDITABLEFIELDUPDATE')) {
                            data = data.substring(19);
                            data = JSON.parse(data);
                            renderRecievedObject(data);
                        }
                        //see if there is a customer handler defined
                        else {
                            if (typeof me.onChange === "function") {
                                AJAXUpdateHandler(data);
                            }
                        }
                    }
                    //dataType: dataType
                });
            }
            //otherwise just update it locally
            else {
                editableItemSetup(
                    targetObject,
                    targetObjectID,
                    fieldName,
                    value
                );
            }



            break;

        default:
    }
}

function editableItemPageSetup() {
    console.log("setting up page for editable items")
    //add formating for all editableItems in document
    $(".editableItem").each(function () {
        editableItemSetup(
            $(this).attr("AJAX_targetObject"),
            $(this).attr("AJAX_targetObjectID"),
            $(this).attr("AJAX_fieldName")
        );
    });
}

function setEditableItem(elementID, targetObject, targetObjectID, fieldName, initialValue) {
    var jqSelector = elementID;
    $(jqSelector).attr('AJAX_targetObject', targetObject);
    $(jqSelector).attr('AJAX_targetObjectID', targetObjectID);
    $(jqSelector).attr('AJAX_fieldName', fieldName);

    editableItemSetup(targetObject, targetObjectID, fieldName, initialValue)

}


function editableItemSetup(
    targetObject,
    targetObjectID,
    fieldName,
    initialValue = null
) {
    var jqSelector =
        ".editableItem" +
        '[AJAX_targetObject="' +
        targetObject +
        '"]' +
        '[AJAX_targetObjectID="' +
        targetObjectID +
        '"]' +
        '[AJAX_fieldName="' +
        fieldName +
        '"]';

    // console.log("jqSelector", targetObjectID, initialValue, jqSelector);

    //use the editing attribute to determine if the element has already been formated, in case editableItemSetup is being called an extra time on it. If it is under edit, flip to false and continue reset
    if (initialValue != null) {
        $(jqSelector).attr("editing", false);
    }
    else if ($(jqSelector).attr("editing") == 'true') {
        $(jqSelector).attr("editing", false);
        initialValue = $(jqSelector).attr('ajax_initialvalue');
    }
    else if ($(jqSelector).attr("editing") == 'false') {
        // console.log('this has been modified before, grab initial value from the inner slector');
        initialValue = $(jqSelector + ' .editableItemValue').html();
    }
    else {
        initialValue = $(jqSelector).html();
    }

    $(jqSelector).attr("editing", false);


    //add a span around the text of editable Fields. This is needed to differentiate for the pencil icon that is automatically needed
    //if a value is sent, place that in the span
    // if (initialValue) {
    //     // console.log("inital value rcv'd", initialValue, jqSelector);
    //     $(jqSelector).html(
    //         '<span class="editableItemValue">' + initialValue + "</span>"
    //     );
    // }
    // //otherwise use the text originally in the html
    // else {
    //     $(jqSelector).html(
    //         '<span class="editableItemValue">' + $(jqSelector).html() + "</span>"
    //     );
    // }

    $(jqSelector).html('');
    $(jqSelector).html('<span class="editableItemValue">' + initialValue + '</span>');
    //add a pencil icon to add editable fields
    $(jqSelector).append('<i class="fas fa-pencil-alt" style="padding:5px">EDIT</i>');
}

function renderRecievedObject(objectData) {
    //parse to JSON
    // objectData = JSON.parse(objectData);
    //determine type of object
    switch (objectData["data"]) {
        case "editableObject":
            // renderUpdatedEditableObject(objectData);
            console.log('pushing UI updates for', objectData);
            break;
        default:
            console.log("unable to handle returned object");
    }

    //act on each data field in the JSON
    jQuery.each(objectData["fields"], function (key, value) {
        // console.log('render this item', key, value)
        editableItemSetup(
            objectData.targetObject,
            objectData.targetObjectID,
            key,
            value
        );
    });
}

//run page setup when DOM loaded
$(document).ready(function () {
    editableItemPageSetup();
});