//
// Written by Volker Wiegand <volker@railduino.de>
//

function onError(error) {
  var err_msg = `${error}`;

  document.getElementById("waiting").textContent = chrome.i18n.getMessage("htmlError");
  document.getElementById("message").textContent = err_msg;
  document.getElementById("spinner").style.display = "none";
  console.log(err_msg);
}

document.getElementById("header").textContent = chrome.i18n.getMessage("htmlHeader");
document.getElementById("waiting").textContent = chrome.i18n.getMessage("htmlWaiting");

var cancel = document.getElementById("cancel");
cancel.textContent = chrome.i18n.getMessage("htmlCancel");
cancel.onclick = function() { window.close(); };

document.addEventListener('DOMContentLoaded', function () {
  chrome.runtime.sendNativeMessage('com.railduino.zeroconf_lookup', { text: "Lookup" }, function(response) {
    var str = JSON.stringify(response, null, 2);

    var server_list = document.getElementById("server_list");
    server_list.textContent = "";

    var i, server, a, hr, br, span;
    for (i in response.result) {
      server = response.result[i];
      a = document.createElement('a');
      a.textContent = server.name;
      a.href = server.url;
      a.classList.add("server", "button");
      server_list.appendChild(a);
      if (server.txt !== "") {
        br = document.createElement('br');
        server_list.appendChild(br);
        span = document.createElement('span');
        span.textContent = server.txt;
        server_list.appendChild(span);
      }
      hr = document.createElement('hr');
      server_list.appendChild(hr);
    }

    document.addEventListener("click", (e) => {
      if (e.target.classList.contains("server")) {
        chrome.tabs.query({active: true, currentWindow: true}, function(tabs) {
          chrome.tabs.update(tabs[0].id, {
            active: true,
            url: e.target.href
          });
          window.close();
        });
      }
      e.preventDefault();
    }, false);
  });
});

