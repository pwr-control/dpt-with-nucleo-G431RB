# dpt-with-nucleo-G431RB

Firmware for a **double pulse test (DPT)** driven by an STM32 Nucleo-G431RB board. Pressing the user button
fires one sequence of two gate pulses on the switch under test of a half-bridge, the standard test for
measuring the turn-on and turn-off transients (and the switching energies) of a power semiconductor at a
given current and DC-link voltage.

## Test sequence

```
            P1 = 80 µs          gap = 10 µs     P2 = 10 µs
gate  ____|‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|______________|‾‾‾‾‾‾‾|______
          ^ current ramps up   ^ turn-off     ^ turn-on at the set current
```

- **P1** charges the load inductor up to the test current; its end gives the **turn-off** transient.
- During the **gap** the current freewheels in the diode of the complementary switch.
- **P2** gives the **turn-on** transient at practically the same current (and the diode reverse
  recovery), then a second turn-off.

The three times are set in [`fw/Application/Inc/dpt.h`](fw/Application/Inc/dpt.h):

| Define | Default | Meaning |
|---|---|---|
| `P1_TICKS` | `80 * CLOCK_PWM_MHZ` | first pulse, in timer ticks at 170 MHz (80 µs) |
| `GAP_US` | `10` | pause between the pulses, in µs (busy-wait on the DWT cycle counter) |
| `P2_TICKS` | `10 * CLOCK_PWM_MHZ` | second pulse, in timer ticks at 170 MHz (10 µs) |

Size P1 from the load inductance: I_test ≈ V_dc · t_P1 / L.

## How it works

- `TIM1` runs at 170 MHz with no prescaler in one-pulse mode; each pulse is produced by loading the
  auto-reload and compare registers with the pulse length, starting the complementary output `CH1N` and
  waiting for the update flag. Between and after the pulses the output is held in *forced inactive* mode,
  so the pin stays low with low impedance rather than floating (`Run_DoublePulseTest()` in
  [`fw/Application/Src/dpt.c`](fw/Application/Src/dpt.c)).
- `TIM4 CH1` produces a continuous ~1 kHz PWM signal (1 µs tick, period 979 µs, compare 750) used as a
  STO (safe torque off) enable signal, as marked in `main.c`.
- `PC6` and `PC7` (`pwm_top_enable`, `pwm_bottom_enable`) are set high at start-up.
- The test runs once per press of the blue user button (`PC13`), after a 500 ms delay; it re-arms only
  when the button is released.
- ADC1 (channels 1 and 2), DAC1, DMA and the virtual COM port (115200 8N1) are initialised by CubeMX but not
  used by the test yet.

## Pins

| Signal | Pin | Peripheral |
|---|---|---|
| Gate pulse (switch under test) | `PB13` | TIM1_CH1N |
| TIM1_CH1 (configured, not started) | `PC0` | TIM1_CH1 |
| STO / enable PWM | `PA11` | TIM4_CH1 |
| Top / bottom driver enable | `PC6` / `PC7` | GPIO |
| Start button | `PC13` | user button B1 |

## Repository layout

| Path | Content |
|---|---|
| [`fw`](fw) | STM32CubeIDE project (`dpt-with-nucleo-G431RB.ioc`, HAL drivers, linker script); test code in `Application/` |
| [`doc`](doc) | Nucleo-G431RB schematic (MB1367), STM32G4 reference manual (RM0440), adapter board documentation |

## Build and run

1. Open `fw/` in STM32CubeIDE (*File → Open Projects from File System*) and build.
2. Flash the Nucleo through the on-board ST-LINK.
3. Connect `PB13` (and the enables) to the gate-driver input, power the bench and press the blue button
   to fire one double pulse.

Safety: the test switches the full DC-link voltage into an inductive load; keep the bench discharged
while wiring and check the pulse times on a scope, with the power stage off, before the first shot.
