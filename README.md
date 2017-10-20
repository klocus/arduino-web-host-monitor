# Arduino Web Host Monitor
Simple host status monitor that checks if the host is online or offline. Unfortunately I ran out of space in Arduino UNO memory to add automatic mode and notification to email. For reasons of memory, I was forced to also limit the number of hosts to four.

Hosts can be edited via web interface.

**I used the following components:**
- Arduino UNO,
- Ethernet Shield,
- LCD Keypad Shield 2x16,
- micro SD card.

**Additional libraries:**
- NettigoKeypad (operating the display keys)
- SdFat (I replaced it with the standard SD library because it took less memory)
- aWOT (web interface)

**A video where you can see it in action:**
https://youtu.be/bNa4fz_fVJo
