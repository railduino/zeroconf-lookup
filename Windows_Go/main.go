package main

import (
	"bytes"
	"encoding/binary"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"os/exec"
	"strings"

	"github.com/google/go-cmp/cmp"
)

const (
	prgName  = "zeroconf_lookup"
	prgVers  = "2.3.0"
	apiVers  = 2
	svcType  = "_http._tcp"
	maniFest = "com.railduino.zeroconf_lookup"
	chrAllow = "anjclddigfkhclmgopnjmmpfllfbhfea"
	mozAllow = "zeroconf_lookup@railduino.com"
	dfltTime = 1
)

type command struct {
	Cmd string `json:"cmd"`
}

type server struct {
	Name   string   `json:"name"`
	Txt    []string `json:"txt"`
	Target string   `json:"target"`
	Port   int      `json:"port"`
	A      string   `json:"a"`
	URL    string   `json:"url"`
}

type output struct {
	Version int      `json:"version"`
	Source  string   `json:"source"`
	Result  []server `json:"result"`
}

var (
	servers   = []server{}
	origin    = flag.String("c", chrAllow, "Setup Chrome allowed_origins (with -i)")
	install   = flag.Bool(  "i", false,    "Install Mozilla/Chrome manifests")
	force     = flag.String("f", "",       "Force use of method (dns-sd | zeroconf, with -i | -r)")
	extension = flag.String("m", mozAllow, "Setup Mozilla allowed_extensions (with -i)")
	readable  = flag.Bool(  "r", false,    "Use human readable i/o size")
	testing   = flag.Bool(  "t", false,    "Enable testing mode")
	uninstall = flag.Bool(  "u", false,    "Uninstall Mozilla/Chrome manifests")
	verbose   = flag.Bool(  "v", false,    "Output diagnostic messages")
	timeout   = flag.Int(   "w", dfltTime, "Timeout in seconds (with -i | -r)")
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
		log.Printf("force ...........: %s\n", *force)
		log.Printf("readable ........: %v\n", *readable)
		log.Printf("verbose .........: %v\n", *verbose)
		log.Printf("timeout .........: %d sec\n", *timeout)
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
	source, err := collectData()
	if err != nil {
		log.Fatal(err)
	}
	log.Printf("time is up, used %s\n", source)

	result := output{Version: apiVers, Source: source, Result: servers}
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

func collectData() (string, error) {
	if *force == "zeroconf" {
		return collectZeroconf()
	}

	path, err := exec.LookPath("dns-sd")
	if err == nil || *force == "dns-sd" {
		return collectDnssd(path)
	}

	return collectZeroconf()
}

func addServer(name, target, a string, port int, txt []string) {
	name = strings.Replace(name, `\032`, " ", -1)
	name = strings.Replace(name, `\ `, " ", -1)
	newSrv := server{
		Name:   name,
		Txt:    txt,
		Target: target,
		Port:   port,
		A:      a,
		URL:    fmt.Sprintf("http://%s:%d/", a, port),
	}
	if newSrv.Port == 3689 {
		newSrv.Txt = append([]string{"DAAP (iTunes) Server"}, newSrv.Txt...)
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
