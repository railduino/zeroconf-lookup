package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"

	"golang.org/x/sys/windows/registry"
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
	writeRegistry(hostPath, "mozilla", "Mozilla", content)

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
	writeRegistry(hostPath, "chrome", "Google\\Chrome", content)
}

func writeRegistry(executable, basename, browser string, content []byte) {
	filename := filepath.Join(filepath.Dir(executable), basename + ".json")
	if err := ioutil.WriteFile(filename, content, 0644); err != nil {
		log.Fatal(err)
	}

	regpath := fmt.Sprintf("SOFTWARE\\%s\\NativeMessagingHosts\\%s", browser, maniFest)
	key, found, err := registry.CreateKey(registry.CURRENT_USER, regpath, registry.SET_VALUE)
	if err != nil {
		log.Fatal(err)
	}
	defer key.Close()

	if err := key.SetStringValue("", filename); err != nil {
		log.Fatal(err)
	}

	log.Printf("done registry for %s (%v) = %s", regpath, found, filename)
	log.Println(string(content))
}

func uninstallManifests() {
	// TODO
}
