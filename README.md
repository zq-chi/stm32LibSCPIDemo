# stm32LibSCPIDemo

stm32f103 + freeRTOS + uart + libscpi

libscpi repo : <https://github.com/j123b567/scpi-parser>

another example : <https://github.com/eez-open/modular-psu-firmware>

## Test Result

```BASH
 * Connected to: ASRL2::INSTR
-> *IDN?
<- MANUFACTURE,INSTR2013,0,01-02
-> SYST:ERR?
<- 0,"No error"
-> *TST?
<- 0
 * Read Status Byte:
<- 0x00, (0)
-> test
-> SYST:ERR?
<- -113,"Undefined header"
-> SYST:ERR?
<- 0,"No error"
-> *IDN?
<- MANUFACTURE,INSTR2013,0,01-02
```
