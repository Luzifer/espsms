# Luzifer / espsms

This project is a replacement for an older version which was based on an USB-SIM800C module. The old setup was quite unreliable as the modem needed to be restarted on every reboot of the machine manually because this wasn't possible from software. Therefore this is a rewrite based on an ESP32 with an SIM800L module being able to reset the modem from software.

The module receives SMS for an older SIM-card of mine and forwards them to a server doing some automation with it.
