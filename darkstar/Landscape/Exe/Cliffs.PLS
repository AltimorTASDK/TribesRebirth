;  push           256
seed	103
clr	0
dup
fill_n	.7
fft	1
pop
normalize 0 1
floor	0.4 0.01 0.4
normalize 0 .3
curve	.01 6
dup
clr	0
dup
fill_n	.7
fft	1
pop
normalize	0 1
floor	0.4 0.01 0.4
ceil	0.69 0.01 0.2
normalize .1 .5
curve         .01 6
overlay	1 .35
curve	.01 6
normalize 0.0 1200.0

 