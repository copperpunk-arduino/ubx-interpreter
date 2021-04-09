# UbxInterpreter
This a generic UBX interpreter library. It expects the creation of another, more specific, class in order to process any messages that are received. See the [ubx-gps](https://github.com/copperpunk-arduino/ubx-gps) repository for an example of this.

> All examples are written to work with [Adafruit Feather M0](https://www.adafruit.com/product/2772) boards. That doesn't mean they don't work with other boards, but you might need to change some of the hardware definitions.

The [CreateMessage](https://github.com/copperpunk-arduino/ubx-interpreter/tree/main/examples/CreateMessage) example shows what a valid UBX message should look like. Only the USB serial port is required.

The [LoopbackMessage](https://github.com/copperpunk-arduino/ubx-interpreter/tree/main/examples/LoopbackMessage) example requires the TX and RX pins of your board to be connected. SAMD boards are great for this, because the USB and `Serial1` ports are physically separate. Please define `debug_port` according to your board's USB serial port name.<br>A valid UBX messages will be sent over `Serial1` and it will be parsed. The results will be sent to the USB serial port. This example demonstrates how to use the `read` function, which will call `parse` internally.

The [DebugParseState](https://github.com/copperpunk-arduino/ubx-interpreter/tree/main/examples/DebugParseState) example expects the same wiring as LoopbackMessage. This provides more verbose debug output, and shows how the `parse` function is used. This can be useful if you are having issues parsing a message correctly. 
<br><br>
## Contact us
If you have any suggestions for improving this library, there are a few ways to get in touch:<br>

*   Create a new issue
*   Submit a pull request
*   Virtual meeting using our [Open Source Office Hours](https://www.copperpunk.com/service-page/open-source-office-hours)
