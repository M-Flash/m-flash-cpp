function [ r ] = btheoricalanalysisDensity( graph_density)
%BANALYSIS Summary of this function goes here
%   Detailed explanation goes here
ram_size = [1:100];
vertex_size = 1;
edge_size =  2; %* vertex_size;
vertices = 1;
edges = vertices * graph_density;
values = zeros(length(ram_size), 4);
cost = edges*edge_size + 2*vertices*vertex_size;
index = 1;
threads = 1;

%densities = [1 2.5 5 10 25 50 100]

for ram = ram_size
    partitions = max(threads * 2 * 100 / ram, 1);
    block_edges = edges / (partitions^2);
    block_vertices = vertices/ partitions;
    [nbp, sbp] = getIOCost(block_vertices, block_edges, vertex_size, edge_size, partitions);
    nbp = nbp * (partitions^2);
    sbp = sbp * (partitions^2);
    values(index, :) = [nbp, sbp, min(nbp, sbp), partitions];
    index = index +1;
end
values = [values(:,1:3)/cost values(:,4)];
%logvalues = log(values);
plot(ram_size, values(:,1:3), '-');
r = [ram_size' values];
title(sprintf('I/O Cost for a graph with density %d', graph_density));
xlabel('Percentage of vertices in RAM ')
ylabel('t-cost units')
ylim([ 0 4]);
legend('DBP', 'SPP', 'BBP')

end

