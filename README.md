# hobd-module
CBR Hacks Project - HOBD Module

On board diagnostics module for the CBR1000 RR (2005) motorcycle.

## WARNING
The information here is **not** guaranteed to be accurate!

It's just what I have discovered by exploring and could be completely wrong!

## Hardware:

  - at90can128 (AVR-CAN from Olimex)
    - [AVR-CAN reference](vendor/avrcan-at90can128/doc/pdf/AVR-CAN-1.pdf)
    - [at90can128 reference](vendor/avrcan-at90can128/doc/pdf/AT90CAN_chip_ref.pdf)
  - MC33660EF (K-line interface)
    - [chip reference](vendor/mc33660ef/doc/MC33660-1126837.pdf)

## HOBD (Honda On-Board Diagnostic) K-line Protocol
The ECU on the motorcycle has a K-line wire brought out through the Data Link Connector.

As far as I can tell, this is similar to ISO 9141 (bi-directional half-duplex ISO K line)
but with a different message protocol.

Here is the implementation [header file](vendor/hobd/include/hobd_kline.h).

  - three byte header (type, total size, subtype)
  - zero or more payload bytes
  - one byte checksum
  - 10,400 bps

### Initialization Sequence

  - pull K-line low for 70 ms
  - return K-line to high state
  - wait 120 ms
  - send wake up message (no response expected)
  - send initialization message (ECU responds)
  - ECU will now respond to data queries
  - ECU will timeout after 120 ms of inactivity (repeat initialization sequence)

### ECU Data Tables
Here is an evolving table of ECU data tables/registers and known conversions.

  - [ECU data tables pdf](doc/pdf/Honda-data-tables.pdf)

## MC33660EF K-line Interface
The MC33660EF is used to provide an isolated K-line bus interface.

It handles the logic level translation (12v to 5v), the Rx/Tx duplexing and provides some
bus isolation protection features.

## CAN Protocol
The integrated CAN controller is used to provide the ECU data to the rest of the ecosystem over the HOBD CAN bus.

Here is the implementation [header file](vendor/hobd/include/hobd_can.h).

### CAN Messages

Heartbeat message published every 50 milliseconds:

  - HEARTBEAT (0x010 + node_id = 0x015)

Diagnostic messages published every 100 milliseconds:

  - OBD_TIME (0x080)
  - OBD1 (0x081)
  - OBD2 (0x082)
  - OBD3 (0x083)

System status messages published every 1000 milliseconds:

  - HEARTBEAT (0x010 + node_id = 0x015)
  - OBD_UPTIME (0x084)

## Simulator
If you don't have a bench ECU or don't want to hook up to real hardware this project
can be useful for verifying the communication protocol and K-line logic.

It is by no means complete and likely does not behave like a real ECU would.

Hardware:

  - Teensy++ 2.0 from [PJRC](https://www.pjrc.com/store/teensypp.html)
  - MC33660EF
  - K-line connected to another MC33660EF/hobd-module/other-thing