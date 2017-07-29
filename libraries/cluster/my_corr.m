function [ res ] = my_corr( x, y )
%MY_CORR This is a trivial implementation of corr() via corrcoef().
%   

xCols = size(x, 2);
yCols = size(y, 2);
res = zeros(xCols, yCols);

for i = 1:xCols
    for j = 1:yCols
        tmp = corrcoef(x(:, i), y(:, j));
        res(i, j) = tmp(1, 2);
    end    
end

end

