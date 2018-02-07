package main

import (
	"bufio"
	"bytes"
	"context"
	"encoding/binary"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"

	"github.com/google/go-cmp/cmp"
)

const (
	prgName  = "zeroconf_lookup"
	prgVers  = "2.2.0"
	apiVers  = 2
	srcInfo  = "Go-Cmd (mDNSResponder)"
	svcType  = "_http._tcp"
	maniFest = "com.railduino.zeroconf_lookup"
	chrAllow = "anjclddigfkhclmgopnjmmpfllfbhfea"
	mozAllow = "zeroconf_lookup@railduino.com"
	dfltWait = 1
)

type record struct {
	instance string
	name     string
	target   string
	port     int
	txt      []string
}

type command struct {
	Cmd      string   `json:"cmd"`
}

type server struct {
	Name     string   `json:"name"`
	Txt      []string `json:"txt"`
	Target   string   `json:"target"`
	Port     int      `json:"port"`
	A        string   `json:"a"`
	URL      string   `json:"url"`
}

type output struct {
	Version  int      `json:"version"`
	Source   string   `json:"source"`
	Result   []server `json:"result"`
}

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

var (
	servers   = []server{}
	timeout   = dfltWait
	origin    = flag.String("c", chrAllow, "Setup Chrome allowed_origins (with -i)")
	install   = flag.Bool(  "i", false,    "Install Mozilla/Chrome manifests (sudo for system wide)")
	extension = flag.String("m", mozAllow, "Setup Mozilla allowed_extensions (with -i)")
	readable  = flag.Bool(  "r", false,    "Use human readable i/o size")
	testing   = flag.Bool(  "t", false,    "Enable testing mode")
	uninstall = flag.Bool(  "u", false,    "Uninstall Mozilla/Chrome manifests (sudo for system wide)")
	verbose   = flag.Bool(  "v", false,    "Output diagnostic messages")
	waiting   = flag.Int(   "w", dfltWait, "Setup server collect timeout (with -i / -r)")
)

func main() {
	flag.Parse()

	if *install {
		installManifests()
		os.Exit(0)
	}

	if *uninstall {
		uninstallManifests()
		os.Exit(0)
	}

	if *testing {
		*readable = true
	}

	log.SetPrefix(prgName + ": ")

	if *verbose {
		log.Printf("Prog Version ....: %s\n", prgVers)
		log.Printf("API Version .....: %d\n", apiVers)
		log.Printf("readable ........: %v\n", *readable)
		log.Printf("verbose .........: %v\n", *verbose)
		log.Printf("timeout .........: %d\n", timeout)
		log.Printf("testing .........: %v\n", *testing)
	}

	if *readable == false {
		if err := readCommand(os.Stdin); err != nil {
			if err != io.EOF {
				log.Fatal(err)
			}
			log.Println("no input - exit")
		}
	}

	log.Println("start collecting")
	collectData()
	log.Println("time is up")

	result := output{Version: apiVers, Source: srcInfo, Result: servers}
	buffer := bytes.Buffer{}
	indent, err := json.MarshalIndent(result, "", "  ")
	if err != nil {
		log.Fatal(err)
	}
	if _, err := buffer.Write(indent); err != nil {
		log.Fatal(err)
	}

	if *readable {
		text := buffer.String()
		fmt.Printf("==> %d bytes <==\n%s\n", len(text), text)
	} else {
		count := uint32(buffer.Len())
		if err := binary.Write(os.Stdout, binary.LittleEndian, count); err != nil {
			log.Fatal(err)
		}
		if _, err := buffer.WriteTo(os.Stdout); err != nil {
			log.Fatal(err)
		}
	}

	os.Exit(0)
}

func readCommand(input io.Reader) error {
	cntBuf := make([]byte, 4)
	if cnt, err := io.ReadAtLeast(input, cntBuf, 4); err != nil {
		return err
	} else if cnt != 4 {
		return errors.New("missing count input")
	}
	counter := int(binary.LittleEndian.Uint32(cntBuf))
	log.Printf("expect %d message bytes\n", counter)

	msgBuf := make([]byte, counter)
	if cnt, err := io.ReadAtLeast(input, msgBuf, counter); err != nil {
		return err
	} else if cnt != counter {
		return errors.New("missing data input")
	}
	msgStr := strings.Trim(string(msgBuf), `"`)

	if msgStr == "Lookup" {
		log.Println("found Lookup (plain)")
		return nil
	}

	msgStr = strings.Replace(msgStr, "\\\"", "\"", -1)
	msgBuf = []byte(msgStr)
	msgCmd := command{}
	if err := json.Unmarshal(msgBuf, &msgCmd); err != nil {
		return err
	}
	if msgCmd.Cmd == "Lookup" {
		log.Println("found Lookup (JSON)")
		return nil
	}

	log.Printf("unknown command %+v\n", msgCmd)
	return nil
}

func collectData() {
	path, err := exec.LookPath("dns-sd")
	if *testing == false && err != nil {
		log.Fatal(err)
	}
	dnsList := []record{}

	for _, line := range callDnsSdZ(path) {
		line = strings.Replace(line, `\032`, "#@@#", -1)
		line = strings.Replace(line, `\ `,   "#@@#", -1)
		line = strings.Replace(line, `"`,    "'",    -1)
		fields := strings.Fields(line)

		if len(fields) >= 6 && fields[1] == "SRV" {
			port, err := strconv.Atoi(fields[4])
			if err != nil || port < 1 || port > 65535 {
				// missing or invalid port
				continue
			}

			instance := fields[0]
			name := strings.TrimSuffix(instance, "." + svcType)
			name =  strings.Replace(name, "#@@#", " ", -1)

			dnsRec := record{
				instance: instance,
				name:     name,
				port:     port,
				target:   fields[5],
				txt:      []string{},
			}
			dnsList = append(dnsList, dnsRec)
			continue
		}

		if len(fields) >= 3 && fields[1] == "TXT" {
			for ndx, _ := range dnsList {
				dnsRec := &dnsList[ndx]
				if dnsRec.instance != fields[0] {
					continue
				}

				txt := strings.Join(fields[2:], " ")
				list := strings.Split(txt, "' '")
				for _, elem := range list {
					elem = strings.Trim(elem, "'")
					elem = strings.Replace(elem, `\032`, " ", -1)
					elem = strings.Replace(elem, `\ `,   " ", -1)
					if elem != "" {
						dnsRec.txt = append(dnsRec.txt, elem)
					}
				}
			}
			continue
		}
	}

	for _, dnsRec := range dnsList {
		line := callDnsSdG(path, dnsRec.target)

		fields := strings.Fields(line)
		if len(fields) < 6 {
			log.Println("invalid address line for " + dnsRec.target)
			continue
		}

		v4addr := fields[5]
		addServer(dnsRec.name, dnsRec.target, v4addr, dnsRec.port, dnsRec.txt)
	}
}

func callDnsSdZ(path string) []string {
	if *testing {
		if out, err := ioutil.ReadFile("../test-data/output_dns-sd_Z"); err != nil {
			log.Fatal(err)
		} else {
			return strings.Split(string(out), "\n")
		}
	}

	ctx, cancel := context.WithTimeout(context.Background(), time.Duration(timeout) * time.Second)
	defer cancel()

	cmd := exec.CommandContext(ctx, path, "-Z", svcType, "local")
	out, _ := cmd.Output()

	if ctx.Err() != nil && ctx.Err() != context.DeadlineExceeded {
		log.Fatal(ctx.Err())
	}

	return strings.Split(string(out), "\n")
}

func callDnsSdG(path, target string) string {
	if *testing {
		name := "../test-data/output_dns-sd_G-" + target
		if out, err := ioutil.ReadFile(name); err != nil {
			log.Fatal(err)
		} else {
			for _, line := range strings.Split(string(out), "\n") {
				if strings.Contains(line, target) {
					return line
				}
			}
		}
		return ""
	}

	ctx, cancel := context.WithTimeout(context.Background(), 1 * time.Second)
	defer cancel()

	cmd := exec.CommandContext(ctx, path, "-G", "v4", target)
	out, err := cmd.StdoutPipe()
	if err != nil {
		log.Fatal(err)
	}
	if err := cmd.Start(); err != nil {
		log.Fatal(err)
	}

	scanner := bufio.NewScanner(out)
	for scanner.Scan() {
		line := scanner.Text()
		if strings.Contains(line, target) {
			return line	// calls cancel()
		}
	}

	return ""
}

func addServer(name, target, a string, port int, txt []string) {
	name = strings.Replace(name, `\032`, " ", -1)
	name = strings.Replace(name, `\ `,   " ", -1)
	newSrv := server{
		Name:   name,
		Txt:    txt,
		Target: target,
		Port:   port,
		A:      a,
		URL:    fmt.Sprintf("http://%s:%d/", a, port),
	}
	if newSrv.Port == 3689 {
		newSrv.Txt = append([]string{ "DAAP (iTunes) Server" }, newSrv.Txt...)
	}

	for _, srv := range servers {
		if cmp.Equal(srv, newSrv) {
			log.Printf("duplicate server %s\n", newSrv.Name)
			return
		}
	}

	log.Printf("found %s for '%s' (%v)\n", newSrv.URL, newSrv.Name, newSrv.Txt)
	servers = append(servers, newSrv)
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
		Name:        maniFest,
		Description: hostInfo,
		Path:        hostPath,
		Type:        hostType,
		AllowedExtensions: []string{ *extension },
	}
	content, err := json.MarshalIndent(mozilla, "", "  ")
	if err != nil {
		log.Fatal(err)
	}
	log.Println(string(content))

	//k, err := registry.OpenKey(registry.CURRENT_USER, `SOFTWARE\Mozilla\NativeMessagingHosts\` + maniFest, registry.SET_VALUE)
	//if err != nil {
	//	log.Fatal(err)
	//}
	//defer k.Close()

	////////////////////////// Chrome //////////////////////////
	chrome := chromeManifest{
		Name:        maniFest,
		Description: hostInfo,
		Path:        hostPath,
		Type:        hostType,
		AllowedOrigins: []string{ fmt.Sprintf("chrome-extension://%s/", *origin) },
	}
	content, err = json.MarshalIndent(chrome, "", "  ")
	if err != nil {
		log.Fatal(err)
	}
	log.Println(string(content))
}

func uninstallManifests() {
}

