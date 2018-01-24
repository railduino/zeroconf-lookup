//
// Written by Volker Wiegand <volker@railduino.de>
//
// See https://github.com/railduino/zeroconf-lookup
//

document.getElementById("header").textContent = chrome.i18n.getMessage("htmlHeader");
document.getElementById("waiting").textContent = chrome.i18n.getMessage("htmlWaiting");

var cancel = document.getElementById("cancel");
cancel.textContent = chrome.i18n.getMessage("htmlCancel");
cancel.onclick = function() { window.close(); };

document.addEventListener('DOMContentLoaded', function () {
  chrome.runtime.sendNativeMessage('com.railduino.zeroconf_lookup', { cmd: "Lookup" }, function(response) {
    if (typeof response !== 'object') {
      document.getElementById("waiting").textContent = chrome.i18n.getMessage("htmlError");
      document.getElementById("message").textContent = chrome.i18n.getMessage("htmlNoServer");
      document.getElementById("spinner").style.display = "none";
    }

    var str = JSON.stringify(response, null, 2);
    var i, server, a, div, hr, br, span;
    var server_list = document.getElementById("server_list");
    server_list.textContent = "";

    if (response.result.length > 0) {
      for (i in response.result) {
        server = response.result[i];
        a = document.createElement('a');
        a.textContent = server.name;
        a.href = server.url;
        a.classList.add("server", "button");
        server_list.appendChild(a);
        br = document.createElement('br');
        server_list.appendChild(br);
        if (Array.isArray(server.txt)) {
          server.txt.forEach(function(item) {
            line = document.createElement('span');
            line.textContent = item;
            server_list.appendChild(line);
            br = document.createElement('br');
            server_list.appendChild(br);
          });
        } else if (server.txt != null) {
          line = document.createElement('span');
          line.textContent = server.txt;
          server_list.appendChild(line);
          br = document.createElement('br');
          server_list.appendChild(br);
        }
        hr = document.createElement('hr');
        server_list.appendChild(hr);
      }
    } else {
      div = document.createElement('div');
      div.textContent = chrome.i18n.getMessage("htmlNoServer");
      server_list.appendChild(div);
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

