# netnote


### ABOUT:

A client/daemon pair inside a single executable. Netnote daemon maintains connection
with other netnote daemons on the local network that are configured to use the same
multicast address for host discovery. The netnote client can be used to request the
daemon to send a file to any connected peer, given that the user invoking the daemon
is part of the netnote group and has read permissions to the to-be-sent file.


### INSTALLATION:

Remember to check the installation script! 

Netnote uses GNU source extensions and as such will only build on Linux systems. 
Ensure that make is installed on your system. For most distros, it will be part of 
the 'build-essentials' package, or its equivalent.

Fetch the repo:
```
$ git clone https://github.com/vykt/netnote
```

Build:
```
$ cd netnote/src
$ make
```

Install:
```
$ cd ..
# ./install.sh
```

Add a user to the 'netnote' group:
```
# usermod -aG netnote [user]
```


### RUN:

The netnote daemon must be run as superuser. Systems where /sbin and /usr/sbin aren't 
visible to regular users (such as the default Debian install) must run the user 
using a login root shell. If you intend to run the daemon on startup, create a 
service file that executes the netnote daemon and ensure it runs after networking 
becomes available.

To execute the daemon, run:
```
# netnote -d
```

To list connected hosts, run:
```
$ netnote -l
```

To send a file to a host, run:
```
$ netnote -s [file] [host id]
```


### FILES:

Configuration file:
```
/etc/netnote.conf
```

Logs:
```
/var/log/netnote/netnoted.log
```

Process ID file:
```
/var/run/netnote/netnoted.pid
```


### CONFIGURATION:

The configuration file at ```/etc/netnote.conf``` uses the following format:
```
[key]=[value]
```

Comments may be added by starting the line with the '#' character, as such:
```
\#comment
```

The following configuration options are available and must be specified:
```
multicast_addr  - Host discovery multicast. Hosts wishing to connect must use 
                  the same multicast address.
				  Default: ff04:0000:ce73:602a:0942:bc84:f0b2:e25c
```

```
shared_udp_port - Host discovery traffic is listened to on this port. 
                  Default: 5148
```

```
shared_tcp_port - Incoming TCP connections for file transfer are listened to on
                  this port.
				  Default: 5149
```

```
downloads_path  - Received files will be stored at the directory specified here.
                  NOTE: The installation script creates a directory at the default 
				  location. If you intend to change this default, please manually 
				  create the new directory.
                  Default: /var/netnote
```


### EXAMPLES:

Start the daemon:
```
\# netnote -d
```

List connected hosts and their repsective IDs:
```
$ netnote -l
```

Send file 'reading_list' from current directory to host with ID of 2:
```
$ netnote -s reading_list 2
```


### FUTURE CONSIDERATIONS:
