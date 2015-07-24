function [ r ] = btheoricalanalysis2( graph_density)
%BANALYSIS Summary of this function goes here
%   Detailed explanation goes here
ram_size = [0.1:0.1:1 1:250];
vertex_size = 1;
edge_size =  2; %* vertex_size;
vertices = 1;
edges = vertices * graph_density;
index = 1;
threads = 1;

densities = [0.01 0.05 0.1 0.5 1 5 10 50 100];
values = zeros(length(densities), 3);

index = 1;
threads = 1;
for ram = ram_size
    partitions = max(threads * 2 * 100 / ram, 1);
    block_edges = edges / (partitions^2);
    block_vertices = vertices/ partitions;
    [nbp, sbp] = getIOCost(block_vertices, block_edges, vertex_size, edge_size, partitions);
    nbp = nbp * partitions^2;
    sbp = sbp * partitions^2;
    values(index, :) = [nbp, sbp, min(nbp, sbp), partitions];
    index = index +1;
end
logvalues = values;
logvalues = log(values);
plot(ram_size, logvalues(:,1:3), '-');
r = [ram_size' values];

end

