function [ output_args ] = banalysis_graph(path, graph, vertices)
vertex_size = 1;
edge_size =  2; %* vertex_size;
threads = 1;

index = 1;
pvalues = fliplr([2 4 20 40 200]);
counters = zeros(length(pvalues), 3);
blockCounters = zeros(length(pvalues), 2);
vertex_percentage = 1./pvalues* 200;
for p = pvalues
    partitions_counters = load(strcat(path, graph, num2str(p)));
    cost = sum(partitions_counters(:))*edge_size + 2*vertices*vertex_size;
    partitions = length(partitions_counters);
    vertices_partition = floor(vertices/p);
    mCost = 0;
    nbpCost = 0;
    sbpCost = 0;
    sparse_blocks = 0;
    dense_blocks = 0;
    for i=1:partitions
        for j=1:partitions
            block_vertices = vertices_partition;
            block_edges = partitions_counters(i,j);
            [nbp, sbp] = getIOCost(block_vertices, block_edges, vertex_size, edge_size, partitions);
            if nbp < sbp
                dense_blocks = dense_blocks + 1;
            else
                sparse_blocks = sparse_blocks + 1;
            end
            mCost = mCost + min(nbp, sbp);
            nbpCost = nbpCost + nbp;
            sbpCost = sbpCost + sbp;
        end
    end
    counters(index, :) = [nbpCost, sbpCost, mCost]/cost;
    blockCounters(index, :) = [dense_blocks, sparse_blocks];
    blockCounters(index, :) = [dense_blocks, sparse_blocks];
    index = index + 1;
end
figure;
bar(counters, 'grouped');
title(sprintf('I/O cost for %s graph using different memory sizes', graph));
set(gca,'xticklabel', num2str(vertex_percentage', '%0.0f%%'));
ylabel('t-cost');
xlabel('Percentage of Vertices in RAM');
legend({'DBP' 'SPP' 'BBP'});
saveas(gcf, strcat(path, graph, '.fig'));
saveas(gcf, strcat(path, graph, '.pdf'));
saveas(gcf, strcat(path, graph, '.png'));
figure,
bar(blockCounters, 'grouped');
title(sprintf('Blocks for %s graph using different memory sizes', graph));
set(gca,'xticklabel', num2str(vertex_percentage', '%0.0f%%'));
legend ({'Dense Blocks' 'Sparse Blocks'} )
ylabel('COST');
saveas(gcf, strcat(path, graph, '-blocks', '.fig'));
saveas(gcf, strcat(path, graph, '-blocks', '.pdf'));
close all;
end

