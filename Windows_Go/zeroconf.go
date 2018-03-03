package main

import (
	"context"
	"fmt"
	"time"

	"github.com/grandcat/zeroconf"
)

func collectZeroconf() (string, error) {
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

			addServer(entry.ServiceRecord.Instance,
				   entry.HostName,
				   entry.AddrIPv4[0].String(),
				   entry.Port,
				   entry.Text)
		}
	}(entries)

	ctx, cancel := context.WithTimeout(context.Background(), time.Duration(*timeout) * time.Second)
	defer cancel()

	if err := resolver.Browse(ctx, "_http._tcp", "local", entries); err != nil {
		return "", err
	}
	<-ctx.Done()

	return fmt.Sprintf("Zeroconf (Go, %s)", prgVers), nil
}
