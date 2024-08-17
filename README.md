# Wifi Uploader for Spectrum Next

## What is it?

This is a small .nex network server program that uses the ESP8266 chip in the Spectrum Next to listen to TCP port 1023.
It is meant to be used with a remote controller running on a Desktop.

The controller uploads a binary image into the banks of the Spectrum Next and executes the code.

## Why?

When I've first started to work on this, I had lots of "why" questions.
My goal was simple: when I develop, I don't want to have to play with unreliable SD cards:

* Power-off the Next,
* eject the SD card from the Next,
* insert the SD card to the reader,
* mount the SD card,
* copy the executable,
* safely unmount the SD card,
* eject the SD card fro mthe reader,
* insert the SD card to the Next
* power-on the Next.

No thanks.

Instead all I have to do with this program is:

* Go to the /home directory,
* Execute wifiupld.nex,
* Wait for a couple of seconds,
* Upload my program using the remote,
* Press "Reset" after use.

## But why?

Also, I've read that working with the ESP8266 was a proper PITA, especially as a server, so I had to try.

There are two problems:
* First, the ESP8266 get overwhelmed above a given speed (above about 8KB/s in my experience).
* Second, the ESP8266 has a plethora of firmwares with (not always) subtle differences.

The first problem was easily fixed by adding some flow control in the remote.

The second problem was the fun part. To avoid having to write some complicated assembly code to handle all the cases,
I've opted to write a data-based decoder. A kind of state machine is used to handle the inputs from the ESP8266 and call the right functions or set the right variables.
If a chip is not supported, one only needs to edit the decision structure to add a branch or two and handle its chip variant.

## Protocol

The protocol is extremely simple.

The controller sends messages and the Spectrum Next may answer with something.
Messages are a command on one bytes followed by some parameters.

### Messages

Where [0,1,2,3,4,5,6,7] represents an array of 8 bytes with values 0 to 7.

#### Ping (0)

A message to receive one byte. It was mostly used for debugging.

The contoller sends [0]
The Spectrum Next answers [0]

#### GetBanks (1)

A message to receive the current memory mapping.
It is used to detect conflict with the executable to be uploaded.
It could also be used to move the server program to a bank to avoid conflict with the executable.

The contoller sends [1]
The Spectrum Next answers [Bank0,Bank1,Bank2,Bank3,Bank4,Bank5,Bank6,Bank7] being the memory pages loaded in these banks.

#### Set8KBank (2)

A message to change the mapping of a page into a given bank
Typically the server will use pages 4 and 5 for uploading the binary in the proper banks.

The contoller sends [2,bank,page]

#### WriteAt (3)

A message to write bytes at a given position in memory.
Used to both upload the pages in banks but also to inject code.

The controller sends [3, offset_lo, offset_hi, len_lo, len_hi, ... len bytes ...]

#### CallTo (4)

A message to execute call with the given destination address
Used to bootstrap to the uploaded executable.
The executed instruction is a call as the server expects to resume its operations.
In practice, the code uploaded by the remote never returns.

The controller sends [4, address_lo, address_hi]

## Protocol as used by the remote

The remote fist do **GetBanks**.
Then for each pages ending with 4 and 5,
using **Set8KBank** it sets the banks 4 and 5 with a 16K page and
uploads it using **WriteAt** at the address $8000 (page 4).

The remote ensures to sends the data at a slow enough rate as to not overwhelm the Spectrum Next.
Typically the rate is set to about 8192 bytes per second.

Then the bytes
```
        F3                 di                   ; disable interrupts
        ED 91 50 FF        nextreg $50, $ff     ; restore the ROM
        31 FE FF           ld sp, $fffe         ; setup the stack
        C3 00 80           jp $8000             ; jump to the entry point
```
are uploaded at address $fff0

The stack pointer and entry point may be different depending on the settings of the .nex binary.

Finally, a call is made at address $fff0 using **CallTo**.

## Other uses

The protocol is simple enough and could be used for other remote code injection/execution usages.

# Notes

The code contains a stub to configure the WiFi.
When I found out there was wifi2.bas on the Spectrum Next I've stopped writing the code and diabled the option.
I've not deleted the code as I don't know if I'll finish it or if I'll delete it.

# How to Build

## Important note

This has been written for the 2MB variant of the Spectrum Next.
In main.asm, the bank selected for the server is 223.
If you Spectrum Next has less memory, you'll want to edit the value to a page in your memory range.
An improvement may be the automatic relocation of the server to another page.

SERVER_BANK EQU 223 ; 2MB
;SERVER_BANK EQU 95  ; 1MB

## Steps

The build is made using a basic cmake and **sjasmplus**

```
mkdir build
cd build
cmake .. -DCMAKE_ZXASM_COMPILER="/path/to/sjasmplus"
make
```
