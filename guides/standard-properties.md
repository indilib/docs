---
sort: 10
---

# INDI Standard Properties

## Introduction

INDI does not place any special semantics on property names (i.e. properties are just texts, numbers, or switches that represent no physical function). While GUI clients can construct graphical representation of properties in order to permit the user to operate the device, we run into situations where clients and drivers need to agree on the exact meaning of some fundamental properties.

What if some client need to be aware of the existence of some property in order to perform some function useful to the user? How can that client tie itself to such a property if the property can be arbitrary defined by drivers?

The solution is to define Standard Properties in order to establish a level of interoperability among INDI drivers and clients. We propose a set of shared INDI properties that encapsulate the most common characteristics of astronomical instrumentation of interest.

If the semantics of such properties are properly defined, not only it will insure base interoperability, but complete device automation becomes possible as well. Put another way, INDI standard properties are in essence properties that represent a clearly defined characteristic related to the operation of the device drivers.

For example, a very common standard property is `EQUATORIAL_EOD_COORD`. This property represents the telescope's current RA and DEC. Clients need to be aware of this property in order to, for example, draw the telescope's cross hair on the sky map. If you write a script to control a telescope, you know that any telescope supporting `EQUATORIAL_EOD_COORD` will behave in an expected manner when the property is invoked.

INDI clients are required to honor standard properties if when and they implement any functions associated with a particular standard property. Furthermore, INDI drivers employing standard properties should strictly adhere to the standard properties structure as defined here.

## General Properties

The following tables describe standard properties pertaining to generic devices and class-specific devices like telescope, CCDs, etc. The name of a property is the text that appears in the `name` attribute of a `defXXX` message.

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
| ACTIVE_DEVICES   | Text   | ACTIVE_TELESCOPE | Name of active telescope device           |
|                  |        | ACTIVE_CCD       | Name of active CCD device                 |
|                  |        | ACTIVE_FILTER    | Name of active filter wheel device        |
|                  |        | ACTIVE_FOCUSER   | Name of active focuser device             |
|                  |        | ACTIVE_DOME      | Name of active dome device                |
|                  |        | ACTIVE_GPS       | Name of active GPS device                 |

`ACTIVE_DEVICES` is used to aid clients in automatically providing the users with a list of active devices (i.e. `CONNECTION` is `ON`) whenever needed. For example, a CCD driver may define `ACTIVE_DEVICES` property with one member: `ACTIVE_TELESCOPE`. Suppose that the client is also running LX200 Basic driver to control the telescope. If the telescope is connected, the client may automatically fill the `ACTIVE_TELESCOPE` field or provide a drop-down list of active telescopes to select from. Once set, the CCD driver may record, for example, the telescope name, RA, DEC, among other metadata once it captures an image. Therefore, `ACTIVE_DEVICES` is primarily used to link together different classes of devices to exchange information if required.

## Telescope Properties

Telescope standard properties define critical properties for the operation and control of the mount. In addition to the properties below, all properties in [INDI Alignment Subsystem](http://www.indilib.org/api/md_libs_indibase_alignment_alignment_white_paper.html) are considered standard properties as well and must be reserved in all implementations.

| Name                     | Type   | Values                       | Description                                                                                    |
| ------------------------ | ------ | ---------------------------- | ---------------------------------------------------------------------------------------------- |
| EQUATORIAL_COORD         | Number |                              | Equatorial astrometric J2000 coordinate                                                        |
|                          |        | RA                           | J2000 RA, hours                                                                                |
|                          |        | DEC                          | J2000 Dec, degrees +N                                                                          |
| EQUATORIAL_EOD_COORD     | Number |                              | Equatorial astrometric epoch of date coordinate                                                |
|                          |        | RA                           | JNow RA, hours                                                                                 |
|                          |        | DEC                          | JNow Dec, degrees +N                                                                           |
| TARGET_EOD_COORD         | Number |                              | Slew Target. Read Only property set once requested EQUATORIAL_EOD_COORD is accepted by driver. |
|                          |        | RA                           | JNow RA, hours                                                                                 |
|                          |        | DEC                          | JNow Dec, degrees +N                                                                           |
| HORIZONTAL_COORD         | Number |                              | topocentric coordinate                                                                         |
|                          |        | ALT                          | Altitude, degrees above horizon                                                                |
|                          |        | AZ                           | Azimuth, degrees E of N                                                                        |
| ON_COORD_SET             | Switch |                              | Action device takes when sent any `*_COORD` property.                                          |
|                          |        | SLEW                         | Slew to a coordinate and stop upon receiving coordinates.                                      |
|                          |        | TRACK                        | Slew to a coordinate and track upon receiving coordinates.                                     |
|                          |        | SYNC                         | Accept coordinate as current position upon receiving coordinates.                              |
| TELESCOPE_MOTION_NS      | Switch |                              | Move telescope north or south                                                                  |
|                          |        | MOTION_NORTH                 | Move the telescope toward North.                                                               |
|                          |        | MOTION_SOUTH                 | Move the telescope toward South.                                                               |
| TELESCOPE_MOTION_WE      | Switch |                              | Move telescope west or east                                                                    |
|                          |        | MOTION_WEST                  | Move the telescope toward West.                                                                |
|                          |        | MOTION_EAST                  | Move the telescope toward East.                                                                |
| TELESCOPE_TIMED_GUIDE_NS | Number |                              | Timed pulse guide in north/south direction                                                     |
|                          |        | TIMED_GUIDE_N                | Guide the scope north for TIMED_GUIDE_N milliseconds.                                          |
|                          |        | TIMED_GUIDE_S                | Guide the scope south for TIMED_GUIDE_S milliseconds.                                          |
| TELESCOPE_TIMED_GUIDE_WE | Number |                              | Timed pulse guide in north/south direction                                                     |
|                          |        | TIMED_GUIDE_W                | Guide the scope west for TIMED_GUIDE_W milliseconds.                                           |
|                          |        | TIMED_GUIDE_E                | Guide the scope east for TIMED_GUIDE_E milliseconds.                                           |
| TELESCOPE_SLEW_RATE      | Switch |                              | Multiple switch slew rate.                                                                     |
|                          |        | SLEW_GUIDE                   | 0.5x to 1.0x sidereal rate or slowest possible speed.                                          |
|                          |        | SLEW_CENTERING               | Slow speed.                                                                                    |
|                          |        | SLEW_FIND                    | Medium speed.                                                                                  |
|                          |        | SLEW_MAX                     | Maximum speed.                                                                                 |
| TELESCOPE_PARK           | Switch |                              | Park and unpark the telescope.                                                                 |
|                          |        | PARK                         | Park the telescope.                                                                            |
|                          |        | UNPARK                       | Unpark the telescope.                                                                          |
| TELESCOPE_PARK_POSITION  | Number |                              | Home park position (RA/DEC or AZ/ALT) in degrees or encoder ticks.                             |
|                          |        | PARK_RA or PARK_AZ           | RA/AZ park coordinates in degrees or ticks.                                                    |
|                          |        | PARK_DEC or PARK_ALT         | DEC/ALT park coordinates in degrees or ticks.                                                  |
| TELESCOPE_PARK_OPTION    | Switch |                              | Updates TELESCOPE_PARK_POSITION values                                                         |
|                          |        | PARK_CURRENT                 | Use current coordinates/encoders as home park position.                                        |
|                          |        | PARK_DEFAULT                 | Use driver's default park position.                                                            |
|                          |        | PARK_WRITE_DATA              | Write TELESCOPE_PARK_POSITION and current parking status to file ($HOME/.indi/ParkData.xml)    |
|                          |        | PARK_PURGE_DATA              | Delete parking entry for this mount from ~/.indi/ParkData.xml file                             |
| TELESCOPE_ABORT_MOTION   | Switch | ABORT_MOTION                 | Stop telescope rapidly, but gracefully                                                         |
| TELESCOPE_TRACK_RATE     | Number |                              | Custom tracking rates in arcseconds per second                                                 |
|                          |        | TRACK_RATE_RA                | RA tracking rate                                                                               |
|                          |        | TRACK_RATE_DE                | DEC tracking rate                                                                              |
| TELESCOPE_INFO           | Number |                              |                                                                                                |
|                          |        | TELESCOPE_APERTURE           | Telescope aperture, mm                                                                         |
|                          |        | TELESCOPE_FOCAL_LENGTH       | Telescope focal length, mm                                                                     |
|                          |        | GUIDER_APERTURE              | Guide telescope aperture, mm                                                                   |
|                          |        | GUIDER_FOCAL_LENGTH          | Guider telescope focal length, mm                                                              |
| TELESCOPE_PIER_SIDE      | Switch |                              | GEM Pier Side                                                                                  |
|                          |        | PIER_EAST                    | Mount on the East side of pier (Pointing West).                                                |
|                          |        | PIER_WEST                    | Mount on the West side of pier (Pointing East).                                                |
| TELESCOPE_HOME           | Switch |                              | Home position operations                                                                       |
|                          |        | FIND                         | Find home position                                                                             |
|                          |        | SET                          | Set current position as home                                                                   |
|                          |        | GO                           | Go to home position                                                                            |
| DOME_POLICY              | Switch |                              |                                                                                                |
|                          |        | DOME_IGNORED                 | Ignore dome status when parking and unparking                                                  |
|                          |        | DOME_LOCKS                   | Prevents mounts from unparking while the dome is parked.                                       |
| PEC                      | Switch |                              |                                                                                                |
|                          |        | PEC OFF                      |                                                                                                |
|                          |        | PEC ON                       |                                                                                                |
| TELESCOPE_TRACK_MODE     | Switch |                              | Common pre-selected tracking rates for RA axis                                                 |
|                          |        | TRACK_SIDEREAL               | Sidereal Rate                                                                                  |
|                          |        | TRACK_SOLAR                  | Solar Rate                                                                                     |
|                          |        | TRACK_LUNAR                  | Lunar Rate                                                                                     |
|                          |        | TRACK_CUSTOM                 | Custom tracking rate in arcseconds per second.                                                 |
| TELESCOPE_TRACK_STATE    | Switch |                              |                                                                                                |
|                          |        | TRACK_ON                     | Toggle mount tracking on                                                                       |
|                          |        | TRACK_OFF                    | Toggle mount tracking off                                                                      |
| SAT_TLE_TEXT             | Text   | TLE                          | Satellite TLE                                                                                  |
| SAT_PASS_WINDOW          | Text   |                              |                                                                                                |
|                          |        | SAT_PASS_WINDOW_END          |                                                                                                |
|                          |        | SAT_PASS_WINDOW_START        |                                                                                                |
| SAT_TRACKING_STAT        | Switch |                              |                                                                                                |
|                          |        | SAT_TRACK                    |                                                                                                |
|                          |        | SAT_HALT                     |                                                                                                |
| TELESCOPE_REVERSE_MOTION | Switch |                              | Toggle motion reversal for each axis                                                           |
|                          |        | REVERSE_NS                   | Toggle North/South or Declination Axis Reversal                                                |
|                          |        | REVERSE_WE                   | Toggle West/East or Right Ascension Axis Reversal                                              |
| MOTION_CONTROL_MODE      | Switch |                              | Joystick motion control mode                                                                   |
|                          |        | MOTION_CONTROL_MODE_JOYSTICK | 4-Way Joystick                                                                                 |
|                          |        | MOTION_CONTROL_MODE_AXES     | Two Separate Axes                                                                              |
| JOYSTICK_LOCK_AXIS       | Switch |                              | Select which joystick axes are locked                                                          |
|                          |        | LOCK_AXIS_1                  | West/East axis                                                                                 |
|                          |        | LOCK_AXIS_2                  | North/South axis                                                                               |
| SIMULATE_PIER_SIDE       | Switch |                              | Simulate pier side for mounts that don't report it                                             |
|                          |        | SIMULATE_YES                 | Yes                                                                                            |
|                          |        | SIMULATE_NO                  | No                                                                                             |

### Notes

Setting the `ON_COORD_SET` property does not cause any action but it prepares the mount driver for the next action when any `*_COORD` number property is received. For example, to sync the mount, first set switch to `SYNC` and then send the `EQUATORIAL_EOD_COORD` with the desired sync coordinates.

The driver can define as many `TELESCOPE_SLEW_RATE` switches as desirable, but at minimum should implement the four switches above.

For clients that want to implement the GOTO functionality of a telescope, they should expect telescope drivers to define either:

- `EQUATORIAL_EOD_COORD` for current Epoch; and/or
- `HORIZONTAL_COORD`

## CCD Properties

| Name                | Type   | Values                | Description                                                                                               |
| ------------------- | ------ | --------------------- | --------------------------------------------------------------------------------------------------------- |
| CCD_EXPOSURE        | Number | CCD_EXPOSURE_VALUE    | Expose the CCD chip for CCD_EXPOSURE_VALUE seconds                                                        |
| CCD_ABORT_EXPOSURE  | Number | ABORT                 | Abort CCD exposure                                                                                        |
| CCD_FRAME           | Number |                       | CCD frame size.                                                                                           |
|                     |        | X                     | Left-most pixel position                                                                                  |
|                     |        | Y                     | Top-most pixel position                                                                                   |
|                     |        | WIDTH                 | Frame width in pixels                                                                                     |
|                     |        | HEIGHT                | Frame height in pixels                                                                                    |
| CCD_TEMPERATURE     | Number | CCD_TEMPERATURE_VALUE | CCD chip temperature in degrees Celsius                                                                   |
| CCD_COOLER          | Switch |                       | CCD Cooler control.                                                                                       |
|                     |        | COOLER_ON             | Turn cooler on                                                                                            |
|                     |        | COOLER_OFF            | Turn cooler off                                                                                           |
| CCD_FRAME_TYPE      | Switch |                       |                                                                                                           |
|                     |        | FRAME_LIGHT           | Take a light frame exposure                                                                               |
|                     |        | FRAME_BIAS            | Take a bias frame exposure                                                                                |
|                     |        | FRAME_DARK            | Take a dark frame exposure                                                                                |
|                     |        | FRAME_FLAT            | Take a flat field frame exposure                                                                          |
| CCD_BINNING         | Number |                       |                                                                                                           |
|                     |        | HOR_BIN               | Horizontal binning                                                                                        |
|                     |        | VER_BIN               | Vertical binning                                                                                          |
| CCD_COMPRESSION     | Switch |                       |                                                                                                           |
|                     |        | CCD_COMPRESS          | Compress CCD frame (If FITS, it uses fpack to send a .fz file)                                            |
|                     |        | CCD_RAW               | Send raw CCD frame                                                                                        |
| CCD_FRAME_RESET     | Switch | RESET                 | Reset CCD frame to default X,Y,W, and H settings. Set binning to 1x1.                                     |
| CCD_INFO            | Number |                       |                                                                                                           |
|                     |        | CCD_MAX_X             | Resolution x                                                                                              |
|                     |        | CCD_MAX_Y             | Resolution y                                                                                              |
|                     |        | CCD_PIXEL_SIZE        | CCD pixel size in microns                                                                                 |
|                     |        | CCD_PIXEL_SIZE_X      | Pixel size X, microns                                                                                     |
|                     |        | CCD_PIXEL_SIZE_Y      | Pixel size Y, microns                                                                                     |
|                     |        | CCD_BITSPERPIXEL      | Bits per pixel                                                                                            |
| CCD_CFA             | Text   |                       | Color Filter Array information if the CCD produces a bayered image. Debayering performed at client side.  |
|                     |        | CFA_OFFSET_X          | CFA X Offset.                                                                                             |
|                     |        | CFA_OFFSET_Y          | CFA Y Offset.                                                                                             |
|                     |        | CFA_TYPE              | CFA Filter type (e.g. RGGB).                                                                              |
| CCD1                | BLOB   |                       | CCD1 for primary CCD, CCD2 for guider CCD.                                                                |
|                     |        | CCD1                  | Binary fits data encoded in base64. The CCD1.format is used to indicate the data type (e.g. ".fits")      |
| CCD_TEMP_RAMP       | Number |                       | Set TEC cooler ramp parameters. The ramp is software controlled inside INDI.                              |
|                     |        | RAMP_SLOPE            | Maximum temperature change in degrees Celsius per minute                                                  |
|                     |        | RAMP_THRESHOLD        | Threshold in degrees celsius. If the absolute difference of target and current temperature equals to or   |
|                     |        |                       | below this threshold, then the cooling operation is complete.                                             |
| WCS_CONTROL         | Switch |                       | Toggle World Coordinate System keyword inclusion in FITS Header.                                          |
|                     |        | WCS_ENABLE            |                                                                                                           |
|                     |        | WCS_DISABLE           |                                                                                                           |
| CCD_ROTATION        | Number |                       | Camera field of view rotation measured as East of North in degrees.                                       |
|                     |        | CCD_ROTATION_VALUE    |                                                                                                           |
| CCD_CAPTURE_FORMAT  | Switch |                       | Raw capture format as supported by the driver or hardware (e.g. Bayer 16bit or RGB)                       |
| CCD_TRANSFER_FORMAT | Switch |                       | Transfer format of the raw captured format before sending the image back to the client or saving to disk. |
|                     |        | FORMAT_FITS           | Encode captured image as FITS                                                                             |
|                     |        | FORMAT_NATIVE         | Send image as-is                                                                                          |
|                     |        | FORMAT_XISF           | Encode captured images as XISF                                                                            |
| CCD_FILE_PATH       | Text   | FILE_PATH             | Absolute path where images is saved on disk                                                               |
| CCD_FAST_TOGGLE     | Switch |                       | Fast Exposure is used to enable camera to immediately begin capturing the next frames                     |
|                     |        | INDI_ENABLED          | Enable fast exposure                                                                                      |
|                     |        | INDI_DISABLED         | Disable fast exposure                                                                                     |
| CCD_FAST_COUNT      | Number | FRAMES                | Number of fast exposure frames to capture once exposure begins                                            |
| FITS_HEADER         | Text   |                       | Name, value, and comment row to be appended to the fits header on the next capture. The row needs to be   |
|                     |        |                       | set once for any subsequent captures. It is not retained on driver restart.                               |
|                     |        | KEYWORD_NAME          |                                                                                                           |
|                     |        | KEYWORD_VALUE         |                                                                                                           |
|                     |        | KEYWORD_COMMENT       |                                                                                                           |

Most `CCD_*` properties are also applicable to `GUIDER` chip, so replace `CCD` with `GUIDER` above to define properties for the `GUIDER` chip, if applicable.

## CCD Streaming

Some CCD drivers support streaming and recording of video streams. All the properties defined in `StreamManager` are considered reserved standard properties. The stream data is sent via the same CCD1 BLOB used for image transmission and therefore it is not possible to stream and send regular CCD captures at the same time.

| Name                | Type   | Values                   | Description                                                                            |
| ------------------- | ------ | ------------------------ | -------------------------------------------------------------------------------------- |
| CCD_VIDEO_STREAM    | Switch | STREAM_ON                | Turn on video stream                                                                   |
|                     |        | STREAM_OFF               | Turn off video stream                                                                  |
| STREAM_DELAY        | Number | STREAM_DELAY_TIME        | Delay in seconds between frames                                                        |
| STREAMING_EXPOSURE  | Number | STREAMING_EXPOSURE_VALUE | Frame exposure values in seconds when streaming                                        |
|                     |        | STREAMING_DIVISOR_VALUE  | The divisor is used to skip frames as way to throttle the stream down                  |
| FPS                 | Number |                          | Read Only                                                                              |
|                     |        | EST_FPS                  | Instant frame rate                                                                     |
|                     |        | AVG_FPS                  | Average FPS over 1 second                                                              |
| CCD_STREAM_FRAME    | Number |                          | Streaming frame size                                                                   |
|                     |        | X                        | Left-most pixel position                                                               |
|                     |        | Y                        | Top-most pixel position                                                                |
|                     |        | WIDTH                    | Frame width in pixels                                                                  |
|                     |        | HEIGHT                   | Frame height in pixels                                                                 |
| CCD_STREAM_ENCODER  | Switch |                          | Encoder used for streaming                                                             |
|                     |        | RAW                      | Raw encoder                                                                            |
|                     |        | MJPEG                    | MJPEG encoder                                                                          |
| CCD_STREAM_RECORDER | Switch |                          | Recorder used for saving streams                                                       |
|                     |        | SER                      | SER recorder                                                                           |
|                     |        | OGV                      | OGV recorder                                                                           |
| LIMITS              | Number |                          | Streaming limits                                                                       |
|                     |        | LIMITS_BUFFER_MAX        | Maximum buffer size in MB                                                              |
|                     |        | LIMITS_PREVIEW_FPS       | Maximum preview FPS                                                                    |
| RECORD_FILE         | Text   | RECORD_FILE_DIR          | Directory to save the file. It defaults to $HOME/indi\__D_                             |
|                     |        | RECORD_FILE_NAME         | Recording file name. It defaults to indi*record\_\_T*                                  |
| RECORD_OPTIONS      | Number |                          | Set the desired duration in seconds or total frames required for the recording         |
|                     |        | RECORD_DURATION          | Duration in seconds                                                                    |
|                     |        | RECORD_FRAME_TOTAL       | Total number of frames required                                                        |
| RECORD_STREAM       | Switch |                          | Start or Stop the stream recording to a file                                           |
|                     |        | RECORD_ON                | Start recording. Do not stop unless asked to                                           |
|                     |        | RECORD_DURATION_ON       | Start recording until the duration set in `RECORD_OPTIONS` has elapsed                 |
|                     |        | RECORD_FRAME_ON          | Start recording until the number of frames set in `RECORD_OPTIONS` has been captured   |
|                     |        | RECORD_OFF               | Stops recording                                                                        |
| CCD_FAST_TOGGLE     | Switch | INDI_ENABLED             | Fast Exposure is used to enable camera to immediately begin capturing the next frames. |
|                     |        | INDI_DISABLED            |                                                                                        |
| CCD_FAST_COUNT      | Number | FRAMES                   | Number of fast exposure captured to take once capture begins.                          |

### Notes

`STREAMING_EXPOSURE_VALUE` is advisory only as some streaming devices cannot control the exposure duration.

`STREAMING_DIVISOR_VALUE` is used to skip frames. By default, a divisor value of 1 does not skip any frames. A value of two would skip every other frame (thereby cutting the FPS in half). A frame is skipped when the number of captured frames % divisor is equal to zero.

`RECORD_FILE` Recorders are responsible for recording the video stream to a file. The recording file directory and name can be set via the RECORD_FILE property which is composed of RECORD_FILE_DIR and RECORD_FILE_NAME elements. You can specify a record directory name together with a file name. You may use special character sequences to generate dynamic names:

- _D_ is replaced with the date ('YYYY-MM-DD')
- _H_ is replaced with the time ('hh-mm-ss')
- _T_ is replaced with a timestamp
- _F_ is replaced with the filter name currently in use

`FAST EXPOSURE` Fast Exposure is used to enable camera to immediately begin capturing the next frame once the previous frame data is downloaded from the camera. With Fast Exposure disabled, the driver have to wait until the client initiates the next capture request. To minimize the downtime, Fast Exposure can be enabled to trigger for a specific number of frame in Fast Exposure Count property.

Once the initial capture is started, the driver would continue to capture all necessary frames without waiting for client until the count reaches zero. The Fast Exposure Count is decremented after each fast exposure is complete.

Upload Mode affects Fast Exposure behavior depending on the mode selected. For LOCAL mode, where images are saved locally to disk, Fast Exposure is most efficient. For CLIENT or BOTH modes, the driver have to transmit the data over network to the client. If the upload time exceeds the requested exposure time, then Fast Exposure cannot reliably work and the driver would abort the exposure.

## Filter Wheel Properties

| Name        | Type   | Values            | Description                                                                         |
| ----------- | ------ | ----------------- | ----------------------------------------------------------------------------------- |
| FILTER_SLOT | Number | FILTER_SLOT_VALUE | The filter wheel's current slot number. Important: Filter numbers start from 1 to N |
| FILTER_NAME | Text   | FILTER_NAME_VALUE | The filter wheel's current slot name                                                |

## Focuser Properties

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

## Dome Properties

| Name                     | Type   | Values                 | Description                                                                                              |
| ------------------------ | ------ | ---------------------- | -------------------------------------------------------------------------------------------------------- |
| DOME_SPEED               | Number | DOME_SPEED_VALUE       | Set dome speed in RPM.                                                                                   |
| DOME_MOTION              | Switch | DOME_CW                | Move dome Clockwise, looking down                                                                        |
|                          |        | DOME_CCW               | Move dome counter clockwise, looking down                                                                |
| DOME_TIMER               | Number | DOME_TIMER_VALUE       | Move the dome in the direction of `DOME_MOTION` at rate `DOME_SPEED` for `DOME_TIMER_VALUE` milliseconds |
| REL_DOME_POSITION        | Number | DOME_RELATIVE_POSITION | Move `DOME_RELATIVE_POSITION` degrees azimuth in the direction of `DOME_MOTION`.                         |
| ABS_DOME_POSITION        | Number | DOME_ABSOLUTE_POSITION | Move dome to `DOME_ABSOLUTE_POSITION` absolute azimuth angle in degrees.                                 |
| DOME_ABORT_MOTION        | Switch | ABORT                  | Abort dome motion.                                                                                       |
| DOME_SHUTTER             | Switch | SHUTTER_OPEN           | Open dome shutter.                                                                                       |
|                          |        | SHUTTER_CLOSE          | Close dome shutter.                                                                                      |
| DOME_GOTO                | Switch | DOME_HOME              | Go to home position.                                                                                     |
|                          |        | DOME_PARK              | Go to park position.                                                                                     |
| DOME_PARAMS              | Number | HOME_POSITION          | Dome home position in absolute degrees azimuth.                                                          |
|                          |        | PARK_POSITION          | Dome parking position in absolute degrees azimuth.                                                       |
|                          |        | AUTOSYNC_THRESHOLD     | Threshold in degrees for dome auto-sync                                                                  |
| DOME_AUTOSYNC            | Switch | DOME_AUTOSYNC_ENABLE   | Enable dome slaving.                                                                                     |
|                          |        | DOME_AUTOSYNC_DISABLE  | Disable dome slaving.                                                                                    |
| DOME_MEASUREMENTS        | Number | DM_DOME_RADIUS         | Dome radius (m).                                                                                         |
|                          |        | DM_SHUTTER_WIDTH       | Shutter width (m).                                                                                       |
|                          |        | DM_NORTH_DISPLACEMENT  | North displacement (m).                                                                                  |
|                          |        | DM_EAST_DISPLACEMENT   | East displacement (m).                                                                                   |
|                          |        | DM_UP_DISPLACEMENT     | Up displacement (m).                                                                                     |
|                          |        | DM_OTA_OFFSET          | OTA offset (m).                                                                                          |
| DM_OTA_SIDE              | Switch |                        | Meridian side                                                                                            |
|                          |        | DM_OTA_SIDE_EAST       | East                                                                                                     |
|                          |        | DM_OTA_SIDE_WEST       | West                                                                                                     |
|                          |        | DM_OTA_SIDE_MOUNT      | Mount                                                                                                    |
|                          |        | DM_OTA_SIDE_HA         | Hour Angle                                                                                               |
|                          |        | DM_OTA_SIDE_IGNORE     | Ignore                                                                                                   |
| DOME_SYNC                | Number | DOME_SYNC_VALUE        | Sync dome to this azimuth value                                                                          |
| DOME_PARK                | Switch | PARK                   | Park the dome                                                                                            |
|                          |        | UNPARK                 | Unpark the dome                                                                                          |
| DOME_PARK_POSITION       | Number | PARK_AZ                | Park position in azimuth degrees or encoder ticks                                                        |
| DOME_PARK_OPTION         | Switch | PARK_CURRENT           | Use current position as park position                                                                    |
|                          |        | PARK_DEFAULT           | Use default park position                                                                                |
|                          |        | PARK_WRITE_DATA        | Write park data to file                                                                                  |
| DOME_SHUTTER_PARK_POLICY | Switch | SHUTTER_CLOSE_ON_PARK  | Close shutter when parking                                                                               |
|                          |        | SHUTTER_OPEN_ON_UNPARK | Open shutter when unparking                                                                              |
| MOUNT_POLICY             | Switch | MOUNT_IGNORED          | Ignore mount status when parking/unparking                                                               |
|                          |        | MOUNT_LOCKS            | Prevent dome from unparking when mount is parked                                                         |
| DOME_BACKLASH_TOGGLE     | Switch | INDI_ENABLED           | Enable backlash compensation                                                                             |
|                          |        | INDI_DISABLED          | Disable backlash compensation                                                                            |
| DOME_BACKLASH_STEPS      | Number | DOME_BACKLASH_VALUE    | Backlash steps                                                                                           |

## Input Interface Properties

| Name                 | Type   | Values           | Description               |
| -------------------- | ------ | ---------------- | ------------------------- |
| DIGITAL_INPUT_LABELS | Text   |                  | Labels for digital inputs |
|                      |        | DIGITAL*INPUT*\* | Label for digital input # |
| ANALOG_INPUT_LABELS  | Text   |                  | Labels for analog inputs  |
|                      |        | ANALOG*INPUT*\*  | Label for analog input #  |
| DIGITAL*INPUT*\*     | Switch |                  | Digital input state       |
|                      |        | OFF              | Input is off              |
|                      |        | ON               | Input is on               |
| ANALOG*INPUT*\*      | Number | ANALOG*INPUT*\*  | Analog input value        |

## Output Interface Properties

| Name                  | Type   | Values            | Description                                         |
| --------------------- | ------ | ----------------- | --------------------------------------------------- |
| DIGITAL_OUTPUT_LABELS | Text   |                   | Labels for digital outputs                          |
|                       |        | DIGITAL*OUTPUT*\* | Label for digital output #                          |
| DIGITAL*OUTPUT*\*     | Switch |                   | Digital output control                              |
|                       |        | OFF               | Turn output off                                     |
|                       |        | ON                | Turn output on                                      |
| PULSE\_\*             | Number | DURATION          | Pulse duration in milliseconds for digital output # |

## Light Box Interface Properties

| Name                        | Type   | Values                     | Description                      |
| --------------------------- | ------ | -------------------------- | -------------------------------- |
| FLAT_LIGHT_CONTROL          | Switch |                            | Control the light box            |
|                             |        | FLAT_LIGHT_ON              | Turn flat light on               |
|                             |        | FLAT_LIGHT_OFF             | Turn flat light off              |
| FLAT_LIGHT_INTENSITY        | Number | FLAT_LIGHT_INTENSITY_VALUE | Light intensity value (0-255)    |
| FLAT_LIGHT_FILTER_INTENSITY | Number |                            | Filter-specific intensity values |

## GPS Interface Properties

| Name               | Type   | Values            | Description                   |
| ------------------ | ------ | ----------------- | ----------------------------- |
| GPS_REFRESH_PERIOD | Number | PERIOD            | GPS refresh period in seconds |
| GPS_REFRESH        | Switch | REFRESH           | Refresh GPS data              |
| GEOGRAPHIC_COORD   | Number |                   | Geographic coordinates        |
|                    |        | LAT               | Latitude in degrees, +N       |
|                    |        | LONG              | Longitude in degrees, +E      |
|                    |        | ELEV              | Elevation in meters           |
| SYSTEM_TIME_UPDATE | Switch |                   | System time update policy     |
|                    |        | UPDATE_NEVER      | Never update system time      |
|                    |        | UPDATE_ON_STARTUP | Update system time on startup |
|                    |        | UPDATE_ON_REFRESH | Update system time on refresh |
| TIME_UTC           | Text   |                   | UTC time information          |
|                    |        | UTC               | UTC time                      |
|                    |        | OFFSET            | UTC offset                    |

## Weather Interface Properties

| Name               | Type   | Values   | Description                            |
| ------------------ | ------ | -------- | -------------------------------------- |
| WEATHER_UPDATE     | Number | PERIOD   | Weather update period in seconds       |
| WEATHER_REFRESH    | Switch | REFRESH  | Refresh weather data                   |
| WEATHER_OVERRIDE   | Switch | OVERRIDE | Override weather status                |
| WEATHER_PARAMETERS | Number |          | Weather parameters (varies by device)  |
| WEATHER_STATUS     | Light  |          | Weather status for critical parameters |

## Rotator Interface Properties

| Name                    | Type   | Values                 | Description                           |
| ----------------------- | ------ | ---------------------- | ------------------------------------- |
| ABS_ROTATOR_ANGLE       | Number | ANGLE                  | Absolute rotator angle in degrees     |
| ROTATOR_ABORT_MOTION    | Switch | ABORT                  | Abort rotator motion                  |
| SYNC_ROTATOR_ANGLE      | Number | ANGLE                  | Sync rotator to this angle in degrees |
| ROTATOR_HOME            | Switch | HOME                   | Home the rotator                      |
| ROTATOR_REVERSE         | Switch |                        | Reverse rotator direction             |
|                         |        | INDI_ENABLED           | Enable reverse                        |
|                         |        | INDI_DISABLED          | Disable reverse                       |
| ROTATOR_BACKLASH_TOGGLE | Switch |                        | Toggle backlash compensation          |
|                         |        | INDI_ENABLED           | Enable backlash compensation          |
|                         |        | INDI_DISABLED          | Disable backlash compensation         |
| ROTATOR_BACKLASH_STEPS  | Number | ROTATOR_BACKLASH_VALUE | Backlash steps                        |
| ROTATOR_LIMITS          | Number | ROTATOR_LIMITS_VALUE   | Maximum range in degrees              |
