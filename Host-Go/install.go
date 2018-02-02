package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"runtime"
	"strings"
)

const (
	manifestName      = "com.railduino.zeroconf_lookup"
	appleChromeRoot   = "/Library/Google/Chrome/NativeMessagingHosts"
	appleChromeUser   = "~/Library/Application Support/Google/Chrome/NativeMessagingHosts"
	appleChromiumRoot = "/Library/Application Support/Chromium/NativeMessagingHosts"
	appleChromiumUser = "~/Library/Application Support/Chromium/NativeMessagingHosts"
	appleMozillaRoot  = "/Library/Application Support/Mozilla/NativeMessagingHosts"
	appleMozillaUser  = "~/Library/Application Support/Mozilla/NativeMessagingHosts"
	linuxChromeRoot   = "/etc/opt/chrome/native-messaging-hosts"
	linuxChromeUser   = "~/.config/google-chrome/NativeMessagingHosts"
	linuxChromiumRoot = "/etc/chromium/native-messaging-hosts"
	linuxChromiumUser = "~/.config/chromium/NativeMessagingHosts"
	linuxMozillaRoot  = "/usr/lib/mozilla/native-messaging-hosts"
	linuxMozillaUser  = "~/.mozilla/native-messaging-hosts"
)

type mozillaManifest struct {
	Name        string `json:"name"`
	Description string `json:"description"`
	Path        string `json:"path"`
	Type        string `json:"type"`
	AllowedExtensions []string `json:"allowed_extensions"`
}

type chromeManifest struct {
	Name        string `json:"name"`
	Description string `json:"description"`
	Path        string `json:"path"`
	Type        string `json:"type"`
	AllowedOrigins []string `json:"allowed_origins"`
}

func installManifests() {
	hostPath, err := os.Executable()
	if err != nil {
		log.Fatal(err)
	}
	hostInfo := "Find HTTP Servers in the .local domain using Zeroconf"
	hostType := "stdio"

	////////////////////////// Mozilla //////////////////////////
	mozilla := mozillaManifest{
		Name:        manifestName,
		Description: hostInfo,
		Path:        hostPath,
		Type:        hostType,
		AllowedExtensions: []string{ *extension },
	}
	content, err := json.MarshalIndent(mozilla, "", "  ")
	if err != nil {
		log.Fatal(err)
	}

	if runtime.GOOS == "linux" {
		if os.Getuid() == 0 {
			writeManifest(linuxMozillaRoot, content)
		} else {
			writeManifest(linuxMozillaUser, content)
		}
	} else if runtime.GOOS == "darwin" {
		if os.Getuid() == 0 {
			writeManifest(appleMozillaRoot, content)
		} else {
			writeManifest(appleMozillaUser, content)
		}
	} else {
		log.Fatal("sorry -- OS not yet implemented: " + runtime.GOOS)
	}

	////////////////////////// Chrome //////////////////////////
	chrome := chromeManifest{
		Name:        manifestName,
		Description: hostInfo,
		Path:        hostPath,
		Type:        hostType,
		AllowedOrigins: []string{ fmt.Sprintf("chrome-extension://%s/", *origin) },
	}
	content, err = json.MarshalIndent(chrome, "", "  ")
	if err != nil {
		log.Fatal(err)
	}

	if runtime.GOOS == "linux" {
		if os.Getuid() == 0 {
			writeManifest(linuxChromeRoot,   content)
			writeManifest(linuxChromiumRoot, content)
		} else {
			writeManifest(linuxChromeUser,   content)
			writeManifest(linuxChromiumUser, content)
		}
	} else if runtime.GOOS == "darwin" {
		if os.Getuid() == 0 {
			writeManifest(appleChromeRoot,   content)
			writeManifest(appleChromiumRoot, content)
		} else {
			writeManifest(appleChromeUser,   content)
			writeManifest(appleChromiumUser, content)
		}
	} else {
		log.Fatal("sorry -- OS not yet implemented: " + runtime.GOOS)
	}

	////////////////////////// Config //////////////////////////
	if *waiting != 2 {
		// TODO write config file
	}

}

func writeManifest(dirName string, data []byte) {
	homeDir := os.Getenv("HOME")
	if strings.HasPrefix(dirName, "~") && homeDir != "" {
		dirName = strings.Replace(dirName, "~", homeDir, 1)
	}

	if err := os.MkdirAll(dirName, 0755); err != nil {
		log.Fatal(err)
	}

	fileName := filepath.Join(dirName, manifestName + ".json")
	if err := ioutil.WriteFile(fileName, data, 0644); err != nil {
		log.Fatal(err)
	}

	fmt.Printf("created manifest %s\n", fileName)
}

func uninstallManifests() {
	var dirs []string

	if runtime.GOOS == "linux" {
		if os.Getuid() == 0 {
			dirs = []string{ linuxChromeRoot, linuxChromiumRoot, linuxMozillaRoot }
		} else {
			dirs = []string{ linuxChromeUser, linuxChromiumUser, linuxMozillaUser }
		}
	} else if runtime.GOOS == "darwin" {
		if os.Getuid() == 0 {
			dirs = []string{ appleChromeRoot, appleChromiumRoot, appleMozillaRoot }
		} else {
			dirs = []string{ appleChromeUser, appleChromiumUser, appleMozillaUser }
		}
	} else {
		log.Fatal("sorry -- OS not yet implemented: " + runtime.GOOS)
	}

	for _, dirName := range dirs {
		homeDir := os.Getenv("HOME")
		if strings.HasPrefix(dirName, "~") && homeDir != "" {
			dirName = strings.Replace(dirName, "~", homeDir, 1)
		}

		fileName := filepath.Join(dirName, manifestName + ".json")
		if err := os.Remove(fileName); err != nil {
			if strings.Contains(err.Error(), "no such") == false {
				log.Fatal(err)
			}
		} else {
			fmt.Printf("removed manifest %s\n", fileName)
		}

		if err := os.Remove(dirName); err != nil {
			if strings.Contains(err.Error(), "no such") == false {
				if strings.Contains(err.Error(), "not empty") == false {
					log.Fatal(err)
				}
			}
		} else {
			fmt.Printf("removed directory %s\n", dirName)
		}
	}
}
