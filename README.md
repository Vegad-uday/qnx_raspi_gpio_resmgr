# Raspberry Pi GPIO for QNX (BCM2711) — `raspi_gpio_resmgr`

A minimal **QNX utility** to configure and drive Raspberry Pi 4 (BCM2711) GPIO pins via **memory‑mapped I/O**. This implementation supports **GPIO pins 2–27 only**, with **basic input/output** direction and **digital high/low** control.

> Note: The current code operates as a small command‑line tool (not a full device-node service). It maps the BCM2711 GPIO registers and applies the requested operation, then exits.

## Files
- `raspi_gpio_resmgr.c` — Command-line program that maps GPIO, selects pin function, and optionally sets its level.
- `bcm2711.h` — BCM2711 register definitions (e.g., `BCM2711_GPIO_BASE = 0xfe200000`).

## Build in Momentics IDE (QNX SDP)
1. **File → New → QNX C Project** (e.g., `raspi_gpio_resmgr`).
2. Remove the auto‑generated `main.c` (if any).
3. Add files:
   - Copy `raspi_gpio_resmgr.c` into `src/`.
   - Copy `bcm2711.h` into `include/` (or alongside the `.c` file and adjust includes).
4. **Project → Build Project** to produce the `raspi_gpio_resmgr` executable.

### Add program arguments (Momentics)
- **Run → Run Configurations… → C/C++ Application → (your target)**  
  Enter command‑line args under **Program arguments** (see *Usage* below).

## Deploy & Run on Raspberry Pi (QNX)
1. Boot your Raspberry Pi 4 with a **QNX image**.
2. Copy the binary to the target (e.g., with `scp`):  
   `scp ./raspi_gpio_resmgr root@<raspi-ip>:/usr/bin/`
3. On target:
   ```sh
   chmod +x /usr/bin/raspi_gpio_resmgr
   /usr/bin/raspi_gpio_resmgr <args...>
   ```
> You typically need **root** privileges because the program uses `mmap_device_memory` to access physical registers.

## Usage (command‑line)
The program expects the **`set`** subcommand and four arguments in total:

```
raspi_gpio_resmgr set <pin> <op|ip> <o1|o0|nc>
```

- `<pin>` — integer **2..27** only (validated by the program).
- `<op>` — configure as **output**; requires `<o1|o0>` to immediately drive **high** (`o1`) or **low** (`o0`).
- `<ip>` — configure as **input**; **do not** pass `o1` or `o0` (the program will error). Use any placeholder like `nc` (no-change).

### Examples
```sh
# Set GPIO17 as output and drive HIGH
./raspi_gpio_resmgr set 17 op o1

# Set GPIO17 as output and drive LOW
./raspi_gpio_resmgr set 17 op o0

# Configure GPIO4 as INPUT (no driving)
./raspi_gpio_resmgr set 4 ip nc
```

### Return codes & messages
- Invalid first argument → `ERROR : Argument 1 <val> is Invalid`
- Pin out of range (2–27) → `ERROR : Argument 2 <val> is Invalid - Try Again`
- Invalid direction → `ERROR : Argument 3 is Invalid Try Again`
- Invalid level for output → `ERROR : Argument 4 is Invalid - Try Again`
- Attempting to set level when in input mode → `ERROR : YOU can't set GPIO as HIGH OR LOW`
- Memory map failure → `ERROR :GPIO Memory map failed` (returns `-1`)

## Technical notes
- Maps **`BCM2711_GPIO_BASE` (0xFE200000)** with 4 KB block size.
- Uses QNX **`mmap_device_memory`** and accesses registers directly.
- Only the **function select** (input/output) and **set/clear** (drive high/low) operations are implemented in this version.

## Limitations
- Supports **GPIO 2–27** only.
- **Input/Output** direction and **High/Low** drive are supported; **no** alternate functions or interrupts yet.
- Not a daemon/service; no `/dev` device node is created in this version.

## Quick hardware test
- Wire an LED + resistor to a supported pin (e.g., GPIO17) and to **GND**.
- Run:
  ```sh
  ./raspi_gpio_resmgr set 17 op o1   # LED on
  ./raspi_gpio_resmgr set 17 op o0   # LED off
  ```

---
