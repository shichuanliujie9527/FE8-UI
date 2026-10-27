# Native trade mouse input

The exact Chinese FE8J image already identified by the compatibility profile
now supports clicking item rows in the game's original two-column trade menu.
Click an item, then an occupied destination for a swap or the offered empty
slot for a transfer. Right-click remains the game's B button. During dialogue,
left-click remains A to advance text. The guided vulnerary tutorial retains
its own item, destination and exit restrictions; use B/right-click when it asks
to finish. Portraits, the column gap and unavailable rows do not confirm an item.

This is separate from Armory. It reads trade state and emits ordinary GBA
direction/A keys; it never writes inventory memory or tutorial flags. Armory
editing remains disabled for this ROM. Other ROMs keep their existing input
behaviour, since these addresses are enabled only by the exact SHA-1 profile.

## Address evidence

The local ROM literals and FE GBA Function Library's FE8J cross-references for
`StartTradeMenu` (`0802DCA4`) identify the global pointer `0203A60C` and script
`085C3FFC`. Unselected/selected loop callbacks are `0802D76D` and `0802D8B9`.
The supplied trade checkpoint independently confirmed the script, callback,
unit pointers, row/cell state and item values. No ROM bytes or saves are included.

Structure offsets and tutorial input behaviour were cross-checked against
[the trade header](https://github.com/FireEmblemUniverse/fireemblem8u/blob/master/include/bmtrade.h)
and [trade implementation](https://github.com/FireEmblemUniverse/fireemblem8u/blob/master/src/bmtrade.c).
These are technical references, not copied implementations. The adapter is
new frontend code using validated state observations and ordinary input.

## Checks and local validation

Each read validates the script, callback, script cursor, aligned player-unit
pointers, row bounds and selectable cells. Navigation cancels on a blocked
menu, changed process/units/items/selection phase/tutorial stage, keyboard use,
right-click, state load or timeout. A is emitted only after the requested cell
is reached. Input pulses have release intervals and stop after one confirmation.

The synthetic `native_trade` test checks navigation and rejection/cancellation;
the existing profile test checks that a header alone does not enable the adapter.
The optional `test_native_trade_rom` utility loads a private checkpoint, advances
the tutorial with normal buttons, selects the first right-side vulnerary through
the same mouse controller, transfers it to the left empty slot, checks both
inventories and exits through the game's tutorial. It does not run in CI:

```
test_native_trade_rom.exe PRIVATE.gba TRADE.ss [PRIVATE-result.ss]
```

Its fixture is the supplied Gilliam/Franz tutorial at the first selection step;
it is not a general test for every trade state. Keep all inputs and outputs local.

The supplied tutorial checkpoint was exercised locally on Windows on 2026-10-27.
The controller selected the first right-side vulnerary, confirmed the offered
left slot, advanced the completion dialogue and exited with B. Inventory values
changed from left `[2D14, 0, 0, 0, 0]` / right `[2E01, 2D14, 036C, 036C, 0]`
to left `[2D14, 036C, 0, 0, 0]` / right `[2E01, 2D14, 036C, 0, 0]`.
The resulting state also loaded successfully in the combined Windows frontend
with LCD enabled and returned to the map tutorial. The original checkpoint's
SHA-256 remained unchanged; no fixture was uploaded or committed.

The combined build passed 41 Windows and 38 Linux tests. This validates the
supplied guided transfer and the tested controller guards, not every chapter
or native game menu. The native trade adapter remains restricted to the exact
Chinese ROM profile.
