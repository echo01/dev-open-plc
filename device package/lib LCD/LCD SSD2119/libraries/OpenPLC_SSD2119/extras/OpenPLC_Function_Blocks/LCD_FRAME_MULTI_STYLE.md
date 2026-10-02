# Multi-style LCD Frame Function Blocks

Keep each FB rung enabled from the left power rail. Use one one-scan
`LCD_UPDATE_PULSE` on the explicit `EXEC` input of every block. Place the FBs
in this exact order:

```text
Rung 1: TRUE rung -> LCD_FRAME_BEGIN(EXEC := LCD_UPDATE_PULSE)
Rung 2: TRUE rung -> LCD_FRAME_TEXT instance 1(EXEC := LCD_UPDATE_PULSE)
Rung 3: TRUE rung -> LCD_FRAME_TEXT instance 2(EXEC := LCD_UPDATE_PULSE)
Rung 4: TRUE rung -> LCD_FRAME_TEXT instance 3(EXEC := LCD_UPDATE_PULSE)
Rung 5: TRUE rung -> LCD_FRAME_PRESENT(EXEC := LCD_UPDATE_PULSE)
```

Gate the update pulse while the previous frame is flushing:

```text
REQUEST_1S AND NOT LCD_PRESENT_BUSY -> LCD_UPDATE_PULSE
```

Example values:

| Instance | X | Y | TEXT | FONT_SIZE | FONT_COLOR | BG_COLOR | CLEAR_BG |
|---|---:|---:|---|---:|---:|---:|---|
| Begin | - | - | - | - | - | `16#0000` | `TRUE` |
| Text 1 | 10 | 45 | `'OPENPLC'` | 24 | `16#FBA1` | `16#0000` | `FALSE` |
| Text 2 | 10 | 90 | `'RUNNING'` | 18 | `16#FFFF` | `16#0000` | `FALSE` |
| Text 3 | 170 | 140 | `'PV=25.4'` | 12 | `16#07E0` | `16#0000` | `FALSE` |
| Text 4 | 10 | 210 | `'ALARM'` | 24 | `16#F800` | `16#0000` | `FALSE` |

`Y` is the font baseline. Supported font mapping is 12, 18, and 24. Colors are RGB565.

Do not add `EN`, `LAST_EN`, or `ACTIVE` to the FB variable tables. STruC++
treats `EN` as the IEC execution-control pin and emits C++ FB Local variables
at file scope, which can cause duplicate definitions. Declare `EXEC` as an
Input BOOL instead. Begin and Text are stateless; Present contains its private
flush state and should have one instance per LCD.

`LCD_FRAME_BEGIN(CLEAR=TRUE)` clears RAM only. The physical SSD2119 keeps
showing the previous frame until `LCD_FRAME_PRESENT` is accepted and
`OpenPLC_LCD_Task()` flushes the new frame.