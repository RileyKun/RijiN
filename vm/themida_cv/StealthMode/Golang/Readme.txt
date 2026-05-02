1) In Golang, we define the Stealth Area in the data section like:

var stealth_area = [STEALTH_SIZE]uint32{0xa1a2a3a4, 0xa4a3a2a1, 0xb1a1b2a2, 0xb8a8a1a1, 
                                  0xb6b5b3b6, 0xa2b2c2d2, 0xa9a8a2a2, 0xa0a9b9b8}


2) The "stealth_area" variable must be referenced somewhere in your code to
   avoid the compiler removing its declaration due to not being used. You can 
   just use a conditional which will be evaluated as false 
   Example:

   if stealth_area[0] == 0x11111111 {
        fmt.Println(stealth_area[0])
   }


