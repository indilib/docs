---
title: Binary Transfers
nav_order: 5
parent: Basics
---

# Binary Transfers

Thus far, the main focus was on control and monitoring oriented properties. These are the properties that represent the physical buttons, knobs, lights, status indicators, and switches in devices. But these properties only support the acquisition of data in specific formats (number or text).

The data acquisition aspect of the protocol is critical to the operation of many astronomical devices that require the ability to send and receive data of any type. INDI provides a mechanism whereas a number of binary streams may travel to and from drivers using BLOB properties.

INDI Clients have the liberty of selecting the mode of data transfer operation to alleviate congestion. The client needs to explicitly enable data transfer capability. To receive binary streams, the client should enable BLOBs (Binary Large OBjects) as "Only" or "Also" (e.g. By sending `<enableBLOB>Also</enableBLOB>`).

BLOBs are disabled by default!. INDI server will only send BLOBs if the client explicitly opts to receive them. Do not forget to enable BLOB transfer in your client. Most clients enable BLOB transfers by default.

Using the Only mode, the driver will only send binary data to this particular client. On the other hand, in the Also mode, the driver will transmit both regular INDI properties along with BLOBs on the same channel. INDI automatically encodes data in base64 before transmission to the client (This insures a valid XML element is transmitted).

INDI server also expects data arriving from clients to be base64-encoded. The driver may optionally compress the data if desired. If compression is used, it must be done via ZLib, and a ".z" should be appended to the format string extension in the BLOB property to indicate that the data is compressed.

[tutorial_three](https://github.com/indilib/indi/tree/master/examples/tutorial_three) contains a demonstration on binary transfer. Data transfer in INDI is accomplished by using BLOB properties. A BLOB property has the following elements:

- `name`: The property unique name
- `label`: The property label, to be displayed in a GUI if desired
- `format`: The type of the data being transfered. The format tell clients how to interpret the data.
- `blob`: a pointer to the data.
- `bloblen`: n compressed bytes if compression is used. Otherwise, the number of bytes uncompressed.
- `size`: n bytes uncompressed.

In astronomical applications, the most common data format is FITS. Therefore, the format string for FITS BLOB property should be ".fits" (or ".fits.z" if compression is used). The ".fits" format is a standard format supported by several INDI clients. Other data formats used in INDI drivers include ".stream" format to transmit live video, and ".ccdpreview" to transmit previews of CCD frames.

To conclude, the basic steps for sending data to the client are the following:

1. Define INDI BLOB property.
2. Allocate memory for your BLOB data.
3. Compress your buffer, if desired.
4. Assign your buffer pointer to BLOB property (e.g. blobProperty->blob = myBuffer).
5. Specify data type in the format element. Append ".z" if compression is used.
6. Specify blob size in bytes (size).
7. Specify blob size in bytes when compressed (bloblen). If no compression is used, then this value should be equal to 'size' above.
8. Send BLOB to client.
9. Free memory resources allocated in step #2.

# Fast BLOBs

Fast BLOBs are supported starting from INDI v1.9.7+. Ludovic Pollet introduced Fast BLOBS to enable support for local connection and fast memory buffer exchange in the INDI protocol.

Working that way, data for BLOB (fits, stream, ...) needs no more being copied/base64 converted. The same memory is directly shared by driver to the client. This provides a significant reduction is CPU utilization and latency, especially on low-end HW.

This works only for client/server located on the same Unix host. In that case, BLOB are written into buffers (shm or memfd) that are then exchanged by reference and shared & mmaped in the client. This is very lightweight compared to the existing base64 transfer.

For remote connection, TCP is still supported for remote clients, unchanged. However, the shared buffer are used between driver and server, to eliminate handling there. In that case, the server handle the base64 encoding on a dedicated work thread.

Client that attempts to connect to localhost will be redirected to the local socket of the unix domain to take advantage. It is possible to target a specific unix socket path by using the syntax: localhost:/path/to/socket (an arg to indiserver is available to decide the path it listen on)

For client, since the existing semantic allows them to modify the blob data and that is not compatible with the new mechanism (blob are received as readonly), I added a new function for the client to explicitely allow readonly blob data. This removes one more copy of the data:

`camera_client->enableDirectBlobAccess(MYCCD, nullptr);`

There are further optimisations possible to avoid more memory copies, on the driver side (like producing the camera frame directly in the memory buffer instead of copying).

The fast-blob protocol is mostly the same than default one, with the following deviations:

- The client must connect to unix domain socket (only supported under Linux, MacOS lacks some important feature there...).
- the client has to reply to "pingRequest" messages by "pingReply" (this implements the back-pressure, see below why it's important). The uid attribute from the request must be repeated in the reply:

```
<pingRequest uid='2'/>
<pingReply uid='2'/>
```

on UNIX domain connection, the server can pass the blob content as an [anciliary file descriptor](http://man7.org/linux/man-pages/man7/unix.7.html). The file descriptor can later be mmapped. In that case, the xml message will have the attached property set to true (and no enclen by the way):

```
<setBLOBVector device='fakedev1' name='someblob' timestamp='2018-01-01T00:01:00'>
    <oneBLOB name='content' size='32' format='.fits' attached='true'/>
</setBLOBVector>
```

1. It is guaranteed that the file descriptor(s) for attached blob(s) are received by the end of the xml message. The exact position probably depends on the OS.
2. The server may also decide to not used shared buffer so the client must still support the use existing base64 layout (for example for small blobs)
3. The transport of buffers as filedescriptor can be very very fast, so your client may receive GB of data without much processing. Without backpressure, memory in the client side may grow uncontrolled (ultimately leading to Out of memory error, possibly system wide invoking the OOM killer). For this reason, it is strongly advised that when a blob arrive, the client stops processing incoming messages (especially replying to pingRequests) until it has finished processing the blob.
4. To actually free the memory used by a filedescriptor, the client must close the fd, and munmap the memory area.

Some example of low level functions for these tasks can be found in the indi source file integs/test.cpp (integration tests), which is somewhat decoupled from indi so probably easier to read for a start:

- integs/test.cpp: the unixSocketConnect function - to connect to a local socket by path name
- integs/ConnectionMock.cpp, method ConnectionMock::read, shows how to read the input channel, collecting received fds at the same time
- integs/SharedBuffer.cpp, shows how to access memory from a fd. It is in the write direction, while you client will want to read insted. So your client should use PROT_READ only and remove PROT_WRITE)

Tests codes in the integs directory may also be useful as they show the plain text version of the messages for various cases.
