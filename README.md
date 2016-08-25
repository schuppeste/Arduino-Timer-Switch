# Arduino-Timer-Switch
The Arduino Core uses Timer0 for timing Functions, if you need Timer0 T0 Pin for Measurement/External Triggering you can Switch the Timing to Timer2. This is a Variant set.

Copy The Files to your IDE Folder and Change/Add Line in Boards.txt

Example1:
Change Line 
uno.build.variant=standard
To
uno.build.variant=TNStandard

Example2(Creates new Menu for Both)
Delete Line
uno.build.variant=standard

Add Lines
uno.menu.cpu.Timer0=Timer0
uno.menu.cpu.Timer0.build.variant=standard
uno.menu.cpu.Timer1=Timer1</br>
uno.menu.cpu.Timer1.build.variant=TNStandard
