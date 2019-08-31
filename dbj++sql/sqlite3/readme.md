
After downloading sqlite3.dll please created the lib with the 
following command for 64 bit machines
```
lib /def:sqlite3.def /MACHINE:X64
```
Of course omitt the `MACHINE` switch for X86 aka 32 bit machines.