function [ r ] = btheoricalanalysis3( graph_density)
%BANALYSIS Summary of this function goes here
%   Detailed explanation goes here
ram_size = fliplr([0.01:0.01:0.99 1:250]);
vertex_size = 1;
edge_size =  2; %* vertex_size;
vertices = 1;
index = 1;
threads = 1;

values = zeros(length(ram_size), 3);
%densities = [1 2.5 5 10 25 50 100]

    
for ram = ram_size
    partitions = max(threads * 2 * 100 / ram, 1);
    ppartitions = partitions^2;
    block_vertices = vertices/ partitions;
    threshold_edges = ( (partitions-1) * block_vertices )/ (2 * partitions * edge_size)
    cost = ppartitions* threshold_edges * edge_size + 2*vertices*vertex_size;
    [nbp, sbp] = getIOCost(block_vertices, threshold_edges, vertex_size, edge_size, partitions);
    values(index,:) = [nbp*ppartitions/cost  threshold_edges*ppartitions/vertices threshold_edges/block_vertices];
    index = index +1;
end
%logvalues = log(values);
plot(ram_size, values(:,1), '-');
r = [ram_size' values];
title('Marginal I/O Cost for 1 iteration on M-Flash')
ylabel('COST Units')
xlabel('Percentage of vertices in RAM ');
ylim([ 0 4]);

end

