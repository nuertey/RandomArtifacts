# Description
BAT-P3 Emulator

The BAT-P3 is an 8 cell Lithium-Ion battery system designed for battery 
life-time, easy integration, and safety. The battery configuration can 
either be 4s2p or 8s1p providing 92 Wh in nominal capacity. The BAT-P3 
is both flexible enough and sufficiently powerful for most nano- and 
small-satellite missions.
 
The automatic balancing circuit maximizes battery lifetime, and the 
automatic heater ensures optimal operational battery environment at all 
times. Short-circuit and over/under voltage detection circuits protects
the batteries from damage. To accommodate different launch vehicle 
requirements, each module has connectors for both soft and hard inhibits.
 
The BAT-P3 uses eight Panasonic NCR18650B cells that have proven themselves
to be very reliable in several space missions.

In this software emulation of the BAT-P3's functionalities, we will take
advantage of the expressiveness of the Boost.MSM State Machine functionality,
and the nimbleness of various software timers, to mimic exactly how the 
BAT-P3 behaves in hardware. Thus begings then our journey into the code 
and logic of the BCUEmulator, the Battery Emulator.  

## List of Source Files/Documentation
```
.
├── BatteryEmulator.cpp
├── BatteryEmulator.hpp
├── build_script.sh
├── custom-raspbian.ini
├── custom-ubuntu.ini
├── doc
│   ├── BCUEmulator_Pi_11-20-2020_test_run-2.txt
│   ├── BCUEmulator_Pi_11-20-2020_test_run.txt
│   ├── BCUEmulator_Pi_11-21-2020_success_run.txt
│   ├── BCUEmulator_Pi_12-02-2020_With_FC_Driver_Code.txt
│   ├── BCUEmulator_StateMachine.png
│   ├── BCUEmulator_StateMachine-test_run_output.txt
│   ├── BCUEmulator_StateMachine-UML_Code.txt
│   ├── BCUEmulator_Ubuntu_10-12-2020_test_run.txt
│   └── BCUEmulator_Ubuntu_10-14-2020_test_run.txt
├── IBatteryEmulator.hpp
├── main.cpp
├── meson.build
├── ParameterStorage.cpp
├── ParameterStorage.hpp
├── README.md
└── subprojects
    ├── csp
    ├── generic-emulator
    │   ├── base16.c
    │   ├── base16.h
    │   ├── CANFrameUtilities.hpp
    │   ├── csp_debug_param.c
    │   ├── csp_param.c
    │   ├── Date.hpp
    │   ├── GenericTimerHandler.h
    │   ├── Logger.cpp
    │   ├── Logger.hpp
    │   ├── param_config.h
    │   ├── ParameterServer.cpp
    │   ├── ParameterServer.hpp
    │   ├── param_sniffer.c
    │   ├── param_sniffer.h
    │   ├── prometheus.c
    │   ├── prometheus.h
    │   ├── randutils.hpp
    │   ├── slash_csp.c
    │   ├── stdbuf_mon.c
    │   ├── Threading.hpp
    │   ├── time.c
    │   ├── UtilityCollections.cpp
    │   └── UtilityCollections.hpp

    ├── param
    ├── slash
    └── spdlog.wrap
```

## How To Build (And Invoke Executable) On Raspbian (Raspberry Pi OS)
A convenience script, build_script.sh, has been provided you which you 
can simply invoke and all these commands would be automated for you.

However, it is instructive to see what the meson build frontend and the
ninja build backend is doing so it behooves us to learn to invoke the
individual commands. Here they are: 

```
# Assuming you have already exported the BOOST include directories (see
# the comments in build_script.sh).
rm -rf build
mkdir build
meson build/
ninja -j3 -C build
cd build
sudo ninja install
cd /usr/local/bin
sudo ./BCUEmulator
```

## How To Build On Ubuntu (Happenstance deemed it that that would be Nuertey Odzeyem's setup at home :) )
```
# Assuming my Ubuntu setup (buntu 16.04.6 LTS, Linux PurpleOS 4.4.0-112-generic, x86_64 GNU/Linux)
# and BOOST install locations as an example:
export BOOST_LIBRARYDIR="/usr/local/lib"
export BOOST_INCLUDEDIR="/usr/local/boost_1_66_0/boost"

rm -rf build
mkdir build
meson build/ --native-file custom-ubuntu.ini
ninja -C build
```

## Logs, Logging and Other Errata
Should you wish to preserve the logs of BCUEmulator's execution, simply pipe it to a file like so:

```
sudo ./BCUEmulator > /home/pi/repos/bcu-emulator/build/execution_output_driver5.log 
```

## Halting the BCUEmulator
```
# To stop/pause execution:
Ctrl+Z

# And then find out the PID of the suspended process:
ps -eaf |grep BCUEmulator

          PID   PPID
...
root      3046  2494  0 19:50 pts/0    00:00:00 sudo ./BCUEmulator
root      3047  3046 61 19:50 pts/0    00:03:59 ./BCUEmulator

# Now kill only the PID for the first. Killing the PPID will destroy the 
# terminal session itself and you will have to re-login into the Pi and 
# re-export any environment variables you previously exported, such as the
# BOOST include directories. So for example, using the above as a case-study:
sudo kill -9 3046

[1]+  Killed                  sudo ./BCUEmulator > /home/pi/repos/bcu-emulator/build/execution_output_driver5.log
```

Copyright (c) 2020 AST & Science, LLC. All Rights Reserved.
