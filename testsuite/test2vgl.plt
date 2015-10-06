eta=0.1
a0=0.5
v0=1
f(x)=v0*exp(-eta*x)+a0/eta*(1-exp(-eta*x))
plot "traj.dat" us 1:4, f(x), "traj.dat" us 1:5