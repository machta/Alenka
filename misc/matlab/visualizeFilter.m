function [ b ] = visualizeFilter( fileName, mode )
%VISUALIZEFILTER A tool for visualizing properties of a FIR filter.
%   
%   fileName  name of the input file
%   mode      0 - freqz() diagram, 1 - open the Filter Vusializing Tool
%
%   Displays a graph with the frequency response of the FIR filter 
%   given by the coefficients in the input file.

if nargin == 1
    mode = 0;
end

b = importdata(fileName);
fs = b(1); % sampling f
f1 = b(2); % lowpass frequancy
f2 = b(3); % highpass frequancy
b = b(4 : length(b));

if mode == 0
    subplot(2,1,1);
        
    p = plot([f1 f1],[10 -100]);
    set(p,'Color','magenta');
    hold on;

    p = plot([f2 f2],[10 -100]);
    set(p,'Color','green');
    
    freqz(b,1,length(b)*10,fs);

    legend('lowpass f','highpass f','filter');
    hold off;
else
    hd = dfilt.dffir(b);
    freqz(hd);
end

end
