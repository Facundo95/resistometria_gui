# Resistomegria GUI (resistometry)

Purpose
-------

Resistomegria GUI is a small C++ application (FLTK-based) for visualizing and performing simple measurements/plots on resistometry data. It provides a lightweight graphical interface to load CSV data, display simple plots, and (optionally) interface with IEEE-488 (GPIB) hardware when available.

Key functionalities
-------------------

- IEEE-488 (GPIB) hardware support controlled by the `ENABLE_IEEE` Makefile option.
- DUMP simple CSV files containing measurement data and display them as plots.
- Provide measurement utilities implemented in `measurement.cpp` / `measurement.h` for the four points method to calculate resistometry.
- Render plots using the minimal plotting helpers in `simple_plot.cpp` / `simple_plot.h`.
- Graphical user interface implemented in `ui.cpp` / `ui.h` using FLTK.


Build & install (Makefile)
--------------------------

The provided `Makefile` is configured to cross-compile a Windows executable using the MinGW cross-compiler and static FLTK libraries. The default target builds `resistometry.exe`.

Prerequisites (typical Linux host):

- A MinGW cross-compiler (provides `i686-w64-mingw32-g++`). On Debian/Ubuntu you can install the tooling with:

```bash
sudo apt update
sudo apt install mingw-w64
```

- FLTK development libraries for the target (static FLTK Windows libs) and any IEEE import libraries if you enable hardware support.

Build steps

1. Build the Windows executable with the default settings:

```bash
make
```

2. To disable the IEEE/GPIB hardware support (no IEEE library required), run:

```bash
make ENABLE_IEEE=0
```

3. Common make targets:

- `make` or `make all` — build `resistometry.exe`.
- `make clean` — remove the executable and object files.
- `make distclean` — remove build artifacts plus generated data/logs.

Running the program
-------------------

Because the Makefile produces a Windows executable (`resistometry.exe`), you can run it on Linux using Wine:

```bash
wine resistometry.exe
```

Alternatively, copy the produced `resistometry.exe` (and any required DLLs such as `ieee_32m.dll` if used) to a Windows machine and run there.

Makefile configuration notes
---------------------------

- `TARGET` is set to `resistometry.exe` in the Makefile.
- `ENABLE_IEEE` (default `1`) toggles compilation/linking of IEEE/GPIB support. Set `ENABLE_IEEE=0` to skip it.
- `IEEE_LIB_PATH` and `IEEE_LIB_NAME` can be adjusted to point at the IEEE import/static libraries required when `ENABLE_IEEE=1`.
- The Makefile attempts to copy `ieee_32m.dll` next to the final executable if it exists in the project root.

Project structure (important files)
----------------------------------

- `main.cpp` — application entry point.
- `ui.cpp`, `ui.h` — FLTK UI code and window definitions.
- `measurement.cpp`, `measurement.h` — measurement utilities and helpers.
- `simple_plot.cpp`, `simple_plot.h` — simple plotting helper code.
- `Makefile` — build rules and configuration.

License
-------

See the `LICENSE` file in the project root for details.

Questions or contributions
--------------------------

If you'd like help building for a native Linux executable (instead of a Windows cross-build), or to add a packaging workflow, open an issue or ask for assistance.
