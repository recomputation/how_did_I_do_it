set terminal png transparent nocrop enhanced size 450,320 font "arial,8" 
set output 'simple.7.png'
set key bmargin left horizontal Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid
set samples 800, 800
set title "Simple Plots" 
set title  font ",20" norotate
plot [-30:20] sin(x*20)*atan(x)

