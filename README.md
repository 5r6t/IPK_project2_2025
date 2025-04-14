# IPK_project_dos

- [assignment](https://git.fit.vutbr.cz/NESFIT/IPK-Projects/src/branch/master/Project_2/README.md)
- [project_repo](https://git.fit.vutbr.cz/xmervaj00/IPK_project_dos)

## Sources
* https://man7.org/linux/man-pages/man3/sockaddr.3type.html

* IPK2023-24L-04-PROGRAMOVANI.pdf

* https://stackoverflow.com/questions/15739490/should-i-use-size-t-or-ssize-t

* https://stackoverflow.com/questions/3074824/reading-buffer-from-socket

* https://stackoverflow.com/questions/32711521/how-to-use-select-on-sockets-properly
* https://stackoverflow.com/questions/2284428/in-c-networking-using-select-do-i-first-have-to-listen-and-accept
* https://man7.org/linux/man-pages/man2/select.2.html
* https://man7.org/linux/man-pages/man2/select_tut.2.html

# Legend
```c#
/**
 * init: gets constructed from
 * data: supplies configuration values
 * uses: functions supplied by said class
*/

                     [ ipk25chat-client ]
                              ║ 
        ╔═══ (init+data) ═════╩══════ (init) ═════════╗
        ║                                             ║ 
        ║                                             ║
  [ Client Config ] ═════════ (data) ═════════> [ Client Session ]
        ║                                             ║  ║
        ║                                             ║  ║
        ╚══ (uses) ════> [ Toolkit ] <════ (uses) ════╝  ║
                                                         ║
        [ Client Comms ] <══════ (init+data+uses) ═══════╝
```

# Conclusion
This document essentially does not advance the state of human knowledge in any way. I hope one day I'll have saved enough to isolate myself in the mountains. But all I do is run for cover...

Todays song: Mercy - Dotan