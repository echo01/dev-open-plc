# Proposed MX25L6433F Connection

Status: analysis only, 2026-10-01. No firmware or hardware changes tested.

For NUCLEO-H563ZI MB1404, prefer the grouped CN10 QSPI signals:

| Signal | MCU | CN10 pin |
|---|---|---|
| NCS | PG6 | 13 |
| CLK | PB2 | 15 |
| IO3 | PD13 | 19 |
| IO1 | PD12 | 21 |
| IO0 | PD11 | 23 |
| IO2 | PE2 | 25 |
| GND | GND | 17 or 27 |

These GPIOs are not allocated in the current IOC. PF7/PF8/PF9 are assigned
to keys; do not use the alternative PF6-PF10 group without moving keys.
Retain all existing LCD pins.

Check actual board routing before wiring: UM3115 specifies SB61 ON/SB66 OFF
for PB2 to CN10-15, and SB70 ON for PE2 to CN10-25. Review PE2 trace/SAI
routes and disable conflicting use. Check header pin-1 orientation and
continuity with power disconnected. Do not assume solder bridge state.

Use 3.3 V, local decoupling, short wiring, and CS pull-up. IO2/IO3 are data
signals in quad mode; never strap them directly to VCC or GND.

Bring-up configuration: OCTOSPI1 regular-protocol NOR, single device,
quad IO0-IO3, SDR, clock mode 0, no DQS or free-running clock, no wrap.
Capacity is 64 Mbit / 8 MiB (DEVSIZE encoding 22), command address width
24 bits. Start at about 10 MHz actual SCK. At a verified 180 MHz kernel,
this requires divide-by-18; confirm CubeMX/HAL prescaler encoding rather
than assuming the field is the divisor. Start CS-high time at 4 cycles,
FIFO threshold at 4 bytes, sample shift none, delay-block bypass enabled;
these are bring-up proposals, not measured timing validation.

First read JEDEC ID using single-line commands, then erase/program/read
only a reserved test sector. Enable and verify the flash QE bit using
read-modify-write, preserving protection bits. Configure a supported quad
read opcode and its exact address/data widths and dummy cycles from the
Macronix datasheet. Do not enable octal or DTR commands for this chip.
Only enable memory mapping after indirect reads pass. Initialize before
TouchGFX accesses assets. A matching loader and linker mapping are also
required; CubeMX peripheral initialization alone is insufficient.

Sources:
- ST UM3115, CN10 pinout and solder bridges: https://www.st.com/resource/en/user_manual/dm00936683.pdf
- Readable copy of ST UM3115 Rev 1: https://device.report/m/41b6eb85350ac97670dfef8b806be5533d04afc28ad8e343b18fd22c337fead5
- STM32H563 datasheet: https://www.st.com/resource/en/datasheet/stm32h563zi.pdf
- ST XSPI HAL: https://github.com/STMicroelectronics/stm32h5xx-hal-driver/blob/main/Inc/stm32h5xx_hal_xspi.h
