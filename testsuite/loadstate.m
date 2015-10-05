function [number snap state] = loadstate(filename)
fid=fopen(filename)
formatSpec = '%f';
snap = fscanf(fid,formatSpec,1);
number = fscanf(fid,formatSpec,1);
formatSpec = '%f %f %f %f %f %f';
sizeA = [6 Inf];
A=fscanf(fid,formatSpec,sizeA);
state=A';

end

