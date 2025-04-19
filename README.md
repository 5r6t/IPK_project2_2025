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

* https://www.oreilly.com/library/view/hands-on-network-programming/9781789349863/8e8ea0c3-cf7f-46c0-bd6f-5c7aa6eaa366.xhtml -- timeout
* https://pubs.opengroup.org/onlinepubs/000095399/functions/setsockopt.html -- decided to not use

//* https://stackoverflow.com/questions/41893303/convert-usigned-integer-uint16-t-to-string-standard-itoa-base-10-is-giving-ne -- converting uint16_t
## Notes and more
- break; // first resolved address is used -> add to documentation?

## Architecture Overview 
```lisp
/**
 * Legend
 * init: gets constructed from
 * data: supplies configuration values
 * uses: functions supplied by said class
*/

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

# Conclusion
This document essentially does not advance the state of human knowledge in any way. I hope one day I'll have saved enough to isolate myself in the mountains. But all I do is run for cover...

Todays song: Mercy - Dotan