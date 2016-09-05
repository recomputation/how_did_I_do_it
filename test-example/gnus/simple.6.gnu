set terminal png transparent nocrop enhanced size 450,320 font "arial,8" 
set output 'simple.6.png'
set key bmargin center horizontal Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid
set samples 400, 400
set title "Simple Plots" 
set title  font ",20" norotate
plot [-5*pi:5*pi] [-5:5] real(tan(x)/atan(x)), 1/x

