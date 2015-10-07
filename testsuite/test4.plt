v0=-1
k=4000
m=1
delta=0.1
omega0=sqrt(k/m)
omega1=omega0*sqrt((1-delta*delta))
v(x)=exp(-omega0*delta*x)*(v0*cos(omega1*(x)))-v0*omega0/omega1*delta*sin(omega1*(x))
plot "traj0.dat" us 1:5, v(x)
replot "traj1.dat" us 1:5, -v(x)
#plot "traj0.dat" us 1:6, a(x)