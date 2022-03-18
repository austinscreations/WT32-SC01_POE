#ifndef bootstrap_html_h
#define bootstrap_html_h

const char BOOTSTRAP_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>OXRS MQTT Config</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="expires" content="0">

  <style>
    body { padding: 0; margin: 0; font-family: Arial; }
    * { box-sizing: border-box; border-collapse: collapse; }

    label { display: block; width: 100%; margin-bottom: 0.5rem; }
    input { display: block; width: 100%; margin-bottom: 1rem; padding: 0.5rem; outline: none; border-radius: .4rem; border: 2px solid rgba(0,0,0,0.5); background-color: rgba(0,0,0,0.1); color: #000; }
    input:active, input:focus, button:hover, button:focus, button:active { box-shadow: 0 0 5px 5px #0a97d8; }

    form { padding: 1rem; }

    button { display: block; width: 100%; margin-bottom: 1rem; padding: 1rem; border-radius: .4rem; cursor: pointer; font-weight: bold; border: none; }
    input:disabled, button:disabled { cursor: not-allowed; }

    .right { float: right; margin-left: 2rem; }

    #submit { background-color: olivedrab; }
    #restart { background-color: darkorange; }
    #factoryReset { background-color: darkred; }

    .status-text { background-color: grey; margin: 1rem 0; padding: .25rem .6rem; border-radius: .4rem; font-weight: bold; text-align: center; }

    a:link, a:visited { color: #000; }
    a:hover, a:active, a:focus { color: #0a97d8; }

    @media only screen and (min-width: 768px) {
      form { padding: 0; margin: 0 auto; max-width: 40rem; }
      label { display: inline-block; width: 24%; }
      input { display: inline-block; width: 74%; }
      button { flex-grow: 1; }
      .btns { display: flex; flex-direction: row; gap: 1rem; }
    }

    @media (prefers-color-scheme: dark) {
      body { background: #1c1c1c; color: #fff; }
      input { color: #fff; border: 2px solid rgba(255,255,255,0.1); }
      a:link, a:visited { color: #fff; }
      a:hover, a:active, a:focus { color: #0a97d8; }
    }
  </style>
</head>

<body onload="handleBodyLoad()">

<form id="fw-form">

  <h1>OXRS MQTT Config <a href="/ota" class="right">OTA</a></h1>

  <div id="fw-text" class="status-text">FIRMWARE</div>

  <label for="fw-name">Name:</label>
  <input type="text" id="fw-name" disabled>

  <label for="fw-shortname">Short Name:</label>
  <input type="text" id="fw-shortname" disabled>

  <label for="fw-maker">Maker:</label>
  <input type="text" id="fw-maker" disabled>

  <label for="fw-version">Version:</label>
  <input type="text" id="fw-version" disabled>

</form>

<form id="mqtt-form">

  <div id="mqtt-text" class="status-text">LOADING...</div>

  <label for="mqtt-broker">Broker Host:</label>
  <input type="text" name="broker" id="mqtt-broker" required>

  <label for="mqtt-port">Port:</label>
  <input type="text" name="port" id="mqtt-port" value="1883" required>

  <label for="mqtt-clientid">Client ID:</label>
  <input type="text" name="clientId" id="mqtt-clientid" required>

  <label for="mqtt-username">Username:</label>
  <input type="text" name="username" id="mqtt-username">

  <label for="mqtt-password">Password:</label>
  <input type="password" name="password" id="mqtt-password" disabled>

  <label for="mqtt-topic-prefix">Topic Prefix:</label>
  <input type="text" name="topicPrefix" id="mqtt-topic-prefix">

  <label for="mqtt-topic-suffix">Topic Suffix:</label>
  <input type="text" name="topicSuffix" id="mqtt-topic-suffix">

  <div class="btns">
    <button id="submit" type="submit">Submit</button>
    <button id="restart" type="button" disabled>Restart</button>
    <button id="factoryReset" type="button" disabled>Factory Reset</button>
  </div>

</form>

<script>

var updateMqttConnectionStatusTimer;

function handleError(error)
{
  // Display the error message in RED
  document.getElementById("mqtt-text").style.background = "red";
  document.getElementById("mqtt-text").innerHTML = 'ERR: ' + error.message;

  // Stop the connection status timer so our error isn't cleared away
  clearTimeout(updateMqttConnectionStatusTimer);
}

function scheduleUpdateMqttConnectionStatus(time)
{
  // Stop any running timer
  clearTimeout(updateMqttConnectionStatusTimer);

  // Reschedule the timer
  updateMqttConnectionStatusTimer = setTimeout(function ()
  {
    checkMqttConnectionStatus();
  }, time);
}

function handleBodyLoad()
{
  fetch('/adopt')
    .then(response =>
    {
      if (!response.ok)
        return;

      return response.json();
    })
    .then(data =>
    {
      if ('firmware' in data)
      {
        let firmware = data.firmware;
        
        if ('name' in firmware)      { document.getElementById('fw-name').value = firmware.name; }
        if ('shortName' in firmware) { document.getElementById('fw-shortname').value = firmware.shortName; }
        if ('maker' in firmware)     { document.getElementById('fw-maker').value = firmware.maker; }
        if ('version' in firmware)   { document.getElementById('fw-version').value = firmware.version; }
      }
    });

  fetch('/mqtt')
    .then(response =>
    {
      if (!response.ok)
        throw new Error("Device response was not ok");

      return response.json();
    })
    .then(data =>
    {
      if ('broker' in data)      { document.getElementById('mqtt-broker').value = data.broker; }
      if ('port' in data)        { document.getElementById('mqtt-port').value = data.port; }
      if ('clientid' in data)    { document.getElementById('mqtt-clientid').value = data.clientId; }
      if ('username' in data)    { document.getElementById('mqtt-username').value = data.username; }
      if ('topicPrefix' in data) { document.getElementById('mqtt-topic-prefix').value = data.topicPrefix; }
      if ('topicSuffix' in data) { document.getElementById('mqtt-topic-suffix').value = data.topicSuffix; }

      // Trigger the username keyup event to ensure the password field
      // is enabled/disabled based on what we have loaded above
      document.getElementById('mqtt-username').dispatchEvent(new Event('keyup'));
    })
    .catch(error =>
    {
      handleError(error);
      return false;
    });

  // Initial connection status check
  scheduleUpdateMqttConnectionStatus(0);
}

function handleFormSubmit(event)
{
  event.preventDefault();

  // Validate input fields
  if (document.getElementById('mqtt-broker').value == "")
  {
    alert('Broker hostname or IP address is mandatory');
    return false;
  }

  if (document.getElementById('mqtt-clientid').value == "")
  {
    alert('Client ID is mandatory');
    return false;
  }

  if (document.getElementById('mqtt-username').value != "" && document.getElementById('mqtt-password').value == "")
  {
    alert('Password is mandatory if username specified');
    return false;
  }

  const form = event.currentTarget;
  var formData = new FormData(form);
  var formEntries = Object.fromEntries(formData.entries());

  fetch('/mqtt',
  {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify(formEntries)
  })
    .then(response =>
    {
      if (!response.ok)
        throw new Error("Device response was not ok");
    })
    .catch(error =>
    {
      handleError(error);
      return false;
    });

  // Schedule a connection status check
  scheduleUpdateMqttConnectionStatus(500);
}

function handleFormRestart()
{
  if (!confirm('Are you sure you want to restart this device?'))
    return false;

  fetch('/restart',
  {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    }
  })
    .then(response =>
    {
      if (!response.ok)
        throw new Error("Device response was not ok");
    })
    .catch(error =>
    {
      handleError(error);
      return false;
    });

  // Force update connection status to OFFLINE
  setMqttConnectionOffline();
}

function handleFormFactoryReset()
{
  if (!confirm('Are you sure you want to factory reset this device?'))
    return false;

  fetch('/factoryReset',
  {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    }
  })
    .then(response =>
    {
      if (!response.ok)
        throw new Error("Device response was not ok");
    })
    .catch(error =>
    {
      handleError(error);
      return false;
    });

  // Clear the input form
  document.getElementById("mqtt-form").reset();

  // Force update connection status to OFFLINE
  setMqttConnectionOffline();
}

function checkMqttConnectionStatus()
{
  fetch('/mqtt')
    .then(response =>
    {
      if (!response.ok)
        throw new Error("Device response was not ok");

      return response.json();
    })
    .then(data =>
    {
      if (document.getElementById("mqtt-clientid").value == "")
      {
        document.getElementById("mqtt-clientid").value = data.clientId;
      }

      setMqttConnectionStatus(data.connected);
    })
    .catch(error =>
    {
      setMqttConnectionOffline();
    });
}

function setMqttConnectionStatus(connected)
{
  if (connected === true)
  {
    document.getElementById("mqtt-text").style.background = "green";
    document.getElementById("mqtt-text").innerHTML = "MQTT CONNECTED";
  }
  else
  {
    document.getElementById("mqtt-text").style.background = "orange";
    document.getElementById("mqtt-text").innerHTML = "MQTT DISCONNECTED";
  }

  document.getElementById("submit").removeAttribute("disabled");
  document.getElementById("restart").removeAttribute("disabled");
  document.getElementById("factoryReset").removeAttribute("disabled");

  // Reschedule a connection status check every 5 secs
  scheduleUpdateMqttConnectionStatus(5000);
}

function setMqttConnectionOffline()
{
  document.getElementById("mqtt-text").style.background = "red";
  document.getElementById("mqtt-text").innerHTML = "OFFLINE";

  document.getElementById("submit").setAttribute("disabled", true);
  document.getElementById("restart").setAttribute("disabled", true);
  document.getElementById("factoryReset").setAttribute("disabled", true);

  // Reschedule a connection status check every 5 secs
  scheduleUpdateMqttConnectionStatus(5000);
}

const mqttForm = document.getElementById("mqtt-form");
mqttForm.addEventListener("submit", handleFormSubmit);

const restartButton = document.getElementById("restart");
restartButton.addEventListener("click", handleFormRestart);

const factoryResetButton = document.getElementById("factoryReset");
factoryResetButton.addEventListener("click", handleFormFactoryReset);

const mqttUsername = document.getElementById('mqtt-username');
mqttUsername.addEventListener('keyup', (event) =>
{
  const mqttPassword = document.getElementById("mqtt-password");
  mqttPassword.removeAttribute("disabled");

  if (event.target.value == "")
  {
    mqttPassword.setAttribute("disabled", true);
    mqttPassword.value = "";
  }
});

</script>

</body>
</html>
)rawliteral";

#endif
