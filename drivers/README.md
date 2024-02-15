# INDI Driver Development

This guide is written with the assumption that you are using the `libindi` C++
library to write your driver.

## Organization of INDI Drivers

The [INDI Main Repo](https://github.com/indilib/indi) includes several default drivers in addition to the main library. These drivers don't require extra libraries to work typically just serial or TCP communication with the hardware. The [INDI 3rd Party Repo](https://github.com/indilib/indi-3rdparty) contains many more drivers that can be developed independently of the main drivers. These may require extra libraries (for instance working with USB cameras) or just have release cycles outside of the slower main library. Having them separate also reduces the size of the main library and it's codebase.

When is a 3rd party driver required?

* Driver requires external dependencies (other than the indi-core package)
* Driver requires a separate release cycle
* Driver has a different license than indi-core

If you want to develop a new driver, it is recommended to create it separate from both, and once you are ready, integrate it into the 3rd party repo.

## Driver Construction

Example drivers can be found at [in the repo for this documentation](https://github.com/indilib/docs/tree/master/drivers/examples/).

An INDI driver is what directly communicates with your device(s) hardware. An INDI driver may control one or more hardware devices. It is responsible for controlling the device, and for representing the device properties to clients using INDI's protocol structures. *The driver does not contain a `main()`*, as it is expected to operate in an event-driven fashion. The core device class is `INDI::DefaultDevice`, and it encapsulates the functionality of the most basic INDI device driver.

The driver must implement each `ISxxx()` function but never calls them. The `IS()` functions are called by the reference implementation `main()` as messages arrive from Clients. Within each `IS()` function the driver performs the desired tasks and then may report back to the Client by calling the `IDxxx()` functions. The INDI library provides routines for common tasks such as serial communication, string formatting & conversion, and XML parsing. `libnova`, an external optional library, provides facility for astronomical calculations, while `cfitsio` provides handling for FITS files.

The reference API provides `IExxx()` functions to allow the driver to add its own callback functions if desired. The driver can arrange for functions to be called when reading a file descriptor that will not block; when a time interval has expired; or when there is no other client traffic in progress. Several utility functions to search and find INDI vector structs are provides for convenience in the API. The sample `indiserver` is a stand-alone process that may be used to run one or more INDI-compliant drivers. It takes the name of each driver process to run from its command line arguments. Once a binary driver is compiled, `indiserver` can load the driver and handle all data steering services between the driver and any number of clients.

The `INDI::Telescope`, `INDI::CCD`, `INDI::Focuser`, and `INDI::FilterWheel` classes provides the standard interface for those classes of devices. You need to subclass those classes in order to develop a driver for your device. If your device does not belong to those classes (i.e. weather station, or rain detector), then you may subclass `INDI::DefaultDevice` directly.

The device properties may be either hard-coded in the driver's code, or stored externally in a skeleton file. Which method you opt to utitlize depends on your preferences & devices. Drivers utilizing skeleton files have their properties stored in an XML file (usually under `/usr/share/indi` in Linux). The skeleton file naming convention is `driver_name_sk.xml`. For `indi_spectracyber` it is `indi_spectracyber_sk.xml`. The contents of the skeleton file is a list of `defXXX` XML statements enclosed by `<INDIDriver>` opening and closing tags. Support for automatic auxiliary controls a Driver may add debug, simulation, and configuration controls to the driver by calling `addAuxControls()`. For each driver, you must provide the device's default name and driver version information.

Tutorial Four is a simple driver to illustrate the skeleton method. Please make sure to install `tutorial_four_sk.xml` to `/usr/share/indi` or define `INDISKEL` envrionment variable to the path of this file before running the tutorial.

INDI Library uses CMake build system, please refer to the CMakeLists.txt file shipped with `libindi` and 3rd party drivers.

## Standard Properties

INDI does not place any special semantics on property names (i.e. properties are just texts, numbers, lights, blobs, or switches that represent no physical function). While GUI clients can construct graphical representation of properties in order to permit the user to operate the device, we run into situations where clients and drivers need to agree on the exact meaning of some fundamental properties.

What if some client need to be aware of the existence of some property in order to perform some function useful to the user? How can that client tie itself to such a property if the property can be arbitrary defined by drivers?

The solution is to define Standard Properties in order to establish a level of interoperability among INDI drivers and clients. We propose a set of shared INDI properties that encapsulate the most common characteristics of astronomical instrumentation of interest.

If the semantics of such properties are properly defined, not only it will insure base interoperability, but complete device automation becomes possible as well. Put another way, INDI standard properties are in essence properties that represent a clearly defined characteristic related to the operation of the device drivers.

For example, a very common standard property is `EQUATORIAL_EOD_COORD`. This property represents the telescope's current RA and DEC. Clients need to be aware of this property in order to, for example, draw the telescope's cross hair on the sky map. If you write a script to control a telescope, you know that any telescope supporting `EQUATORIAL_EOD_COORD` will behave in an expected manner when the property is invoked.

INDI clients are required to honor standard properties if when and they implement any functions associated with a particular standard property. Furthermore, INDI drivers employing standard properties should strictly adhere to the standard properties structure as defined here.

### General Properties

| Name             | Type   | Values           | Description                               |
| ---------------- | ------ | ---------------- | ----------------------------------------- |
| CONNECTION       | Switch | CONNECT          | Establish connection to the hardware      |
|                  |        | DISCONNECT       | Disconnect from the hardware              |
| DEVICE_PORT      | Text   | PORT             | Device connection port                    |
| TIME_LST         | Number | LST              | Local sidereal time HH:MM:SS              |
| TIME_UTC         | Text   | UTC              | UTC time in ISO 8601 format               |
|                  |        | OFFSET           | UTC offset, in hours +E                   |
| GEOGRAPHIC_COORD | Number | LAT              | Site latitude (-90 to +90) degrees +N     |
|                  |        | LONG             | Site longitude (0 to 360) degrees +E      |
|                  |        | ELEV             | Site elevation, meters                    |
| ATMOSPHERE       | Number | TEMPERATURE      | Kelvin                                    |
|                  |        | PRESSURE         | hPa                                       |
|                  |        | HUMIDITY         | Percentage %                              |
| UPLOAD_MODE      | Switch | UPLOAD_CLIENT    | Send BLOB to client                       |
|                  |        | UPLOAD_LOCAL     | Save BLOB locally                         |
|                  |        | UPLOAD_BOTH      | Send blob to client and save locally      |
| UPLOAD_SETTINGS  | Text   | UPLOAD_DIR       | Upload directory if BLOB is saved locally |
|                  |        | UPLOAD_PREFIX    | Prefix used when saving filename          |
| ACTIVE_DEVICES   | Text   | ACTIVE_TELESCOPE | Name of active devices.                   |
|                  |        | ACTIVE_CCD       |                                           |
|                  |        | ACTIVE_FILTER    |                                           |
|                  |        | ACTIVE_FOCUSER   |                                           |
|                  |        | ACTIVE_DOME      |                                           |
|                  |        | ACTIVE_GPS       |                                           |

`ACTIVE_DEVICES` is used to aid clients in automatically providing the users with a list of active devices (i.e. `CONNECTION` is `ON`) whenever needed. For example, a CCD driver may define `ACTIVE_DEVICES` property with one member: `ACTIVE_TELESCOPE`. Suppose that the client is also running LX200 Basic driver to control the telescope. If the telescope is connected, the client may automatically fill the `ACTIVE_TELESCOPE` field or provide a drop-down list of active telescopes to select from. Once set, the CCD driver may record, for example, the telescope name, RA, DEC, among other metadata once it captures an image. Therefore, `ACTIVE_DEVICES` is primarily used to link together different classes of devices to exchange information if required.

### Telescope Properties

Telescope standard properties define critical properties for the operation and control of the mount. In addition to the properties below, all properties in [INDI Alignment Subsystem](http://www.indilib.org/api/md_libs_indibase_alignment_alignment_white_paper.html) are considered standard properties as well and must be reserved in all implementations.

| Name                     | Type   | Values                 | Description                                                                                    |
| ------------------------ | ------ | ---------------------- | ---------------------------------------------------------------------------------------------- |
| EQUATORIAL_COORD         | Number |                        | Equatorial astrometric J2000 coordinate                                                        |
|                          |        | RA                     | J2000 RA, hours                                                                                |
|                          |        | DEC                    | J2000 Dec, degrees +N                                                                          |
| EQUATORIAL_EOD_COORD     | Number |                        | Equatorial astrometric epoch of date coordinate                                                |
|                          |        | RA                     | JNow RA, hours                                                                                 |
|                          |        | DEC                    | JNow Dec, degrees +N                                                                           |
| TARGET_EOD_COORD         | Number |                        | Slew Target. Read Only property set once requested EQUATORIAL_EOD_COORD is accepted by driver. |
|                          |        | RA                     | JNow RA, hours                                                                                 |
|                          |        | DEC                    | JNow Dec, degrees +N                                                                           |
| HORIZONTAL_COORD         | Number |                        | topocentric coordinate                                                                         |
|                          |        | ALT                    | Altitude, degrees above horizon                                                                |
|                          |        | AZ                     | Azimuth, degrees E of N                                                                        |
| ON_COORD_SET             | Switch |                        | Action device takes when sent any `*_COORD` property.                                          |
|                          |        | SLEW                   | Slew to a coordinate and stop upon receiving coordinates.                                      |
|                          |        | TRACK                  | Slew to a coordinate and track upon receiving coordinates.                                     |
|                          |        | SYNC                   | Accept coordinate as current position upon receiving coordinates.                              |
| TELESCOPE_MOTION_NS      | Switch |                        | Move telescope north or south                                                                  |
|                          |        | MOTION_NORTH           | Move the telescope toward North.                                                               |
|                          |        | MOTION_SOUTH           | Move the telescope toward South.                                                               |
| TELESCOPE_MOTION_WE      | Switch |                        | Move telescope west or east                                                                    |
|                          |        | MOTION_WEST            | Move the telescope toward West.                                                                |
|                          |        | MOTION_EAST            | Move the telescope toward East.                                                                |
| TELESCOPE_TIMED_GUIDE_NS | Number |                        | Timed pulse guide in north/south direction                                                     |
|                          |        | TIMED_GUIDE_N          | Guide the scope north for TIMED_GUIDE_N milliseconds.                                          |
|                          |        | TIMED_GUIDE_S          | Guide the scope south for TIMED_GUIDE_S milliseconds.                                          |
| TELESCOPE_TIMED_GUIDE_WE | Number |                        | Timed pulse guide in north/south direction                                                     |
|                          |        | TIMED_GUIDE_W          | Guide the scope west for TIMED_GUIDE_W milliseconds.                                           |
|                          |        | TIMED_GUIDE_E          | Guide the scope east for TIMED_GUIDE_E milliseconds.                                           |
| TELESCOPE_SLEW_RATE      | Switch |                        | Multiple switch slew rate.                                                                     |
|                          |        | SLEW_GUIDE             | 0.5x to 1.0x sidereal rate or slowest possible speed.                                          |
|                          |        | SLEW_CENTERING         | Slow speed.                                                                                    |
|                          |        | SLEW_FIND              | Medium speed.                                                                                  |
|                          |        | SLEW_MAX               | Maximum speed.                                                                                 |
| TELESCOPE_PARK           | Switch |                        | Park and unpark the telescope.                                                                 |
|                          |        | PARK                   | Park the telescope.                                                                            |
|                          |        | UNPARK                 | Unpark the telescope.                                                                          |
| TELESCOPE_PARK_POSITION  | Number |                        | Home park position (RA/DEC or AZ/ALT) in degrees or encoder ticks.                             |
|                          |        | PARK_RA or PARK_AZ     | RA/AZ park coordinates in degrees or ticks.                                                    |
|                          |        | PARK_DEC or PARK_ALT   | DEC/ALT park coordinates in degrees or ticks.                                                  |
| TELESCOPE_PARK_OPTION    | Switch |                        | Updates TELESCOPE_PARK_POSITION values                                                         |
|                          |        | PARK_CURRENT           | Use current coordinates/encoders as home park position.                                        |
|                          |        | PARK_DEFAULT           | Use driver's default park position.                                                            |
|                          |        | PARK_WRITE_DATA        | Write TELESCOPE_PARK_POSITION and current parking status to file ($HOME/.indi/ParkData.xml)    |
| TELESCOPE_ABORT_MOTION   | Switch | ABORT_MOTION           | Stop telescope rapidly, but gracefully                                                         |
| TELESCOPE_TRACK_RATE     | Switch |                        |                                                                                                |
|                          |        | TRACK_SIDEREAL         | Track at sidereal rate.                                                                        |
|                          |        | TRACK_SOLAR            | Track at solar rate.                                                                           |
|                          |        | TRACK_LUNAR            | Track at lunar rate.                                                                           |
|                          |        | TRACK_CUSTOM           | Custom track rate. This element is optional                                                    |
| TELESCOPE_INFO           | Number |                        |                                                                                                |
|                          |        | TELESCOPE_APERTURE     | Telescope aperture, mm                                                                         |
|                          |        | TELESCOPE_FOCAL_LENGTH | Telescope focal length, mm                                                                     |
|                          |        | GUIDER_APERTURE        | Guide telescope aperture, mm                                                                   |
|                          |        | GUIDER_FOCAL_LENGTH    | Guider telescope focal length, mm                                                              |
| TELESCOPE_PIER_SIDE      | Switch |                        | GEM Pier Side                                                                                  |
|                          |        | PIER_EAST              | Mount on the East side of pier (Pointing West).                                                |
|                          |        | PIER_WEST              | Mount on the West side of pier (Pointing East).                                                |

#### Notes

Setting the `ON_COORD_SET` property does not cause any action but it prepares the mount driver for the next action when any `*_COORD` number property is received. For example, to sync the mount, first set switch to `SYNC` and then send the `EQUATORIAL_EOD_COORD` with the desired sync coordinates.

The driver can define as many `TELESCOPE_SLEW_RATE` switches as desirable, but at minimum should implement the four switches above.

For clients that want to implement the GOTO functionality of a telescope, they should expect telescope drivers to define either:

* `EQUATORIAL_EOD_COORD` for current Epoch; and/or
* `HORIZONTAL_COORD`

### CCD Properties

| Name               | Type   | Values                | Description                                                                                              |
| ------------------ | ------ | --------------------- | -------------------------------------------------------------------------------------------------------- |
| CCD_EXPOSURE       | Number | CCD_EXPOSURE_VALUE    | Expose the CCD chip for CCD_EXPOSURE_VALUE seconds                                                       |
| CCD_ABORT_EXPOSURE | Number | ABORT                 | Abort CCD exposure                                                                                       |
| CCD_FRAME          | Number |                       | CCD frame size.                                                                                          |
|                    |        | X                     | Left-most pixel position                                                                                 |
|                    |        | Y                     | Top-most pixel position                                                                                  |
|                    |        | WIDTH                 | Frame width in pixels                                                                                    |
|                    |        | HEIGHT                | Frame height in pixels                                                                                   |
| CCD_TEMPERATURE    | Number | CCD_TEMPERATURE_VALUE | CCD chip temperature in degrees Celsius                                                                  |
| CCD_COOLER         | Switch |                       | CCD Cooler control.                                                                                      |
|                    |        | COOLER_ON             | Turn cooler on                                                                                           |
|                    |        | COOLER_OFF            | Turn cooler off                                                                                          |
| CCD_FRAME_TYPE     | Switch |                       |                                                                                                          |
|                    |        | FRAME_LIGHT           | Take a light frame exposure                                                                              |
|                    |        | FRAME_BIAS            | Take a bias frame exposure                                                                               |
|                    |        | FRAME_DARK            | Take a dark frame exposure                                                                               |
|                    |        | FRAME_FLAT            | Take a flat field frame exposure                                                                         |
| CCD_BINNING        | Number |                       |                                                                                                          |
|                    |        | HOR_BIN               | Horizontal binning                                                                                       |
|                    |        | VER_BIN               | Vertical binning                                                                                         |
| CCD_COMPRESSION    | Switch |                       |                                                                                                          |
|                    |        | CCD_COMPRESS          | Compress CCD frame (If FITS, it uses fpack to send a .fz file)                                           |
|                    |        | CCD_RAW               | Send raw CCD frame                                                                                       |
| CCD_FRAME_RESET    | Switch | RESET                 | Reset CCD frame to default X,Y,W, and H settings. Set binning to 1x1.                                    |
| CCD_INFO           | Number |                       |                                                                                                          |
|                    |        | CCD_MAX_X             | Resolution x                                                                                             |
|                    |        | CCD_MAX_Y             | Resolution y                                                                                             |
|                    |        | CCD_PIXEL_SIZE        | CCD pixel size in microns                                                                                |
|                    |        | CCD_PIXEL_SIZE_X      | Pixel size X, microns                                                                                    |
|                    |        | CCD_PIXEL_SIZE_Y      | Pixel size Y, microns                                                                                    |
|                    |        | CCD_BITSPERPIXEL      | Bits per pixel                                                                                           |
| CCD_CFA            | Text   |                       | Color Filter Array information if the CCD produces a bayered image. Debayering performed at client side. |
|                    |        | CFA_OFFSET_X          | CFA X Offset.                                                                                            |
|                    |        | CFA_OFFSET_Y          | CFA Y Offset.                                                                                            |
|                    |        | CFA_TYPE              | CFA Filter type (e.g. RGGB).                                                                             |
| CCD1               | BLOB   |                       | CCD1 for primary CCD, CCD2 for guider CCD.                                                               |
|                    |        | CCD1                  | Binary fits data encoded in base64. The CCD1.format is used to indicate the data type (e.g. ".fits")     |

Most `CCD_*` properties are also applicable to `GUIDER` chip, so replace `CCD` with `GUIDER` above to define properties for the `GUIDER` chip, if applicable.

### CCD Streaming

Some CCD drivers support streaming and recording of video streams. All the properties defined in `StreamManager` are considered reserved standard properties. The stream data is sent via the same CCD1 BLOB used for image transmission and therefore it is not possible to stream and send regular CCD captures at the same time.

| Name               | Type   | Values                   | Description                                                                           |
| ------------------ | ------ | ------------------------ | ------------------------------------------------------------------------------------- |
| CCD_VIDEO_STREAM   | Switch | STREAM_ON                | Turn on video stream                                                                  |
|                    |        | STREAM_OFF               | Turn off video stream                                                                 |
| STREAMING_EXPOSURE | Number | STREAMING_EXPOSURE_VALUE | Frame exposure values in seconds when streaming.                                      |
|                    |        | STREAMING_DIVISOR_VALUE  | The divisor is used to skip frames as way to throttle the stream down.                |
| FPS                | Number |                          | Read Only                                                                             |
|                    |        | EST_FPS                  | Instant frame rate.                                                                   |
|                    |        | AVG_FPS                  | Average FPS over 1 second.                                                            |
| RECORD_FILE        | Text   | RECORD_FILE_DIR          | Directory to save the file. It defaults to $HOME/indi__D_                             |
|                    |        | RECORD_FILE_NAME         | Recording file name. It defaults to indi_record__T_                                   |
| RECORD_OPTIONS     | Switch |                          | Set the desired duration in seconds or total frames required for the recording.       |
|                    |        | RECORD_DURATION          | Duration in seconds.                                                                  |
|                    |        | RECORD_FRAME_TOTAL       | Total number of frames required                                                       |
| RECORD_STREAM      | Switch |                          | Start or Stop the stream recording to a file.                                         |
|                    |        | RECORD_ON                | Start recording. Do not stop unless asked to.                                         |
|                    |        | RECORD_DURATION_ON       | Start recording until the duration set in `RECORD_OPTIONS` has elapsed.               |
|                    |        | RECORD_FRAME_ON          | Start recording until the number of frames set in `RECORD_OPTIONS` has been captured. |
|                    |        | RECORD_OFF               | Stops recording.                                                                      |
| Fast Exposure      | Switch | INDI_ENABLED             | Stops recording.                                                                      |
|                    |        | INDI_DISABLED            | Fast Exposure is used to enable camera to immediately begin capturing the next frames |

#### Notes

`STREAMING_EXPOSURE_VALUE` is advisory only as some streaming devices cannot control the exposure duration.

`STREAMING_DIVISOR_VALUE` is used to skip frames. By default, a divisor value of 1 does not skip any frames. A value of two would skip every other frame (thereby cutting the FPS in half). A frame is skipped when the number of captured frames % divisor is equal to zero.

`RECORD_FILE` Recorders are responsible for recording the video stream to a file. The recording file directory and name can be set via the RECORD_FILE property which is composed of RECORD_FILE_DIR and RECORD_FILE_NAME elements. You can specify a record directory name together with a file name. You may use special character sequences to generate dynamic names:

* _D_ is replaced with the date ('YYYY-MM-DD')
* _H_ is replaced with the time ('hh-mm-ss')
* _T_ is replaced with a timestamp
* _F_ is replaced with the filter name currently in use

`FAST EXPOSURE` Fast Exposure is used to enable camera to immediately begin capturing the next frame once the previous frame data is downloaded from the camera. With Fast Exposure disabled, the driver have to wait until the client initiates the next capture request. To minimize the downtime, Fast Exposure can be enabled to trigger for a specific number of frame in Fast Exposure Count property.

Once the initial capture is started, the driver would continue to capture all necessary frames without waiting for client until the count reaches zero. The Fast Exposure Count is decremented after each fast exposure is complete.

Upload Mode affects Fast Exposure behavior depending on the mode selected. For LOCAL mode, where images are saved locally to disk, Fast Exposure is most efficient. For CLIENT or BOTH modes, the driver have to transmit the data over network to the client. If the upload time exceeds the requested exposure time, then Fast Exposure cannot reliably work and the driver would abort the exposure.

### Filter Wheel Properties

| Name        | Type   | Values            | Description                                                                         |
| ----------- | ------ | ----------------- | ----------------------------------------------------------------------------------- |
| FILTER_SLOT | Number | FILTER_SLOT_VALUE | The filter wheel's current slot number. Important: Filter numbers start from 1 to N |
| FILTER_NAME | Text   | FILTER_NAME_VALUE | The filter wheel's current slot name                                                |

### Focuser Properties

| Name                 | Type   | Values                  | Description                                                                                         |
| -------------------- | ------ | ----------------------- | --------------------------------------------------------------------------------------------------- |
| FOCUS_SPEED          | Number |                         | Select focus speed from 0 to N where 0 maps to no motion, and N maps to the fastest speed possible  |
|                      |        | FOCUS_SPEED_VALUE       | Set focuser speed to SPEED                                                                          |
| FOCUS_MOTION         | Switch | FOCUS_INWARD            | Focus inward                                                                                        |
|                      |        | FOCUS_OUTWARD           | Focus outward                                                                                       |
| FOCUS_TIMER          | Number | FOCUS_TIMER_VALUE       | Focus in the direction of `FOCUS_MOTION` at rate `FOCUS_SPEED` for `FOCUS_TIMER_VALUE` milliseconds |
| REL_FOCUS_POSITION   | Number | FOCUS_RELATIVE_POSITION | Move # of steps in `FOCUS_MOTION` direction.                                                        |
| ABS_FOCUS_POSITION   | Number | FOCUS_ABSOLUTE_POSITION | Move to this absolute position.                                                                     |
| FOCUS_MAX            | Number | FOCUS_MAX_VALUE         | Focus maximum travel limit in steps.                                                                |
| FOCUS_REVERSE_MOTION | Switch | ENABLED                 | Reverse default motor direction                                                                     |
|                      |        | DISABLED                | Do not reverse, move motor in the default direction.                                                |
| FOCUS_ABORT_MOTION   | Switch | ABORT                   | Abort focus motion.                                                                                 |
| FOCUS_SYNC           | Number | FOCUS_SYNC_VALUE        | Accept this position as the new focuser absolute position.                                          |

### Dome Properties

| Name              | Type   | Values                 | Description                                                                                              |
| ----------------- | ------ | ---------------------- | -------------------------------------------------------------------------------------------------------- |
| DOME_SPEED        | Number | DOME_SPEED_VALUE       | Set dome speed in RPM.                                                                                   |
| DOME_MOTION       | Switch | DOME_CW                | Move dome Clockwise, looking down                                                                        |
|                   |        | DOME_CCW               | Move dome counter clockwise, looking down                                                                |
| DOME_TIMER        | Number | DOME_TIMER_VALUE       | Move the dome in the direction of `DOME_MOTION` at rate `DOME_SPEED` for `DOME_TIMER_VALUE` milliseconds |
| REL_DOME_POSITION | Number | DOME_RELATIVE_POSITION | Move `DOME_RELATIVE_POSITION` degrees azimuth in the direction of `DOME_MOTION`.                         |
| ABS_DOME_POSITION | Number | DOME_ABSOLUTE_POSITION | Move dome to `DOME_ABSOLUTE_POSITION` absolute azimuth angle in degrees.                                 |
| DOME_ABORT_MOTION | Switch | ABORT                  | Abort dome motion.                                                                                       |
| DOME_SHUTTER      | Switch | SHUTTER_OPEN           | Open dome shutter.                                                                                       |
|                   |        | SHUTTER_CLOSE          | Close dome shutter.                                                                                      |
| DOME_GOTO         | Switch | DOME_HOME              | Go to home position.                                                                                     |
|                   |        | DOME_PARK              | Go to park position.                                                                                     |
| DOME_PARAMS       | Number | HOME_POSITION          | Dome home position in absolute degrees azimuth.                                                          |
|                   |        | PARK_POSITION          | Dome parking position in absolute degrees azimuth.                                                       |
|                   |        | AUTOSYNC_THRESHOLD     | see note below                                                                                           |
| DOME_AUTOSYNC     | Switch | DOME_AUTOSYNC_ENABLE   | Enable dome slaving.                                                                                     |
|                   |        | DOME_AUTOSYNC_DISABLE  | Disable dome slaving.                                                                                    |
| DOME_MEASUREMENTS | Number | DM_DOME_RADIUS         | Dome radius (m).                                                                                         |
|                   |        | DOME_SHUTTER_WIDTH     | Dome shutter width (m).                                                                                  |
|                   |        | DM_NORTH_DISPLACEMENT  | Displacement to the north of the mount center (m).                                                       |
|                   |        | DM_EAST_DISPLACEMENT   | Displacement to the east of the mount center (m).                                                        |
|                   |        | DM_UP_DISPLACEMENT     | UP displacement of the mount center (m).                                                                 |
|                   |        | DM_OTA_OFFSET          | Distance from the optical axis to the mount center (m).                                                  |

#### Notes

* `AUTOSYNC_THRESHOLD`: If dome is slaved, `AUTOSYNC_THRESHOLD` is the number of acceptable azimuth degrees error between reported and requested dome position. Once the difference between target and current dome positions exceed this value, the dome shall be commanded to move to the target position.
