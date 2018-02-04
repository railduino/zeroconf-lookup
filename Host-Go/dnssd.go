package main

import (
	"context"
	"fmt"
	"io/ioutil"
	"log"
	"os/exec"
	"regexp"
	"strconv"
	"strings"
	"time"
)

type dnsRecord struct {
	name    string
	target  string
	port    int
	txt     []string
	a       string
}

func collectWithDnssd(path string) (string, error) {
	dnsList := []dnsRecord{}
	var lines []string

	reSrv, err := regexp.Compile(`^(\S+)\s+SRV\s+\d+\s+\d+\s+(\d+)\s+(\S+)\s+;.*$`)
	if err != nil {
		log.Fatal(err)
	}
	reTxt, err := regexp.Compile(`^(\S+)\s+TXT(.*)$`)
	if err != nil {
		log.Fatal(err)
	}

	if *testing {
		if out, err := ioutil.ReadFile("../test-data/output_dns-sd_Z"); err != nil {
			log.Fatal(err)
		} else {
			lines = strings.Split(string(out), "\n")
		}
	} else {
		ctx, cancel := context.WithTimeout(context.Background(), time.Duration(timeout) * time.Second)
		defer cancel()
		cmd := exec.CommandContext(ctx, path, "-Z", "_http._tcp", "local")
		out, _ := cmd.Output()
		if ctx.Err() != nil && ctx.Err() != context.DeadlineExceeded {
			log.Fatal(ctx.Err())
		}
		lines = strings.Split(string(out), "\n")
	}

	for _, line := range lines {
		if fields := reSrv.FindStringSubmatch(line); fields != nil {
			port, err := strconv.Atoi(fields[2])
			if err != nil || port < 1 || port > 65535 {
				// missing or invalid port
				continue
			}
			dnsRec := dnsRecord{ name: fields[1], port: port, target: fields[3] }
			//fmt.Printf("SRV: %+v\n", dnsRec)
			dnsList = append(dnsList, dnsRec)
			continue
		}

		if fields := reTxt.FindStringSubmatch(line); fields != nil {
			if len(fields) < 3 {
				continue
			}
			for ndx, _ := range dnsList {
				dnsRec := &dnsList[ndx]
				if dnsRec.name != fields[1] {
					continue
				}
				txt := strings.TrimSpace(fields[2])
				list := strings.Split(txt, `" "`)
				for _, elem := range list {
					elem = strings.Trim(elem, `"`)
					elem = strings.Replace(elem, "\\032", " ", -1)
					if elem != "" {
						dnsRec.txt = append(dnsRec.txt, elem)
					}
				}
				fmt.Printf("TXT (%s): '%v'\n", dnsRec.name, dnsRec.txt)
			}
		}
	}

	for _, dnsRec := range dnsList {
		if *testing {
			name := "../test-data/output_dns-sd_G-" + dnsRec.target
			if out, err := ioutil.ReadFile(name); err != nil {
				log.Fatal(err)
			} else {
				lines = strings.Split(string(out), "\n")
			}
		} else {
			ctx, cancel := context.WithCancel(context.Background())
			defer cancel()
			cmd := exec.CommandContext(ctx, path, "-G", "v4", dnsRec.name)
			out, _ := cmd.Output()
			lines = strings.Split(string(out), "\n")
		}
		fmt.Printf("dnsRec: %+v\n", dnsRec)
	}

	return "Go (mDNSResponder)", nil
}
