function [ ret ] = loadBuffers( fileC, montC )
%LOADBUFFERS Load content of buffers from files.
%   

ret.filterBuffer = readBuffer('filterBuffer.txt', 1);
ret.after_readData = readBuffer('after_readData.txt', fileC);
ret.after_writeBuffer = readBuffer('after_writeBuffer.txt', fileC);
ret.after_getAny = readBuffer('after_getAny.txt', fileC);
ret.after_fft = readBuffer('after_fft.txt', fileC);
ret.after_multiply = readBuffer('after_multiply.txt', fileC);
ret.after_filter = readBuffer('after_filter.txt', fileC);
ret.after_montage = readBuffer('after_montage.txt', montC);

end

