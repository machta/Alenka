function [ after_readData after_writeBuffer after_getAny after_filter after_montage ] = loadBuffers( fileC, montC )
%LOADBUFFERS Load content of buffers from files.
%   

after_readData = readBuffer('after_readData.txt', fileC);
after_writeBuffer = readBuffer('after_writeBuffer.txt', fileC);

after_getAny = readBuffer('after_getAny.txt', fileC);
after_filter = readBuffer('after_filter.txt', fileC);
after_montage = readBuffer('after_montage.txt', montC);

end

function [ data ] = readBuffer( filePath, channels )

data = importdata(filePath);
data = reshape(data, length(data)/channels, channels);
data = data';
 
end