# Arduino-Timer-Switch
The Arduino Core uses Timer0 for timing Functions, if you need Timer0 T0 Pin for Measurement/External Triggering you can Switch the Timing to Timer2. This is a Variant set.
</br>


Copy The Files to your IDE Folder
/hardware/Arduino/avr/variants/TNStandard

Change/Add Line in Boards.txt</br>
</br>
Example1:</br>
Change Line </br>
uno.build.variant=standard</br>
To</br>
uno.build.variant=TNStandard</br>
</br>
Example2(Creates new Menu for Both)</br>
Delete Line</br>
uno.build.variant=standard</br>
</br>
Add Lines</br>
uno.menu.cpu.Timer0=Timer0</br>
uno.menu.cpu.Timer0.build.variant=standard</br>
uno.menu.cpu.Timer1=Timer1</br>
uno.menu.cpu.Timer1.build.variant=TNStandard</br>
