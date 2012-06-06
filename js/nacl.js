// Check for Native Client support in the browser before the DOM loads.
var isValidBrowser = false;
var browserSupportStatus = 0;
var checker = new browser_version.BrowserChecker(
    17,  // Minumum Chrome version.
    navigator["appVersion"],
    navigator["plugins"]);
checker.checkBrowser();

jitModule = null;  // Global application object.
statusText = 'NO-STATUS';

isValidBrowser = checker.getIsValidBrowser();
browserSupportStatus = checker.getBrowserSupportStatus();

// Handler that gets called when the NaCl module starts loading.  This
// event is always triggered when an <EMBED> tag has a MIME type of
// application/x-nacl.
function moduleDidStartLoad() {
    appendToEventLog('Begin module loading');
}

// Progress event handler.  |event| contains a couple of interesting
// properties that are used in this example:
//     total The size of the NaCl module in bytes.  Note that this value
//         is 0 until |lengthComputable| is true.  In particular, this
//         value is 0 for the first 'progress' event.
//     loaded The number of bytes loaded so far.
//     lengthComputable A boolean indicating that the |total| field
//         represents a valid length.
//
// event The ProgressEvent that triggered this handler.
function moduleLoadProgress(event) {
    var loadPercent = 0.0;
    var loadPercentString;
    if (event.lengthComputable && event.total > 0) {
        loadPercent = event.loaded / event.total * 100.0;
        loadPercentString = loadPercent + '%';
    } else {
        // The total length is not yet known.
        loadPercent = -1.0;
        loadPercentString = 'Computing...';
    }
    appendToEventLog('progress: ' + loadPercentString +
                     ' (' + event.loaded + ' of ' + event.total + ' bytes)');
}

// Handler that gets called if an error occurred while loading the NaCl
// module.  Note that the event does not carry any meaningful data about
// the error, you have to check lastError on the <EMBED> element to find
// out what happened.
function moduleLoadError() {
    appendToEventLog('error: ' + jitModule.lastError);
}

// Handler that gets called if the NaCl module load is aborted.
function moduleLoadAbort() {
    appendToEventLog('abort');
}

// When the NaCl module has loaded indicate success.
function moduleDidLoad() {
    jitModule = document.getElementById('jit');
    appendToEventLog('Finished module loading');
    updateStatus('SUCCESS');
    jitModule.postMessage("GO")
}

// Handler that gets called when the NaCl module loading has completed.
// You will always get one of these events, regardless of whether the NaCl
// module loaded successfully or not.  For example, if there is an error
// during load, you will get an 'error' event and a 'loadend' event.  Note
// that if the NaCl module loads successfully, you will get both a 'load'
// event and a 'loadend' event.
function moduleDidEndLoad() {
    // appendToEventLog('loadend');
    var lastError = event.target.lastError;
    if (lastError != undefined && lastError.length != 0) {
        appendToEventLog('lastError: ' + lastError);
    }
}


// Handle a message coming from the NaCl module.
function handleMessage(message_event) {
    appendToEventLog('nexe sez: ' + message_event.data);
}

// Set the global status message.  Updates the 'status_field' element with
// the new text.
// opt_message The message text.  If this is null or undefined, then
//     attempt to set the element with id 'status_field' to the value of
//     |statusText|.
function updateStatus(opt_message) {
    if (opt_message)
        statusText = opt_message;
    var statusField = document.getElementById('status_field');
    if (statusField) {
        statusField.innerHTML = statusText;
    }
}

// Append an event name to the 'event_log_field' element.  Event names
// are separated by a <br> tag so they get listed one per line.
// logMessage The message to append to the log.
function appendToEventLog(logMessage) {
    var eventLogField = document.getElementById('event_log_field');
    if (eventLogField.innerHTML.length == 0) {
        eventLogField.innerHTML = logMessage;
    } else {
        eventLogField.innerHTML = eventLogField.innerHTML +
            '<br />' + logMessage;
    }
}
