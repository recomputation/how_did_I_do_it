set terminal png transparent nocrop enhanced size 450,320 font "arial,8" 
set output 'simple.4.png'
set key inside left top vertical Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid
set samples 200, 200
set title "Simple Plots" 
set title  font ",20" norotate
plot [-30:20] besj0(x)*0.12e1 with impulses, (x**besj0(x))-2.5 with points

