set terminal png transparent nocrop enhanced size 450,320 font "arial,8" 
set output 'simple.8.png'
set key bmargin left horizontal Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid
set samples 800, 800
set title "Simple Plots" 
set title  font ",20" norotate
x = 0.0
## Last datafile plotted: "3.dat"
plot [-19:19] '1.dat'with impulses ,'2.dat' ,'3.dat' with lines

