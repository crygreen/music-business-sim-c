# Music Business Simulator

A five-round console campaign in C. You play a music manager and negotiate
contracts across two fictional albums. Each round includes a release proposal
and a chance of a concert. Campaign cash, catalog revenue, followers, artist
rank and the director's trust carry over between rounds.

## Build and run

Windows (GCC from MinGW-w64 in PowerShell):

```powershell
gcc -Wall -Wextra -std=c11 -fexec-charset=UTF-8 main.c -o main.exe
.\main.exe
.\main.exe --demo 34
```

In the interactive campaign, enter 1 for a balanced contract, 2 for a large
advance and a more optimistic report, or 3 for an attempt to divert a rival.
Demo mode plays all five rounds automatically. The numeric seed repeats the
same demo on the same C runtime.

## How decisions work

- The manager earns a fixed salary each round and a commission on signed
  advances and completed concerts.
- The director checks the cash available, artist and manager limits, and the
  projected label profit. Audits and forecast errors affect future trust.
- The artist compares estimated contract pay and independent earnings.
  Immediate expenses increase the appeal of an advance, while longer rights
  make a deal less attractive.
- The director sees the manager's report. The game draws the release result
  using separate probabilities based on the market, audience, promotion and
  random events. The Brier score measures report error after each signed deal.
- An advance and eligible costs are recovered from royalties. Any amount left
  at the end of the rights term stays unrecouped. It is not a personal debt.

The game estimates a week's revenue from historical trends and settles each
contract over its full rights term in the same round. The selected scenario's
weekly revenue stays constant for that calculation. Figures and probability
rules are fictional game parameters.
