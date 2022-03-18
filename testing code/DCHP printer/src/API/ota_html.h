#ifndef ota_html_h
#define ota_html_h

const char OTA_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>OXRS OTA Update</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="expires" content="0">

  <style>
    body { padding: 0; margin: 0; font-family: Arial; }
    * { box-sizing: border-box; border-collapse: collapse; }

    label { display: block; width: 100%; margin-bottom: 0.5rem; }
    input { display: block; width: 100%; margin-bottom: 1rem; padding: 0.5rem; outline: none; border-radius: .4rem; border: 2px solid rgba(0,0,0,0.5); background-color: rgba(0,0,0,0.1); color: #000; }
    input:active, input:focus, button:hover, button:focus, button:active { box-shadow: 0 0 5px 5px #0a97d8; }

    form { padding: 1rem; }

    .right { float: right; margin-left: 2rem; }

    button { display: block; width: 100%; margin-bottom: 1rem; padding: 1rem; border-radius: .4rem; cursor: pointer; font-weight: bold; border: none; }
    input:disabled, button:disabled { cursor: not-allowed; }

    input[type="file"]:hover { cursor: pointer; }
    .btns { position: relative; }

    button[type="submit"] { background-color: olivedrab; }

    .status-text { background-color: grey; margin: 1rem 0; padding: .25rem .6rem; border-radius: .4rem; font-weight: bold; text-align: center; }

    a:link, a:visited { color: #000; }
    a:hover, a:active, a:focus { color: #0a97d8; }

    @media only screen and (min-width: 768px) {
      form { padding: 0; margin: 0 auto; max-width: 40rem; }
      label { display: inline-block; width: 24%; }
      input { display: inline-block; width: 74%; }
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

  <h1>OXRS OTA Update <a href="/" class="right">MQTT</a></h1>

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

<form id="ota-form" enctype="multipart/form-data" action="/ota">

  <div id="ota-text" class="status-text">SELECT NEW FIRMWARE</div>

  <label for="ota-file">Firmware Binary:</label>
  <input type="file" name="file" id="ota-file" required>

  <div class="btns">
    <button id="submit" type="submit">Upload</button>
  </div>

</form>

<script>

function handleBodyLoad()
{
  fetch('/api/adopt')
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
}

function handleFormSubmit(event)
{
  event.preventDefault();

  let fileInput = document.getElementById('ota-file');
  let file = fileInput.files[0];
  if (!file) return false;

  let otaText = document.getElementById("ota-text");
  let submitBtn = document.getElementById('submit');

  otaText.style.background = "orange";
  otaText.innerHTML = "UPLOADING...";

  submitBtn.setAttribute("disabled", true);

  let request = new XMLHttpRequest();
  request.addEventListener("load", requestComplete);
  request.addEventListener('error', requestComplete);
  request.addEventListener('abort', requestComplete);

  function requestComplete(event)
  {
    if (request.status !== 200 && request.status !== 204)
    {
      otaText.style.background = "red";
      otaText.innerHTML = "UPLOAD FAILED";

      submitBtn.removeAttribute("disabled");
    }
    else
    {
      otaText.style.background = "green";
      otaText.innerHTML = "UPLOAD COMPLETE - RESTARTING";

      setTimeout(() => window.location.reload(), 10000);
    }
  }

  request.open('POST', otaForm.getAttribute('action'), true);
  request.send(fileInput.files[0]);
}

let otaForm = document.getElementById("ota-form");
otaForm.addEventListener("submit", handleFormSubmit);
</script>
</body>
</html>

)rawliteral";

#endif
