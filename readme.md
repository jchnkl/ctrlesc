### CtrlEsc

A simple utility to use the Escape key as Control key as well.  
It works by grabbing the Escape key globally and relaying Key{Press,Release}
events. The control logic works like this:

    if pressed key == escape then
        until not (released key == escape) then
            send (key + ctrl)

    else if released key == escape then
        if not other key was pressed then
            send escape

This makes other methods like timeouts for deciding whether escape or ctrl
should be used unnecessary.

Caveat: Escape will be sent *after* the Escape key is released. If you are
a fast/messy typist you'll notice that this might interfere with your habits.
