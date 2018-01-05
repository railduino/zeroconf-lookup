//
// Written by Volker Wiegand <volker@railduino.de>
//
// License: See https://github.com/volkerwiegand/zeroconf-lookup/blob/master/LICENSE
//

function onError(error) {
  console.log(`Error: ${error}`);
}

function onResponse(response) {
  var str = JSON.stringify(response, null, 2);

  var server_list = document.getElementById("server_list");
  server_list.innerHTML = "";

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
      var tab = browser.tabs.query({active: true, currentWindow: true});
      tab.then((tabs) => {
        browser.tabs.update(tabs[0].id, {
          active: true,
          url: e.target.href
        });
        window.close();
      });
    }
    e.preventDefault();
  }, false);
}

var elem;
elem = document.getElementById("header");
elem.textContent = browser.i18n.getMessage("htmlHeader");

elem = document.getElementById("waiting");
elem.textContent = browser.i18n.getMessage("htmlWaiting");

elem = document.getElementById("cancel");
elem.textContent = browser.i18n.getMessage("htmlCancel");
elem.onclick = function() { window.close(); };

var sending = browser.runtime.sendNativeMessage("com.railduino.zeroconf_lookup", "Lookup");
sending.then(onResponse, onError);

