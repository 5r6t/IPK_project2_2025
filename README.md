# IPK Project 2 – Chat Client (`ipk25chat-client`)

## Assignment and Repository
- [Assignment Link](https://git.fit.vutbr.cz/NESFIT/IPK-Projects/src/branch/master/Project_2/README.md)
- [Project Repository Link](https://git.fit.vutbr.cz/xmervaj00/IPK_project_dos)

## 1. Description
This is an CLI client for communicating with a predefined server using either TCP or UDP protocol. 

## 2. Compilation and Usage
### 2.1. Compilation
- Build with command `make` or `make debug` for version with debugging prints
- Libraries used should be available on most machines by default:
```cpp
#include <optional>
#include <sys/time.h>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <iostream>
#include <stdexcept>
#include <regex>
#include <vector>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
```
### 2.2. Usage
```bash
./ipk25chat-client [-t protocol] [-s hostname] [-p port] 
                   [-d udp confirmation timeout] 
                   [-r udp retransmissions] 
```

**Arguments**:
- `-t` - must be provided, either `udp` or `tcp`
- `-s` - must be provided, either IP address or hostname
- `-p` - default port is 4567, unless provided
- `-d` - default UDP confirmation timeout is 250ms, unless provided
- `-r` - default number of UDP retransmissions 3, unless provided
- `-h` - prints help and exits

**Examples**:
  ```
  ./ipk25chat-client -h
  ./ipk25chat-client -t udp -s hostname
  ./ipk25chat-client -t tcp -s hostname -p 4567 -d 250 -r 3
  ```

## 3. Features
### 3.1. Supported Commands
- `/auth <username> <secret> <displayname>`
- `/join <channel>`
- `/rename <displayname>`
- `/help`

### 3.2. Supported Message Types
- MSG
- REPLY OK / NOK
- ERR
- BYE
- PING - UDP, only receiving
- CONFIRM - UDP

### 3.3. Error codes
- `ERR_MISSING (10)`: Required argument or data is missing.
- `ERR_INVALID (11)`: Provided input or format is invalid.
- `ERR_TIMEOUT (12)`: Operation timed out (e.g., no response within limit).
- `ERR_CONNECTION (13)`: Failed to establish a connection (e.g., `connect()` failed).
- `ERR_SERVER (14)`: Server closed the connection or responded with an error.
- `ERR_INTERNAL (99)`: Internal program error (e.g., failed to create socket).

### 3.3. Notes
- Argument order is flexible and code has been copied from the first Project 1 - OMEGA: L4 Scanner [(1)](#sources)
- Program can be interrupted at any time using `Ctrl+C/Ctrl+D` (graceful shutdown -- does not immediately kill the program).
- Errors relevant to the user are printed to standard input.
- Format of messages, ChannelID and the rest is enforced through regular expressions and easily modifiable. Located inside `Tools`.

## 4. Implementation Details
### 4.1. Architecture
```lisp
                     [ ipk25chat-client ]
                              ║ 
        ╔═══ (init+data) ═════╩══════ (init) ═════════╗
        ║                                             ║ 
        ║                                             ║
  [ Client Init ] ═════════ (data) ═════════> [ Client Session ]
        ║                                             ║  ║
        ║                                             ║  ║
        ╚══ (uses) ════> [ Toolkit ] <════ (uses) ════╝  ║
                                                         ║
        [ Client Comms ] <══════ (init+data+uses) ═══════╝
```
- `ipk25chat-client.cpp` creates instances `Client_Init` and `Client_Session`, feeds data from CLI to `Client_Init` and calls main `run` loop from `Client_Session`.
- `Client_Init` converts arguments received in string format to appropriate formats, ensuring their correctness. Prints help and exits if given `-h` argument. Uses static functions from `Toolkit` class.
- `Client_Session` uses data from `Client_Init` and static functions from `Toolkit`. It creates an instance of `Client_Comms` in order to separate data handling from the networking aspect. It uses state logic to ensure correctness of actions executed.
- `Client_Comms` receives data from `Client_Session`. It contains functions to resolve hostname, send and receive messages from UDP/TCP protocol and closing connections.
- `Toolkit` contains various functions to abstract from building UDP messages, checking type sizes and regular expressions. It aims to be readable and easily modifiable, containing seemingly redundant functions like `append_uint8()`.

### 4.2. Message Sending and Receiving
- Sending and receiving in real time is handled by using `select()`[(7-11)](#sources). While `poll()` is better [(9)](#sources) than select by allowing larger descriptors, in our case, `select()` is enough.

**TCP behavior**:

Because TCP is a byte stream, received messages are stored in a buffer [(6)](#sources), to handle them in order without dropping anything. Timeout of 5 seconds gives server enough time to respond. If no response is received, program gracefully terminates the connection and exits (meaning it sends ERR/BYE to the server and ends connection without any RST flags).

**UDP behavior**: 

UDP is unreliable, and therefore, it is important to handle its flaws on an application level. Each message sent is expected to receive a confirmation message, and likewise, each received message to be sent a confirmation.

### 4.3. Packet Parsing
While using TCP protocol, `tcp_buffer` is checked for delimiters, in order to extract complete messages. If a complete message is found, it is sent to `parse_tcp_message(msg)`, where either a match occurs, and a filled `ParseMessage` structure is returned, or no match results in nothing being returned (thanks to `optional` library).

As for the UDP protocol, each message has to be reconstructed from bytes, therefore small, but readable messages have been created for that purpose. Those functions also decide how to react to the messages based on FSM.

## 5. Testing
### 5.1. Tools Used:
- Wireshark (version 4.4.5) with IPK25-CHAT protocol dissector plugin (provided in specification [(13)](#sources))
- Netcat (to receive and send messages on loopback)
- xxd (to convert hexadecimal messages into binary format)

### 5.2. Testing Environments
- Loopback
- Public reference server [(2)](#sources)

### 5.3. Testing Machines
- Laptop with Fedora Linux 41 (Workstation Edition) and Windows 11, with WSL2 (Debian GNU/Linux 12 (bookworm)).
- Provided virtual machine (Ubuntu 24.04.1 LTS)
- FIT Server: merlin.fit.vutbr.cz (Devuan GNU/Linux 5 (daedalus))

Note: It is not possible to compile this project natively on Windows because it lacks libraries like `sys/socket.h` [(14)](#sources). 
### 5.4. Testing UDP on Loopback:
1. Creating messsages from hexadecimal representation, e.g. PING, with MessageID 2.
```bash
echo "fd0002" >  ping_msg.hex
xxd -r -p x_msg.hex > ping_msg.bin
```
2. Start server: 
```bash
nc -4 -u -l -v 127.0.0.1 4567

Ncat: Version 7.92 ( https://nmap.org/ncat )
Ncat: Listening on 127.0.0.1:4567
```
3. Launch client with larger timeout value, because we are responding manually:
```bash
./ipk25chat-client -s 127.0.0.1 -t udp -d 25000
```
4. Start Wireshark, start scanning on `lo` and set filter `udp.port == 4567`
5. Send AUTH message from client: 
```bash
/auth name secret disp_name
```
6. Find out from which port is client sending the data with Wireshark (e.g. 87654).

**Note**: To make sending confirmation messages easier, the following script can be used:
```bash
#!/bin/bash
# File: make_confirm.sh
if [ $# -ne 1 ]; then
   exit 1
fi

MSG_ID=$1
HEX=$(printf "00%04x" $MSG_ID)
echo $HEX | xxd -r -p > confirm_msg.bin
```

7. Send confirmation message to the client using:
```bash
./make_confirm 0
nc -4 -u -v 127.0.0.1 87654 < confirm_msg.bin
```
(confirmations for following messages are omitted for simplicity's sake)

8. Send reply message to the client using
```bash
nc -4 -u -v 127.0.0.1 87654 < reply_msg.bin
```
9. Client now displays message `Action Success: welcome` and a debugging print notifies us that the dynamic address has been set.
```bash
src/client_comms.cpp:266  | receive_udp_packet | Stored dynamic server address: port 55655
Action Success: Auth success.
```
10. End old and start new server, where port `55655` is port from which the previous Netcat command sent the message.:
```bash
nc -4 -u -l -v 127.0.0.1 55655
```
11. Now it is possible for client to send messages to the server and server can send messages using 
```bash
nc -4 -u -v 127.0.0.1 87654 < msg_msg.bin`
```
12. End client by pressing `Ctrl+C` / `Ctrl+D`, or by sending a BYE or ERR message:
```bash
nc -4 -u -v 127.0.0.1 4567 < bye_msg.bin

# BYE message can be seen in Wireshark as well
3274	961.083449771	127.0.0.1	127.0.0.1	IPK25-CHAT	46	C → Server | ID=0, Type=bye
```
### 5.5. Testing TCP on Loopback:
1. Start server: `nc -C -4 -l 127.0.0.1 4567` 
2. Start client: `./ipk25chat-client -s 127.0.0.1 -t tcp`
3. Send AUTH message from client `/auth name secret disp_name`
4. Server now displays the message received from the client.
5. To send a reply ok message, we just type `REPLY OK IS welcome` and press ENTER.
6. Client now displays message `Action Success: welcome`.
7. Now client and server can send messages back and forth.
8. Client will exit on an ERR/BYE message received from the server or by `Ctrl+C/Ctrl+D`.
#### 5.5.1. Authentication example
Server is launched:
```bash
nc -l -C -4 -v 127.0.0.1 4567

Ncat: Version 7.92 ( https://nmap.org/ncat )
Ncat: Listening on 127.0.0.1:4567
```
Client is launched and user sends an AUTH message:
```bash
./ipk25chat-client -t tcp -s 127.0.0.1

/auth xlogin secret_seq disp_name
```

Server receives AUTH message and responds with positive REPLY:
```bash
AUTH xlogin AS disp_name USING secret_seq
REPLY OK IS welcome
```

Client acknowledges the REPLY message:
```bash
Action Success: welcome
/help
-----------------------------------------
Supported commands:
  /auth <username> <secret> <displayname>
  /join <channel>
  /rename <displayname>
  /help
Status:
  Current display name: disp_name
  Authenticated: true
-----------------------------------------
```

#### 5.5.2. Timeout example
REPLY message was not received within 5s:
```bash
AUTH xlogin AS disp_name USING secret_seq
ERROR: Authentication timed out. # Client sends BYE message
```
Server receives BYE message:
```bash
BYE FROM disp_name
```

#### 5.5.2. Sending messages and ending client session
Client (invalid message, then valid one, then exit):
```bash
^[[B          # accidental arrow key
ERROR: Invalid format of MessageContent, try again.
Hello 
Henry: I'm feeling quite hungry # message received from server
^C
```

Server receives message and sends message from another other user:
```bash
MSG FROM disp_name IS Hello # message received from client
MSG FROM Henry IS I'm feeling quite hungry # message received from other client
BYE FROM disp_name 
```

### 5.6. Testing TCP/UDP on public server
Testing was done only after the program has progressed enough to not cause any issues, such as retransmitting packages indefinitely and similar possible issues. In order to test out the join functionality, `.` character had to be temporarily allowed in `ChannelID` check.
- As for the testing itself, there wasn't really much to do. Connected using `/auth xlogin secret display_name`, then tried sending a message and joining other channels. At the end, after pressing `Ctrl+C`, chat messages were checked to see if `User left the channel` is present. 
- Singular issue encountered was that the client never received UDP reply from the server after trying to authenticate. Using FIT VPN [(12)](#sources) solved the problem.

## 6. Known Limitations / Edge Cases
- If no REPLY to AUTH is received, the client will be stuck in an AUTH state. This happens because of how the FSM logic is handled. This was discovered when FIT VPN wasn't used during testing on publicly available server (UDP). Possible fix could include forced receive in `send_auth()` function.

## 7. Regarding the Use of Artificial Intelligence
Models used:
- GPT-4o
- Qwen2.5-Coder (7B / 14B), hosted locally with Ollama [(4)](#sources)

Usage:
- Debugging and testing
- Discussing design decisions
- Clarifying the specification requirements
- Understanding aspects of C++ language
- Refining code structure (no copy-pasting)

## Sources
1. IPK Project 1 – OMEGA. L4 Scanner (completed assignment). Gitea, 2025. Available from: https://git.fit.vutbr.cz/xmervaj00/IPK_project
2.  IPK Project 2: Client for a chat server using the `IPK25-CHAT` protocol. Gitea, 2025. Accessed from https://git.fit.vutbr.cz/NESFIT/IPK-Projects/src/branch/master/Project_2#cli-arguments
3. DOLEJŠKA, Daniel. _IPK2023-24L-04-PROGRAMOVANI_. Brno: VUT FIT, 2023. Accessed from: https://moodle.vut.cz/pluginfile.php/1081877/mod_folder/content/0/IPK2023-24L-04-PROGRAMOVANI.pdf
4. Ollama. GitHub, 2025. Available from: https://github.com/ollama/ollama
5. Stack Overflow. Should I use size_t or ssize_t? 2015. Accessed from: https://stackoverflow.com/questions/15739490/should-i-use-size-t-or-ssize-t
6. Stack Overflow. Reading buffer from socket. 2011. Accessed from: https://stackoverflow.com/questions/3074824/reading-buffer-from-socket
7. Stack Overflow. How to use select on sockets properly? 2015. Accessed from: https://stackoverflow.com/questions/32711521/how-to-use-select-on-sockets-properly
8. Stack Overflow. In C networking, using select(), do I first have to listen() and accept()? 2010. Accessed from: https://stackoverflow.com/questions/2284428/in-c-networking-using-select-do-i-first-have-to-listen-and-accept
9. select(2) - monitor multiple file descriptors. Linux manual pages, 2024. Available from: https://man7.org/linux/man-pages/man2/select.2.html
10. select_tut(2) - background and tutorial information on the use select and pselect functions. Linux manual pages, 2024. Available from: https://man7.org/linux/man-pages/man2/select_tut.2.html
11. Hands-On Network Programming with C. O’Reilly Online Learning – select() timeout, 2019. Accessed from: https://www.oreilly.com/library/view/hands-on-network-programming/9781789349863/8e8ea0c3-cf7f-46c0-bd6f-5c7aa6eaa366.xhtml
12. FIT VPN - Configuration, 2025. Accessed from: https://www.fit.vut.cz/units/cvt/net/vpn.php.en
13. IPK projects guideline repository. Accessed from: https://git.fit.vutbr.cz/NESFIT/IPK-Projects/src/branch/master
14. Stack Overflow. How can I use sys/socket.h on Windows? 2023. Accessed from: https://stackoverflow.com/questions/67726142/how-can-i-use-sys-socket-h-on-windows