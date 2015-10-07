v0=-1
k=4000
m=1
delta=0.1*sqrt(2)
omega0=sqrt(k/m*2)
omega1=omega0*sqrt((1-delta*delta))
vsch=0.5
vr(x)=exp(-omega0*delta*x)*(v0*cos(omega1*(x)))-v0*omega0/omega1*delta*sin(omega1*(x))
v1(x)=vsch+0.5*vr(x)
v2(x)=vsch-0.5*vr(x)
#plot vr(x)
set xrange[0:0.1]
plot v1(x),v2(x)
replot "traj0.dat" us 1:4
replot "traj1.dat" us 1:4
#plot "traj0.dat" us 1:6, a(x)