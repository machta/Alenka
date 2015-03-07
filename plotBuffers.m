function [ ] = plotBuffers( data )
%PLOTBUFFERS Plot data in a similar way as the program does.
%   

height = 1;
yScale = 0.00001*height;
[m n] = size(data);

y0 = zeros(m, 1);
for i = 1:m
    y0(i) = (m - i + 0.5)*height/m;
end

y0 = repmat(y0, 1, n);

data = data*yScale + y0;
plot(data')

end

