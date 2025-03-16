---
title: Scripting
nav_order: 7
has_children: false
permalink: /scripting/
---

# INDI Scripting Tools

INDI provides three command line tools that can be used to write complex scripts for setting, retrieving, and controlling the behavior of INDI devices. These tools provide the programmer with a software equivalent of a closed loop control system where the behavior of INDI devices can be watched and corrected when desired. Furthermore, you can evaluate arithmetic expressions of INDI properties which would permit the formulation of very complex and nested requirements lists.

## indi_getprop

### Purpose

Retrieve readable properties from an INDI server.

### Usage

```bash
$ indi_getprop [options] [device.property.element ...]
```

Any component may be "\*" to match all (beware shell metacharacters). Reports all properties if none specified. BLOBs are saved in file named `device.property.element.format`.

In perl try: `%props = split (/[=\n]/, 'indi_getprop')`.

Set element to one of following to return property attribute:

```
_LABEL to report label
_GROUP to report group
_STATE to report state
_PERM to report perm
_TO to report timeout
_TS to report timestamp
```

### Output Format

Output is fully qualified name=value one per line or just value if -1 and exactly one query without wildcards.

### Options

```
-1    : print just value if expecting exactly one response
-d f  : use file descriptor f already open to server
-h h  : alternate host, default is localhost
-p p  : alternate port, default is 7624
-t t  : max time to wait, default is 2 secs
-v    : verbose (cumulative)
```

### Exit Status

```
0: found at least one match for each query
1: at least one query returned nothing
2: real trouble, try repeating with -v
```

### Examples

1. In a perl script, gather all properties for the default indiserver and save them in an associative array %props which can then be used to look up a property value by name:

   ```perl
   %props = split (/[=\n]/, 'indi_getprop');
   ```

2. Wait up to ten seconds to get the values of all properties from the Mount device on the given host and non-standard port:

   ```bash
   indi_getprop -h indihub -p 7625 -t 10 "Mount.*"
   ```

3. Print just current value of the WIND.SPEED element from the weather device:
   ```bash
   indi_getprop -1 weather.WIND.SPEED
   ```

## indi_setprop

### Purpose

Set one or more writable INDI properties. indi_setprop connects to an indiserver and sends commands to set new values for specified properties. Each property is specified using three components followed by the new value in the following form:

```
device.property.element=value
```

Since in element may be an array, the syntax allows for multiple elements for one property to be specified separated by semicolons, and the value to have an equal number of components also separated by semicolons. When specified in this manner, all elements are updated atomically. **For some properties, it is critical to send ALL components of the property atomically otherwise the individual values will be considered invalid.**

For example, to send the telescope position, you must send both RA/DEC at the same time to the driver.

If there are any spaces in the device or property names, then the whole command must be wrapped with double quotes as illustrated in the examples below.

### Usage

```bash
indi_setprop [options] device.property.e1[;e2...]=v1[;v2...] ...
```

### Options

```
-d f  : use file descriptor f already open to server
-h h  : alternate host, default is localhost
-p p  : alternate port, default is 7624
-t t  : max time to wait, default is 2 secs
-v    : verbose (cumulative)
```

### Exit Status

```
0: all settings successful
1: at least one setting was invalid
2: real trouble, try repeating with -v
```

### Examples

1. Send new lat/long location values atomically to the Mount driver:

   ```bash
   indi_setprop "Mount.GEOGRAPHIC_COORD.LAT;LONG=38;108"
   ```

2. Send new RA (06:00:00) / DEC (+50:00:00) value to EQMod Mount:
   ```bash
   indi_setprop "EQMod Mount.EQUATORIAL_EOD_COORD.RA;DEC=6;50"
   ```

## indi_eval

### Purpose

indi_eval connects to an indiserver and listens for the values of properties to evaluate an arithmetic expression. Each property is specified using three components enclosed in double quotes in the following form:

```
"device.property.element"
```

The last component of the property specification is usually the element name, but may be a reserved name to indicate an attribute of the property as a whole. These reserved names are as follows:

- \_STATE: the state attribute, where for the purposes of evaluation the usual keywords Idle, Ok, Busy, Alert are converted to the numeric values of 0, 1, 2 and 3 respectively.
- \_TS: evaluate the timestamp attribute as the number of UNIX seconds from epoch

Switch vectors evaluate to 0 or 1 based on the state values of Off and On, respectively. Light vectors evaluate to 0-3 similarly to the keywords described above for \_STATE.

The arithmetic expression, exp, follows the form of that used in the C programming language. The operators supported include:

```
! + - * / % ^ && || > >= == != <= <
```

and the mathematical functions supported include:

```
sin(rad) cos(rad) tan(rad) asin(x) acos(x) atan(x) atan2(y,x) abs(x) degrad(deg) raddeg(rad) floor(x) log(x) log10(x) exp(x) sqrt(x) pow(x,exp)
```

The value of PI can be specified using a constant named "pi".

### Options

```
-d f  : use file descriptor f already open to server
-e    : print each updated expression value
-f    : print final expression value
-h h  : alternate host, default is localhost
-i    : read expression from stdin
-o    : print operands as they change
-p p  : alternate port, default is 7624
-t t  : max secs to wait, 0 is forever, default is 2
-v    : verbose (cumulative)
-w    : wait for expression to evaluate as true
```

### Exit Status

Exit 0 if expression evaluates to non-0, 1 if 0, else 2

### Examples

1. To print 0/1 whether Security.Doors.Front or .Rear are in Alert:

   ```bash
   indi_eval -f '"Security.Doors.Front"==3 || "Security.Doors.Rear"==3'
   ```

2. To exit 0 if the Security property as a whole is in a state of OK:

   ```bash
   indi_eval '"Security.Security._STATE"==1'
   ```

3. To wait for RA and Dec to be near zero and watch their values as they change:
   ```bash
   indi_eval -t 0 -wo 'abs("Mount.Eq32k.RA")<.01 && abs("Mount.Eq32k.DEC")<.01'
   ```
