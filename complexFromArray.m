function [ out ] = complexFromArray( data )
%COMPLEXFROMARRAY Create array of complex numbers.
%   

out = reshape(data, 2, length(data)/2);
out = complex(out(1,:), out(2,:));

end

