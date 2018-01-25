package main

import (
	"context"
	"fmt"
	"os/exec"
	"time"
)

func collect_with_mDNSResponder(path string) (string, error) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Duration(timeout) * time.Second)
	defer cancel()

	cmd := exec.CommandContext(ctx, path, "-Z", "_http._tcp", "local")
	out, _ := cmd.Output()
	fmt.Printf("##############\n%s\n##############\n", out)

	if ctx.Err() == context.DeadlineExceeded {
		fmt.Println("Command timed out")
	}

	return "Go (mDNSResponder)", nil
}
