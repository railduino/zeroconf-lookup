package main

import (
	"bufio"
	"context"
	"io/ioutil"
	"log"
	"os/exec"
	"strconv"
	"strings"
	"time"
)

type dnsRecord struct {
	instance string
	name     string
	target   string
	port     int
	txt      []string
}

func collectWithDnssd(path string) (string, error) {
	dnsList := []dnsRecord{}
	var lines []string

	if *testing {
		if out, err := ioutil.ReadFile("../test-data/output_dns-sd_Z"); err != nil {
			log.Fatal(err)
		} else {
			lines = strings.Split(string(out), "\n")
		}
	} else {
		ctx, cancel := context.WithTimeout(context.Background(), time.Duration(timeout) * time.Second)
		defer cancel()
		cmd := exec.CommandContext(ctx, path, "-Z", svcType, "local")
		out, _ := cmd.Output()
		cancel()
		if ctx.Err() != nil && ctx.Err() != context.DeadlineExceeded {
			log.Fatal(ctx.Err())
		}
		lines = strings.Split(string(out), "\n")
	}

	for _, line := range lines {
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
			dnsRec := dnsRecord{
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
		}
	}

	for _, dnsRec := range dnsList {
		line := dnssdGetAddrLine(path, dnsRec.target)
		fields := strings.Fields(line)
		if len(fields) < 6 {
			log.Println("invalid address line for " + dnsRec.target)
			continue
		}

		v4addr := fields[5]
		addServer(dnsRec.name, dnsRec.target, v4addr, dnsRec.port, dnsRec.txt)
	}

	return "Go (mDNSResponder)", nil
}

func dnssdGetAddrLine(path, target string) string {
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
			return line	// does cancel()
		}
	}

	return ""
}

