function [] = alenkaSave(fileName, data, Fs)
%ALENKASAVE Saves big signals in the required format for Alenka.
%   fileName name of the new MAT file
%   data     column-major matrix with the signal samples
%   Fs       sampling frequency
%
% You can use this to quickly export your signal to a MAT file in a format
% that is expected by Alenka.
%
% The resulting file will be in version 4. The reason for this is that the 
% new compressed format is tragically slow to the point of being absolutely
% useless other than for the smallest files (something like 10-50 MB).
%
% The signal is split into chunks of constant size so there is no size limit
% other than memory and disk space.
% 

save(fileName, 'fs', '-v4');

samples = size(data, 1);
channels = size(data, 2);
chunkLength = floor(500*1000*1000/channels/8);

for i = 0:samples/chunkLength
    from = 1 + i*chunkLength;
    to = (i + 1)*chunkLength;
    to = min(to, samples);
    
    dataChunk = data(from:to, :);
    
    varName = ['data' num2str(i)];
    eval([varName ' = dataChunk;']);

    save(fileName, varName, '-append');
    
    eval(['clear ' varName]);
end

end

