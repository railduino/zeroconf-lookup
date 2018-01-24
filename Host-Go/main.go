package main

import (
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
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"github.com/grandcat/zeroconf"
	"github.com/spf13/viper"
)

const (
	PROG_NAME = "zeroconf_lookup"
	HOST_NAME = "com.railduino.zeroconf_lookup"
	VERSION   = "2.0.0"
	TIMEOUT   = 2
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

var (
	srvList   = []Server{}
	timeout   = TIMEOUT
	verbose   = flag.Bool("v",   false,   "Output diagnostic messages")
	readable  = flag.Bool("r",   false,   "Use human readable i/o size")
	install   = flag.Bool("i",   false,   "Install Mozilla/Chrome manifests (sudo for system wide)")
	uninstall = flag.Bool("u",   false,   "Uninstall Mozilla/Chrome manifests (sudo for system wide)")
	settime   = flag.Int("t",    TIMEOUT, "Setup server collect timeout (with -i)")
	extension = flag.String("m", "zeroconf_lookup@railduino.com",    "Setup Mozilla allowed_extensions (with -i)")
	origin    = flag.String("c", "eajgigammocepkmcgfcicpeljokgcchh", "Setup Chrome allowed_origins (with -i)")
	homedir   = os.Getenv("HOME")
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

	logfile := filepath.Join(os.TempDir(), PROG_NAME + ".log")
	f, err := os.OpenFile(logfile, os.O_WRONLY | os.O_CREATE | os.O_TRUNC, 0666)
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()
	log.SetOutput(f)
	log.SetPrefix(PROG_NAME + ": ")

	viper.SetConfigName(PROG_NAME)
	viper.AddConfigPath("/etc/" + PROG_NAME + "/")
	viper.AddConfigPath("$HOME/." + PROG_NAME)

	viper.SetEnvPrefix(PROG_NAME)
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
		Name:   name,
		Txt:    txt,
		Target: strings.Trim(target, "."),
		Port:   port,
		A:      a,
		Url:    fmt.Sprintf("http://%s:%d/", a, port),
	}
	if server.Port == 3689 {
		server.Txt = append([]string{ "DAAP (iTunes) Server" }, server.Txt...)
	}

	log.Printf("found %s for %s (%v)", server.Url, server.Name, server.Txt)
	srvList = append(srvList, server)
}

func collect_data() (string, error) {
	log.Println("start collecting")

	path, err := exec.LookPath("avahi-browse")
	if err == nil {
		out, _ := exec.Command(path, "-t", "-c", "-r", "-p", "_http._tcp").Output()
		lines := strings.Split(string(out), "\n")
		for _, line := range lines {
			fields := strings.Split(line, ";")
			if len(fields) < 9 || fields[0] != "=" {
				continue
			}
			if strings.ToLower(fields[2]) != "ipv4" {
				continue
			}
			if strings.Contains(fields[7], ":") {
				continue
			}
			port, err := strconv.Atoi(fields[8])
			if err != nil || port < 1 || port > 65535 {
				continue
			}
			add_server(fields[3],
				   strings.TrimSuffix(fields[6], ".local"),
				   fields[7],
				   port,
				   []string{"Hallo", "Welt"})
		}
		return "Go (Avahi)", nil
	}

	resolver, err := zeroconf.NewResolver(nil)
	if err != nil {
		return "", err
	}

	entries := make(chan *zeroconf.ServiceEntry)
	go func(results <-chan *zeroconf.ServiceEntry) {
		for entry := range results {
			if len(entry.AddrIPv4) < 1 {
				continue
			}
			add_server(entry.ServiceRecord.Instance,
				   entry.HostName,
				   entry.AddrIPv4[0].String(),
				   entry.Port,
				   entry.Text)
		}
	}(entries)

	ctx, cancel := context.WithTimeout(context.Background(), time.Duration(timeout) * time.Second)
	defer cancel()
	if err := resolver.Browse(ctx, "_http._tcp", "local.", entries); err != nil {
		return "", err
	}
	<-ctx.Done()

	return "Go (Query)", nil
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
		Name:        HOST_NAME,
		Description: host_info,
		Path:        host_path,
		Type:        host_type,
		AllowedExtensions: []string{
			*extension,
		},
	}
	content, err := json.MarshalIndent(mozilla, "", "  ")
	if err != nil {
		log.Fatal(err)
	}
	if os.Getuid() == 0 {
		write_manifest("/usr/lib/mozilla/native-messaging-hosts", content)
	} else {
		write_manifest(homedir + "/.mozilla/native-messaging-hosts", content)
	}

	////////////////////////// Chrome //////////////////////////
	chrome := ChromeManifest{
		Name:        HOST_NAME,
		Description: host_info,
		Path:        host_path,
		Type:        host_type,
		AllowedOrigins: []string{
			fmt.Sprintf("chrome-extension://%s/", *origin),
		},
	}
	content, err = json.MarshalIndent(chrome, "", "  ")
	if err != nil {
		log.Fatal(err)
	}
	if os.Getuid() == 0 {
		write_manifest("/etc/opt/chrome/native-messaging-hosts", content)
		write_manifest("/etc/chromium/native-messaging-hosts",   content)
	} else {
		write_manifest(homedir + "/.config/google-chrome/NativeMessagingHosts", content)
		write_manifest(homedir + "/.config/chromium/NativeMessagingHosts",      content)
	}

	////////////////////////// Config //////////////////////////
	if *settime != TIMEOUT {
		// TODO write config file
	}

}

func uninstall_manifests() {
	var dirs []string

	if os.Getuid() == 0 {
		dirs = []string{
			"/usr/lib/mozilla/native-messaging-hosts",
			"/etc/opt/chrome/native-messaging-hosts",
			"/etc/chromium/native-messaging-hosts",
		}
	} else {
		dirs = []string{
			homedir + "/.mozilla/native-messaging-hosts",
			homedir + "/.config/google-chrome/NativeMessagingHosts",
			homedir + "/.config/chromium/NativeMessagingHosts",
		}
	}

	for _, dir := range dirs {
		filename := filepath.Join(dir, HOST_NAME + ".json")
		if err := os.Remove(filename); err != nil {
			if strings.Contains(err.Error(), "no such") == false {
				log.Fatal(err)
			}
		} else {
			fmt.Printf("removed manifest %s\n", filename)
		}

		if err := os.Remove(dir); err != nil {
			if strings.Contains(err.Error(), "no such") == false {
				if strings.Contains(err.Error(), "not empty") == false {
					log.Fatal(err)
				}
			}
		} else {
			fmt.Printf("removed directory %s\n", dir)
		}
	}
}

func write_manifest(dir string, data []byte) {
	if err := os.MkdirAll(dir, 0755); err != nil {
		log.Fatal(err)
	}

	filename := filepath.Join(dir, HOST_NAME + ".json")
	if err := ioutil.WriteFile(filename, data, 0644); err != nil {
		log.Fatal(err)
	}

	fmt.Printf("created manifest %s\n", filename)
}
