block_vertices = 1073741824;
counters = [48345979280 635523016; 3816360040 294943896 ] / 8;
n = length(counters);
sparse = logical(n);
for i = 1:n
    for j = 1:n
        [dbp spp] = getIOCost(block_vertices, counters(i,j), 4, 8, 3);
        sparse(i,j) = spp < dbp;
    end
end

sparse
    