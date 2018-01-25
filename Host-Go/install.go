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
	MANIFEST_NAME       = "com.railduino.zeroconf_lookup"
	APPLE_CHROME_ROOT   = "/Library/Google/Chrome/NativeMessagingHosts"
	APPLE_CHROME_USER   = "~/Library/Application Support/Google/Chrome/NativeMessagingHosts"
	APPLE_CHROMIUM_ROOT = "/Library/Application Support/Chromium/NativeMessagingHosts"
	APPLE_CHROMIUM_USER = "~/Library/Application Support/Chromium/NativeMessagingHosts"
	APPLE_MOZILLA_ROOT  = "/Library/Application Support/Mozilla/NativeMessagingHosts"
	APPLE_MOZILLA_USER  = "~/Library/Application Support/Mozilla/NativeMessagingHosts"
	LINUX_CHROME_ROOT   = "/etc/opt/chrome/native-messaging-hosts"
	LINUX_CHROME_USER   = "~/.config/google-chrome/NativeMessagingHosts"
	LINUX_CHROMIUM_ROOT = "/etc/chromium/native-messaging-hosts"
	LINUX_CHROMIUM_USER = "~/.config/chromium/NativeMessagingHosts"
	LINUX_MOZILLA_ROOT  = "/usr/lib/mozilla/native-messaging-hosts"
	LINUX_MOZILLA_USER  = "~/.mozilla/native-messaging-hosts"
)

type MozillaManifest struct {
	Name        string `json:"name"`
	Description string `json:"description"`
	Path        string `json:"path"`
	Type        string `json:"type"`
	AllowedExtensions []string `json:"allowed_extensions"`
}

type ChromeManifest struct {
	Name        string `json:"name"`
	Description string `json:"description"`
	Path        string `json:"path"`
	Type        string `json:"type"`
	AllowedOrigins []string `json:"allowed_origins"`
}

func install_manifests() {
	host_path, err := os.Executable()
	if err != nil {
		log.Fatal(err)
	}
	host_info := "Find HTTP Servers in the .local domain using Zeroconf"
	host_type := "stdio"

	////////////////////////// Mozilla //////////////////////////
	mozilla := MozillaManifest{
		Name:        MANIFEST_NAME,
		Description: host_info,
		Path:        host_path,
		Type:        host_type,
		AllowedExtensions: []string{ *extension },
	}
	content, err := json.MarshalIndent(mozilla, "", "  ")
	if err != nil {
		log.Fatal(err)
	}

	if runtime.GOOS == "linux" {
		if os.Getuid() == 0 {
			write_manifest(LINUX_MOZILLA_ROOT, content)
		} else {
			write_manifest(LINUX_MOZILLA_USER, content)
		}
	} else if runtime.GOOS == "darwin" {
		if os.Getuid() == 0 {
			write_manifest(APPLE_MOZILLA_ROOT, content)
		} else {
			write_manifest(APPLE_MOZILLA_USER, content)
		}
	} else {
		log.Fatal("sorry -- OS %s not yet implemented", runtime.GOOS)
	}

	////////////////////////// Chrome //////////////////////////
	chrome := ChromeManifest{
		Name:        MANIFEST_NAME,
		Description: host_info,
		Path:        host_path,
		Type:        host_type,
		AllowedOrigins: []string{ fmt.Sprintf("chrome-extension://%s/", *origin) },
	}
	content, err = json.MarshalIndent(chrome, "", "  ")
	if err != nil {
		log.Fatal(err)
	}

	if runtime.GOOS == "linux" {
		if os.Getuid() == 0 {
			write_manifest(LINUX_CHROME_ROOT,   content)
			write_manifest(LINUX_CHROMIUM_ROOT, content)
		} else {
			write_manifest(LINUX_CHROME_USER,   content)
			write_manifest(LINUX_CHROMIUM_USER, content)
		}
	} else if runtime.GOOS == "darwin" {
		if os.Getuid() == 0 {
			write_manifest(APPLE_CHROME_ROOT,   content)
			write_manifest(APPLE_CHROMIUM_ROOT, content)
		} else {
			write_manifest(APPLE_CHROME_USER,   content)
			write_manifest(APPLE_CHROMIUM_USER, content)
		}
	} else {
		log.Fatal("sorry -- OS %s not yet implemented", runtime.GOOS)
	}

	////////////////////////// Config //////////////////////////
	if *settime != TIMEOUT {
		// TODO write config file
	}

}

func write_manifest(dirname string, data []byte) {
	homedir := os.Getenv("HOME")
	if strings.HasPrefix(dirname, "~") && homedir != "" {
		dirname = strings.Replace(dirname, "~", homedir, 1)
	}

	if err := os.MkdirAll(dirname, 0755); err != nil {
		log.Fatal(err)
	}

	filename := filepath.Join(dirname, MANIFEST_NAME + ".json")
	if err := ioutil.WriteFile(filename, data, 0644); err != nil {
		log.Fatal(err)
	}

	fmt.Printf("created manifest %s\n", filename)
}

func uninstall_manifests() {
	var dirs []string

	if runtime.GOOS == "linux" {
		if os.Getuid() == 0 {
			dirs = []string{ LINUX_CHROME_ROOT, LINUX_CHROMIUM_ROOT, LINUX_MOZILLA_ROOT }
		} else {
			dirs = []string{ LINUX_CHROME_USER, LINUX_CHROMIUM_USER, LINUX_MOZILLA_USER }
		}
	} else if runtime.GOOS == "darwin" {
		if os.Getuid() == 0 {
			dirs = []string{ APPLE_CHROME_ROOT, APPLE_CHROMIUM_ROOT, APPLE_MOZILLA_ROOT }
		} else {
			dirs = []string{ APPLE_CHROME_USER, APPLE_CHROMIUM_USER, APPLE_MOZILLA_USER }
		}
	} else {
		log.Fatal("sorry -- OS %s not yet implemented", runtime.GOOS)
	}

	for _, dirname := range dirs {
		homedir := os.Getenv("HOME")
		if strings.HasPrefix(dirname, "~") && homedir != "" {
			dirname = strings.Replace(dirname, "~", homedir, 1)
		}

		filename := filepath.Join(dirname, MANIFEST_NAME + ".json")
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
