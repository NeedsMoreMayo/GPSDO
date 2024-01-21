# GPSDO
A GPSDO built around readily available hardware

## Current Status
The GPSDO is a work in progress (Jan 2024). It's the first time I'm working with the AVR-Dx family, and it's looking very promissing.
	
## The GPSDO working principle
I've pulled together a number of designs, and ignored a lot of forum comments. The design uses a NEO-M8N GPS as PPS source, the generated pulse is configurable contrary to some sources. The OCXO is a cheap one. I'm using the Time Integration method (TIC) to tune the crystal.

The OCXO clock is fed to the uC as external clock. This way the control runs synchronous to the XO and we need to measure 2 events to keep the OCXO in check:
- the number of pulses contained within 1 sec of PPS. We can count this with the TCD or TCB counter.
- the fraction of the XO pulse that runs out of phase with the PPS. In other words, the out-of-phaseness of the PPS with the rest of the system. We measure this with a TIC (Time Interpolation Counter)

The corrections are fed to a 18-bit DAC which corrects the voltage control of the OCXO. The phase of the XO isn't adjusted, and doesn't need to be. It doesn't matter what the phase difference is, if the 1 PPS envelope contains exactly 10M clocks.

### TIC

