# Music Business Simulator

A C prototype that grew out of exercises in Boolean logic and probability. It
reports metrics for three fictional tracks, forecasts next-week streams and
revenue, and simulates 10,000 concert opportunities.

## Build and run on Windows

Use GCC from MinGW-w64 in PowerShell:

```powershell
gcc -Wall -Wextra -std=c11 -fexec-charset=UTF-8 main.c -o main.exe
.\main.exe
```

Enter manager loyalty from 0 to 10 to run the simulation. The data are fictional
and the results vary between runs. Contract negotiations and label budgets are
planned.
