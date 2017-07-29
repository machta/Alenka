function [ out ] = repeat_if_smaller( x, n )
%REPEAT_IF_SMALLER Repeat the first element n times if x is not of length n.
%   

if length(x) ~= n
    fprintf(2, 'Warning: size mismatch\n');
    out = repmat(x(1), n, 1);
else
    out = x;
end

end

