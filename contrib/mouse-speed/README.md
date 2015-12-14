This script executes an external bash script on microbreak start 
and another on successfull completion of the microbreak without
postpone/skip used.

In this example it calls `mouse-speed`, which slows down the mouse
if you don't take your break and resets it to full speed, when you
complete a break.

Depends on https://github.com/rubo77/mouse-speed
