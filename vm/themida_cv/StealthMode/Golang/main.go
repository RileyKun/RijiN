package main

import (
    "fmt"
    "github.com/oreans/virtualizersdk"
)

// -------------------------------------------
// STEALTH AREA definition
// -------------------------------------------
const STEALTH_ONE_KB = 1024 / 4
const STEALTH_ONE_MB = 1024 * STEALTH_ONE_KB

const STEALTH_SIZE   = STEALTH_ONE_MB * 2

var stealth_area = [STEALTH_SIZE]uint32{0xa1a2a3a4, 0xa4a3a2a1, 0xb1a1b2a2, 0xb8a8a1a1, 
                                  0xb6b5b3b6, 0xa2b2c2d2, 0xa9a8a2a2, 0xa0a9b9b8}


func main() {

    virtualizersdk.Macro(virtualizersdk.FISH_WHITE_START)

    fmt.Println("We are inside the FISH macro")

    virtualizersdk.Macro(virtualizersdk.FISH_WHITE_END)

    // Here we put a dummy reference to the stealth Area. So the compiler
    // does not remove it from compilation when not referenced
    if stealth_area[0] == 0x11111111 {
        fmt.Println(stealth_area[0])
    }
}