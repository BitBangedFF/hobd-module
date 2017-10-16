# hobd-module
CBR Hacks Project - HOBD Module

On board diagnostics module for the CBR1000 RR (2005) motorcycle.

Hardware: at90can128 (AVR-CAN from Olimex)

## WARNING
The information here is **not** guaranteed to be accurate!

It's just what I have discovered by exploring and could be completely wrong!

TODO:

  * Do I need any pullups on the Rx/Tx <-> K-line?
  * Document the protocol, similar to ISO 9141-2
  * Make this readme better, links to docs/diagrams/etc
  * Talk about hw setup, k-line, converter, etc
  * Hook up diagnostics/error-reporting (like in the USART ISRs)
  * Figure out how to use the new hw MC33660EF

## HOBD (Honda On-Board Diagnostic) Protocol
The ECU on the motorcycle has a K-line wire brought out through the Data Link Connector.

As far as I can tell, this is similar to ISO 9141 (bi-directional half-duplex ISO K line)
but with a different message protocol.

  - three byte header (type, total size, subtype)
  - zero or more payload bytes
  - one byte checksum

### Initialization Sequence

  - pull K-line low for 70 ms
  - return K-line to high state
  - wait 120 ms
  - send wake up message (no response expected)
  - send initialization message (ECU responds)
  - ECU will now respond to data queries
  - ECU will timeout after 120 ms of inactivity (repeat initialization sequence)

### MC33660EF K-line Interface
TODO

## CAN Protocol
TODO

## Simulator
TODO