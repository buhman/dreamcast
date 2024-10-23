cdrecord -speed=8 -v dev=/dev/sr0 -dao -multi taudio01.wav
cdrecord -eject -overburn -speed=8 -v dev=/dev/sr0 -tao -xa tdata02.iso
