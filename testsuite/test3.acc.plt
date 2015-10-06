v0=-1
k=4000
m=1
delta=0.1
omega0=sqrt(k/m)
omega1=omega0*sqrt((1-delta*delta))
v(x)=v0*exp(-omega0*delta*(x-0.01))*cos(omega1*(x-0.01))
plot "traj0.dat" us 1:4, v(x)
replot "traj1.dat" us 1:4, -v(x)
