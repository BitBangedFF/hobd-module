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

## HOBD (Honda On-Board Diagnostic) Protocol
The ECU on the motorcycle has a K-line wire brought out through the Data Link Connector.

As far as I can tell, this is similar to ISO 9141 (bi-directional half-duplex ISO K line)
but with a different message protocol.

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
TODO

  - [ECU data tables pdf](doc/pdf/Honda-data-tables.pdf)

## MC33660EF K-line Interface
TODO

## CAN Protocol
TODO

## Simulator
TODO