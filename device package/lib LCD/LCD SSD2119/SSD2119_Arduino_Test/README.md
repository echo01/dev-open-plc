# SSD2119 Arduino Test Project

This sketch tests the Arduino port of the SSD2119 driver using `SPI` and `Adafruit_GFX`.

## Library Path

Use the library in:

```text
../libraries/OpenPLC_SSD2119
```

For Arduino IDE, copy `Arduino/libraries/OpenPLC_SSD2119` into your Arduino sketchbook `libraries` folder, or compile with Arduino CLI using `--libraries Arduino/libraries`.

## NUCLEO-H563ZI Pinout

```text
PB5  = SPI MOSI
PA5  = SPI SCK
PG9  = SPI MISO, not used
PF3  = LCD RST
PD15 = LCD DC
PD14 = LCD CS
```

## Dependencies

Install from Arduino Library Manager:

```text
Adafruit GFX Library
```
