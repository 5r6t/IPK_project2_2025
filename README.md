# IPK Project 2 – Chat Client (`ipk25chat-client`)

## Assignment and Repository
- [Assignment Link](https://git.fit.vutbr.cz/NESFIT/IPK-Projects/src/branch/master/Project_2/README.md)
- [Project Repository Link](https://git.fit.vutbr.cz/xmervaj00/IPK_project_dos)

## 1. Project Overview
- Brief description of what the client does
- Supported protocols: TCP and UDP

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
- Argument order is flexible
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
- Sending and receiving in real time is handled by using `select()`. While `poll()` is better than select by allowing more descriptors, in our case, `select()` is enough.

**TCP behavior**:

Because TCP is a byte stream, received messages are stored in a buffer, to handle them in order without dropping anything. Timeout of 5 seconds gives server enough time to respond. In case no response is received, program gracefully ends (meaning it sends ERR/BYE to the server and ends connection without any RST flags). 

**UDP behavior**: 

UDP is considered lossy, and therefore, it is important to handle its flaws on an application level. Each message sent is expected to receive a confirmation message, and likewise, each received message to be sent a confirmation.

### 4.3. Packet Parsing
While using TCP protocol, `tcp_buffer` is checked for delimiters, in order to extract complete messages. If a complete message is found, it is sent to `parse_tcp_message(msg)`, where either a match occurs, and a filled `ParseMessage` structure is returned, or no match results in nothing being returned (thanks to `optional` library).

As for the UDP protocol, each message has to be reconstructed from bytes, therefore small, but readable messages have been created for that purpose. Those functions also decide how to react to the messages based on FSM.

##5. Testing
### 5.1. Tools Used:
- Wireshark (version 4.4.5) with IPK25-CHAT protocol dissector plugin (provided in [specification](https://git.fit.vutbr.cz/NESFIT/IPK-Projects/src/branch/master/Project_2#cli-arguments))
- Netcat (to receive and send messages on loopback)
- xxd (to convert hexadecimal messages into binary format)

### 5.2. Testing Environments
- Loopback
- Public reference server (provided in [specification](https://git.fit.vutbr.cz/NESFIT/IPK-Projects/src/branch/master/Project_2#cli-arguments))

### 5.3. Testing Machines
- Laptop with Fedora Linux 41 (Workstation Edition) and Windows 11 installed.
- While there was an attempt to test on Windows 11 and Windows Linux Subsystem 2 (WSL), it has been dropped.
- Provided virtual machine was used sparingly.

### 5.4. Testing UDP on Loopback:
1. Create message from hex (reply/auth/msg/confirm) `xxd -r -p x_msg.hex > x_msg.bin`
2. Start server: `nc -4 -u -l -v 127.0.0.1 4567`
3. Start client: `./ipk25chat-client -s 127.0.0.1 -t udp -d 25000` with larger timeout value, because we are responding manually
4. Start Wireshark, start scanning on `lo` and set filter `udp.port == 4567`
5. Send AUTH message from client `/auth name secret disp_name`
6. Find out from which port is client sending the data with Wireshark (e.g. 87654).
7. Send confirmation message to the client using `nc -4 -u -v 127.0.0.1 87654 < confirm_msg.bin`
8. Send reply message to the client using  `nc -4 -u -v 127.0.0.1 87654 < reply_msg.bin`
9. Client now displays message `Action Success: welcome` and a debugging print notifies us that the dynamic address has been set.
10. End old and start new server: `nc -4 -u -l -v 127.0.0.1 55655`, where port `55655` is port from which the previous Netcat command sent the message.
11. Now it is possible for client to send messages to the server and server can send messages using `nc -4 -u -v 127.0.0.1 87654 < msg_msg.bin` (confirmation messages are omitted for simplicity's sake).
12. End client by `Ctrl+C/Ctrl+D` or send `nc -4 -u -v 127.0.0.1 4567 < bye_msg.bin` (or err_msg.bin)`.

### 5.5. Testing TCP on Loopback:
1. Start server: `nc -C -4 -l 127.0.0.1 4567` 
2. Start client: `./ipk25chat-client -s 127.0.0.1 -t tcp`
3. Send AUTH message from client `/auth name secret disp_name`
4. Server now displays the message received from the client.
5. To send a reply ok message, we just type `REPLY OK IS welcome` and press ENTER.
6. Client now displays message `Action Success: welcome`.
7. Now client and server can send messages back and forth.
8. Client will exit on an ERR/BYE message received from the server or by `Ctrl+C/Ctrl+D`.

### 5.6. Testing TCP/UDP on public server
Testing was done only after the program has progressed enough to not cause any issues, such as retransmitting packages indefinitely and similar possible issues. In order to test out the join functionality, `.` character had to be temporarily allowed in `ChannelID` check.
- As for the testing itself, there wasn't really much to do. Connected using `/auth xlogin secret display_name`, then tried sending a message and joining other channels. At the end, after pressing `Ctrl+C`, chat messages were checked to see if `User left the channel` is present. 
- Singular issue encountered was that the client never received UDP reply from the server after trying to authenticate. Using FIT VPN solved the problem.

## 6. Known Limitations / Edge Cases
- If no REPLY to AUTH is received, the client will be stuck in an AUTH state. This happens because of how the FSM logic is handled. This was discovered when FIT VPN wasn't used during testing on publicly available server (UDP). Possible fix could include forced receive in `send_auth()` function.

## 7. Regarding the Use of Artificial Intelligence
Models used for this purpose:
- GPT-4o
- Qwen2.5-Coder (7B / 14B), hosted locally with [Ollama](https://github.com/ollama/ollama)

Usage:
- Debugging
- Discussing design decisions
- Clarifying the specification requirements
- Understanding aspects of C++ language
- Refining code structure

None of the code generated by AI was directly copied.

## 8. Sources
* [IPK Project 1 - OMEGA: L4 Scanner](https://git.fit.vutbr.cz/xmervaj00/IPK_project)
* https://man7.org/linux/man-pages/man3/sockaddr.3type.html

* [IPK2023-24L-04-PROGRAMOVANI.pdf](https://moodle.vut.cz/pluginfile.php/1081877/mod_folder/content/0/IPK2023-24L-04-PROGRAMOVANI.pdf?forcedownload=1)

* https://stackoverflow.com/questions/15739490/should-i-use-size-t-or-ssize-t

* https://stackoverflow.com/questions/3074824/reading-buffer-from-socket

* https://stackoverflow.com/questions/32711521/how-to-use-select-on-sockets-properly
* https://stackoverflow.com/questions/2284428/in-c-networking-using-select-do-i-first-have-to-listen-and-accept
* https://man7.org/linux/man-pages/man2/select.2.html
* https://man7.org/linux/man-pages/man2/select_tut.2.html

* https://www.oreilly.com/library/view/hands-on-network-programming/9781789349863/8e8ea0c3-cf7f-46c0-bd6f-5c7aa6eaa366.xhtml