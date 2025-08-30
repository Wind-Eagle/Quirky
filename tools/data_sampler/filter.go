package main

import "os/exec"

func FilterFens(input string, output string, name string) {
	cmd := exec.Command(name, "-i", input, "-o", output)
	err := cmd.Run()
	if err != nil {
		panic(err)
	}
}
