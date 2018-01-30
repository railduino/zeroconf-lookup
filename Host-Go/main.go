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
	"path/filepath"
	"strings"

	"github.com/google/go-cmp/cmp"
	"github.com/spf13/viper"
)

const (
	version = "2.0.1"
	chrome  = "anjclddigfkhclmgopnjmmpfllfbhfea"
	mozilla = "zeroconf_lookup@railduino.com"
)

type command struct {
	Cmd string `json:"cmd"`
}

type server struct {
	Name    string   `json:"name"`
	Txt     []string `json:"txt"`
	Target  string   `json:"target"`
	Port    int      `json:"port"`
	A       string   `json:"a"`
	URL     string   `json:"url"`
}

type output struct {
	Version int      `json:"version"`
	Source  string   `json:"source"`
	Result  []server `json:"result"`
}

var (
	myName    = "zeroconf_lookup"
	srvList   = []server{}
	timeout   = 2
	origin    = flag.String("c", chrome,  "Setup Chrome allowed_origins (with -i)")
	install   = flag.Bool(  "i", false,   "Install Mozilla/Chrome manifests (sudo for system wide)")
	extension = flag.String("m", mozilla, "Setup Mozilla allowed_extensions (with -i)")
	readable  = flag.Bool(  "r", false,   "Use human readable i/o size")
	settime   = flag.Int(   "t", 2,       "Setup server collect timeout (with -i)")
	uninstall = flag.Bool(  "u", false,   "Uninstall Mozilla/Chrome manifests (sudo for system wide)")
	verbose   = flag.Bool(  "v", false,   "Output diagnostic messages")
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

	logfile := filepath.Join(os.TempDir(), myName + ".log")
	f, err := os.OpenFile(logfile, os.O_WRONLY | os.O_CREATE | os.O_TRUNC, 0666)
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()
	log.SetOutput(f)
	log.SetPrefix(myName + ": ")

	viper.SetConfigName(myName)
	viper.AddConfigPath("/etc/" + myName + "/")
	viper.AddConfigPath("$HOME/." + myName)

	viper.SetEnvPrefix(myName)
	viper.AutomaticEnv()

	viper.SetDefault("timeout", 2)

	if err := viper.ReadInConfig(); err != nil {
		if strings.Contains(err.Error(), "Not Found") == false {
			log.Printf("FATAL ReadInConfig: %s", err)
			os.Exit(1)
		}
		// No config file: use defaults
	}
	timeout = viper.GetInt("timeout")

	if *verbose {
		fmt.Printf("Logfile is: %s\n", logfile)

		log.Printf("version .........: %s", version)
		log.Printf("readable ........: %v", *readable)
		log.Printf("verbose .........: %v", *verbose)
		log.Printf("timeout .........: %d", timeout)
	}

	if *readable == false {
		if err := readCommand(os.Stdin); err != nil {
			if err != io.EOF {
				log.Fatal(err)
			}
			log.Println("no input - exit")
		}
	}

	source, err := collectData()
	if err != nil {
		log.Fatal(err)
	}
	log.Println("time is up")

	result := output{Version: 2, Source: source, Result: srvList}
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
	log.Printf("expect %d message bytes", counter)

	msgBuf := make([]byte, counter)
	if cnt, err := io.ReadAtLeast(input, msgBuf, counter); err != nil {
		return err
	} else if cnt != counter {
		return errors.New("missing data input")
	}
	msgStr := strings.Trim(string(msgBuf), `"`)

	if msgStr == "Lookup" {
		log.Printf("found Lookup (plain)")
		return nil
	}

	msgStr = strings.Replace(msgStr, "\\\"", "\"", -1)
	msgBuf = []byte(msgStr)
	msgCmd := command{}
	if err := json.Unmarshal(msgBuf, &msgCmd); err != nil {
		return err
	}
	if msgCmd.Cmd == "Lookup" {
		log.Printf("found Lookup (JSON)")
		return nil
	}

	log.Printf("unknown command %+v", msgCmd)
	return nil
}

func addServer(name, target, a string, port int, txt []string) {
	newSrv := server{
		Name:   strings.Replace(name, "\\032", " ", -1),
		Txt:    txt,
		Target: target,
		Port:   port,
		A:      a,
		URL:    fmt.Sprintf("http://%s:%d/", a, port),
	}
	if newSrv.Port == 3689 {
		newSrv.Txt = append([]string{ "DAAP (iTunes) Server" }, newSrv.Txt...)
	}

	for _, srv := range srvList {
		if cmp.Equal(srv, newSrv) {
			log.Printf("duplicate %s", newSrv.Name)
			return
		}
	}

	log.Printf("found %s for '%s' (%v)", newSrv.URL, newSrv.Name, newSrv.Txt)
	srvList = append(srvList, newSrv)
}

func collectData() (string, error) {
	log.Println("start collecting")

	if path, err := exec.LookPath("avahi-browse"); err == nil {
		return collectWithAvahi(path)
	}

	if path, err := exec.LookPath("dns-sd"); err == nil {
		return collectWithDnssd(path)
	}

	return collectWithQuery()
}
