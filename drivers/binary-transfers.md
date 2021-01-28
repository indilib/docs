# Binary Transfers

Thus far, the main focus was on control and monitoring oriented properties. These are the properties that represent the physical buttons, knobs, lights, status indicators, and switches in devices. But these properties only support the acquisition of data in specific formats (number or text).

The data acquisition aspect of the protocol is critical to the operation of many astronomical devices that require the ability to send and receive data of any type. INDI provides a mechanism whereas a number of binary streams may travel to and from drivers using BLOB properties.

INDI Clients have the liberty of selecting the mode of data transfer operation to alleviate congestion. The client needs to explicitly enable data transfer capability. To receive binary streams, the client should enable BLOBs (Binary Large OBjects) as "Only" or "Also" (e.g. By sending `<enableBLOB>Also</enableBLOB>`).

BLOBs are disabled by default!. INDI server will only send BLOBs if the client explicitly opts to receive them. Do not forget to enable BLOB transfer in your client. Most clients enable BLOB transfers by default.

Using the Only mode, the driver will only send binary data to this particular client. On the other hand, in the Also mode, the driver will transmit both regular INDI properties along with BLOBs on the same channel. INDI automatically encodes data in base64 before transmission to the client (This insures a valid XML element is transmitted).

INDI server also expects data arriving from clients to be base64-encoded. The driver may optionally compress the data if desired. If compression is used, it must be done via ZLib, and a ".z" should be appended to the format string extension in the BLOB property to indicate that the data is compressed.

[tutorial_three](https://github.com/indilib/indi/tree/master/examples/tutorial_three) contains a demonstration on binary transfer. Data transfer in INDI is accomplished by using BLOB properties. A BLOB property has the following elements:

* `name`: The property unique name
* `label`: The property label, to be displayed in a GUI if desired
* `format`: The type of the data being transfered. The format tell clients how to interpret the data.
* `blob`: a pointer to the data.
* `bloblen`: n compressed bytes if compression is used. Otherwise, the number of bytes uncompressed.
* `size`: n bytes uncompressed.

In astronomical applications, the most common data format is FITS. Therefore, the format string for FITS BLOB property should be ".fits" (or ".fits.z" if compression is used). The ".fits" format is a standard format supported by several INDI clients. Other data formats used in INDI drivers include ".stream" format to transmit live video, and ".ccdpreview" to transmit previews of CCD frames.

To conclude, the basic steps for sending data to the client are the following:

1. Define INDI BLOB property.
1. Allocate memory for your BLOB data.
1. Compress your buffer, if desired.
1. Assign your buffer pointer to BLOB property (e.g. blobProperty->blob = myBuffer).
1. Specify data type in the format element. Append ".z" is compression is used.
1. Specify blob size in bytes (size).
1. Specify blob size in bytes when compressed (bloblen). If no compression is used, then this value should be equal to 'size' above.
1. Send BLOB to client.
1. Free memory resources allocated in step #2.
