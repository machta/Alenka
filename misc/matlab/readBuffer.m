function [ data ] = readBuffer( filePath, channels )
%READBUFFER Import data from a text file.
% 

data = importdata(filePath);
data = reshape(data, length(data)/channels, channels);
data = data';
 
end
