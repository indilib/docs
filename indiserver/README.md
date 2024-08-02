# indiserver

## Running `indiserver`

```
$ indiserver [options] driver [driver ...]
Options:
 -l d     : log driver messages to <d>/YYYY-MM-DD.islog
 -m m     : kill client if gets more than this many MB behind, default 10
 -d m     : drop streaming blobs if client gets more than this many MB behind, default 5. 0 to disable
 -u path  : Path for the local connection socket (abstract), default /tmp/indiserver
 -p p     : alternate IP port, default 7624
 -f path  : Path to fifo for dynamic startup and shutdown of drivers.
 -r r     : maximum driver restarts on error, default 10.
 -v       : show key events, no traffic
 -vv      : -v + key message content
 -vvv     : -vv + complete xml
driver    : executable or device@host[:port]
```

Each additional argument can be either the name of a local program to run or a specification of an INDI Device on a remote `indiserver`. A local program is specified as the path name of the execuble to run. The program is presumed to implement the INDI protocol on its stdin and stdout channels, and may send ad-hoc out-of-band error or trace messages to its stderr which gets merged in with the `indiserver` stderr.

A remote Device is given in the form `device@host[:port]`, where device is the INDI device already available on another running instance of `indiserver`, host is the TCP host name on which said instance is running and the optional port is the port on which to connect if other than the standard port 7624.

Note that local programs specify the name of the executable file, not the Device it implements. Conversely, remote connections specify the name of the Device, irrespective of the name of its Driver program.

`indiserver` is intended to run forever and so never exits normally. If it does exit, it prints a message to stderr and exits with status 1.

## Dynamic Driver Startup & Shutdown

Starting `indiserver` in FIFO mode will allow you to dynamically start and stop
drivers. To use INDI server in FIFO mode, first create a FIFO. For example,
under Linux:

```bash
mkfifo /tmp/myFIFO
```

Then pass the -f parameter along with the path name of the FIFO when starting
`indiserver`:

```bash
indiserver -f /tmp/myFIFO
```

A client/script can write to this FIFO to request starting and shutting down of
drivers using the following syntax:

To start a driver:

```bash
start driver_path_name -n ["driver_name"] -c ["driver_config_path_name"] -s ["skeleton_file_path_name"]
```

To stop a driver:

```bash
stop driver_path_name -n ["driver_name"]
```

The `driver_name` parameter is optional and specifies the name of the driver to
be defined to the client. The optinal `driver_config_path_name` specifies the
configuration file to be used, if any. Finally, `skeleton_file_path_name`
designates the skeleton file to be used for this driver if desired. These
variables are set to environment variables `INDIDEV`, `INDICONFIG`, and
`INDISKEL` respectively. It is important to note that INDI server only defines
these environment variables, and it is up to the driver to read and utilize them
upon startup. All drivers based on `INDI::DefaultDriver` automatically check the
environment variable and set the device name accordingly.

It is critical to enclose all optional variables (`driver_name`,
`driver_config_path_name`, and `skeleton_file_path_name`) with double quotation
marks when issuing FIFO commands.

For example, to start `indi_lx200generic` driver, but with two driver names
(LX90 and "My ETX 90"), we write to FIFO:

```
start indi_lx200generic -n "LX90"
start indi_lx200generic -n "My ETX 90"
```

When redirecting commands to FIFO via `echo`, ensure to add blackslash to
quotes so that they do not get escaped:

```
echo "start indi_lx200generic -n \"My ETX 90\"" > myFIFO
```

To stop a driver, you must specify the driver_path_name at least. But if you
are running more than one instance of the driver, you should also specify the
`driver_name`. If no `driver_name` is specified, the first driver in INDI server
that matches `driver_path_name` will be stopped. For example, to stop
"My ETX 90", write this to the FIFO:

```
stop indi_lx200generic "My ETX 90"
```
