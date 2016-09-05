set terminal png transparent nocrop enhanced size 450,320 font "arial,8" 
set output 'simple.5.png'
set key inside left top vertical Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid
set samples 400, 400
set title "Simple Plots" 
set title  font ",20" norotate
plot [-10:10] real(sin(x)**besj0(x))

