# GPSDO
	A GPSDO built around readily available hardware

### Current Status
	The GPSDO is a work in progress (Jan 2024). It's the first time I'm working with the AVR-Dx family, and it's looking very promissing.
	
### The GPSDO working principle
	I've pulled together a number of designs, and ignored a lot of forum comments. The design uses a NEO-M8N GPS as PPS source, the generated pulse is configurable contrary to some sources. The OCXO is a cheap one. I'm using the Time Integration method (TIC) to tune the crystal.
