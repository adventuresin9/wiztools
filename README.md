# wiztools
Send commands from Plan9 or 9Front to Philips WiZ lightbulbs

All these commands all take an ip address as the first argument.
Some take a number for the second agrument.
wizrgb take 3 numbers for the second, third, and forth arguments.

command [ip address]
wizget
wizon
wizoff 
wizpick

command [ip address] [#]
wizstate
wiztemp
wizscene
wizspeed
wizdim

command [ip address] [#] [#] [#]
wizrgb

See notes.txt for valid integers.
