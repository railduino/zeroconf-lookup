package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
)

const (
	chromeRoot   = "/etc/opt/chrome/native-messaging-hosts"
	chromiumRoot = "/etc/chromium/native-messaging-hosts"
	mozillaRoot  = "/usr/lib/mozilla/native-messaging-hosts"
	chromeUser   = "~/.config/google-chrome/NativeMessagingHosts"
	chromiumUser = "~/.config/chromium/NativeMessagingHosts"
	mozillaUser  = "~/.mozilla/native-messaging-hosts"
)

type mozillaManifest struct {
	Name              string   `json:"name"`
	Description       string   `json:"description"`
	Path              string   `json:"path"`
	Type              string   `json:"type"`
	AllowedExtensions []string `json:"allowed_extensions"`
}

type chromeManifest struct {
	Name           string   `json:"name"`
	Description    string   `json:"description"`
	Path           string   `json:"path"`
	Type           string   `json:"type"`
	AllowedOrigins []string `json:"allowed_origins"`
}

func installManifests() {
	hostPath, err := os.Executable()
	if err != nil {
		log.Fatal(err)
	}
	hostInfo := "Find HTTP Servers in the .local domain using Zeroconf"
	hostType := "stdio"

	mozilla := mozillaManifest{
		Name:              maniFest,
		Description:       hostInfo,
		Path:              filepath.Clean(hostPath),
		Type:              hostType,
		AllowedExtensions: []string{*extension},
	}
	content, err := json.MarshalIndent(mozilla, "", "  ")
	if err != nil {
		log.Fatal(err)
	}
	if os.Getuid() == 0 {
		writeManifest(mozillaRoot, content)
	} else {
		writeManifest(mozillaUser, content)
	}

	chrome := chromeManifest{
		Name:           maniFest,
		Description:    hostInfo,
		Path:           filepath.Clean(hostPath),
		Type:           hostType,
		AllowedOrigins: []string{fmt.Sprintf("chrome-extension://%s/", *origin)},
	}
	content, err = json.MarshalIndent(chrome, "", "  ")
	if err != nil {
		log.Fatal(err)
	}
	if os.Getuid() == 0 {
		writeManifest(chromeRoot,   content)
		writeManifest(chromiumRoot, content)
	} else {
		writeManifest(chromeUser,   content)
		writeManifest(chromiumUser, content)
	}
}

func writeManifest(dirname string, data []byte) {
	dirname = expandHome(dirname)
	if err := os.MkdirAll(dirname, 0755); err != nil {
		log.Fatal(err)
	}

	filename := filepath.Join(dirname, maniFest + ".json")
	if err := ioutil.WriteFile(filename, data, 0644); err != nil {
		log.Fatal(err)
	}

	fmt.Printf("created manifest %s\n", filename)
}

func uninstallManifests() {
	var dirs []string

	if os.Getuid() == 0 {
		dirs = []string{chromeRoot, chromiumRoot, mozillaRoot}
	} else {
		dirs = []string{chromeUser, chromiumUser, mozillaUser}
	}

	for _, dirname := range dirs {
		dirname = expandHome(dirname)
		filename := filepath.Join(dirname, maniFest + ".json")
		if err := os.Remove(filename); err != nil {
			if strings.Contains(err.Error(), "no such") == false {
				log.Fatal(err)
			}
		} else {
			fmt.Printf("removed manifest %s\n", filename)
		}

		if err := os.Remove(dirname); err != nil {
			if strings.Contains(err.Error(), "no such") == false {
				if strings.Contains(err.Error(), "not empty") == false {
					log.Fatal(err)
				}
			}
		} else {
			fmt.Printf("removed directory %s\n", dirname)
		}
	}
}

func expandHome(dirname string) string {
	if strings.HasPrefix(dirname, "~") {
		if homedir := os.Getenv("HOME"); homedir != "" {
			dirname = strings.Replace(dirname, "~", homedir, 1)
		}
	}

	return dirname
}
