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
	VERSION   = "2.0.1"
	TIMEOUT   = 2
	CHROME    = "anjclddigfkhclmgopnjmmpfllfbhfea"
	MOZILLA   = "zeroconf_lookup@railduino.com"
)

type Command struct {
	Cmd string `json:"cmd"`
}

type Server struct {
	Name    string   `json:"name"`
	Txt     []string `json:"txt"`
	Target  string   `json:"target"`
	Port    int      `json:"port"`
	A       string   `json:"a"`
	Url     string   `json:"url"`
}

type Output struct {
	Version int      `json:"version"`
	Source  string   `json:"source"`
	Result  []Server `json:"result"`
}

var (
	my_name   = "zeroconf_lookup"
	srvList   = []Server{}
	timeout   = TIMEOUT
	origin    = flag.String("c", CHROME,  "Setup Chrome allowed_origins (with -i)")
	install   = flag.Bool(  "i", false,   "Install Mozilla/Chrome manifests (sudo for system wide)")
	extension = flag.String("m", MOZILLA, "Setup Mozilla allowed_extensions (with -i)")
	readable  = flag.Bool(  "r", false,   "Use human readable i/o size")
	settime   = flag.Int(   "t", TIMEOUT, "Setup server collect timeout (with -i)")
	uninstall = flag.Bool(  "u", false,   "Uninstall Mozilla/Chrome manifests (sudo for system wide)")
	verbose   = flag.Bool(  "v", false,   "Output diagnostic messages")
)

func main() {
	flag.Parse()

	if *install {
		install_manifests()
		os.Exit(0)
	}

	if *uninstall {
		uninstall_manifests()
		os.Exit(0)
	}

	logfile := filepath.Join(os.TempDir(), my_name + ".log")
	f, err := os.OpenFile(logfile, os.O_WRONLY | os.O_CREATE | os.O_TRUNC, 0666)
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()
	log.SetOutput(f)
	log.SetPrefix(my_name + ": ")

	viper.SetConfigName(my_name)
	viper.AddConfigPath("/etc/" + my_name + "/")
	viper.AddConfigPath("$HOME/." + my_name)

	viper.SetEnvPrefix(my_name)
	viper.AutomaticEnv()

	viper.SetDefault("timeout", TIMEOUT)

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

		log.Printf("VERSION .........: %s", VERSION)
		log.Printf("readable ........: %v", *readable)
		log.Printf("verbose .........: %v", *verbose)
		log.Printf("timeout .........: %d", timeout)
	}

	if *readable == false {
		if err := read_command(os.Stdin); err != nil {
			if err != io.EOF {
				log.Fatal(err)
			}
			log.Println("no input - exit")
		}
	}

	source, err := collect_data()
	if err != nil {
		log.Fatal(err)
	}
	log.Println("time is up")

	output := Output{Version: 2, Source: source, Result: srvList}
	buffer := bytes.Buffer{}
	indent, err := json.MarshalIndent(output, "", "  ")
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

func read_command(input io.Reader) error {
	cnt_buf := make([]byte, 4)
	cnt, err := io.ReadAtLeast(input, cnt_buf, 4)
	if err != nil {
		return err
	}
	if cnt != 4 {
		return errors.New("missing input")
	}

	counter := int(binary.LittleEndian.Uint32(cnt_buf))
	log.Printf("expect %d message bytes", counter)

	msg_buf := make([]byte, counter)
	cnt, err = io.ReadAtLeast(input, msg_buf, counter)
	if err != nil {
		return err
	}
	msg_str := strings.Trim(string(msg_buf), `"`)

	if msg_str == "Lookup" {
		log.Printf("found Lookup (plain)")
		return nil
	}

	msg_str = strings.Replace(msg_str, "\\\"", "\"", -1)
	msg_buf = []byte(msg_str)
	command := Command{}
	if err := json.Unmarshal(msg_buf, &command); err != nil {
		return err
	}
	if command.Cmd == "Lookup" {
		log.Printf("found Lookup (JSON)")
		return nil
	}

	log.Printf("unknown command %+v", command)
	return nil
}

func add_server(name, target, a string, port int, txt []string) {
	server := Server{
		Name:   strings.Replace(name, "\\032", " ", -1),
		Txt:    txt,
		Target: target,
		Port:   port,
		A:      a,
		Url:    fmt.Sprintf("http://%s:%d/", a, port),
	}
	if server.Port == 3689 {
		server.Txt = append([]string{ "DAAP (iTunes) Server" }, server.Txt...)
	}

	log.Printf("found %s for '%s' (%v)", server.Url, server.Name, server.Txt)

	for _, srv := range srvList {
		if cmp.Equal(srv, server) {
			log.Printf("duplicate %s", server.Name)
			return
		}
	}

	srvList = append(srvList, server)
}

func collect_data() (string, error) {
	log.Println("start collecting")

	if path, err := exec.LookPath("avahi-browse"); err == nil {
		return collect_with_avahi(path)
	}

	if path, err := exec.LookPath("dns-sd"); err == nil {
		return collect_with_mDNSResponder(path)
	}

	return collect_with_query()
}
