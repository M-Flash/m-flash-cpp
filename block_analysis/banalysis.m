function [ output_args ] = banalysis( partitions_counters, vertices, vertices_partition)
%BANALYSIS Summary of this function goes here
%   Detailed explanation goes here
vertex_size = 1;
edge_size =  2; %* vertex_size;
cost = sum(partitions_counters(:))*edge_size + 2*vertices*vertex_size;
threads = 1;

partitions = length(partitions_counters);

nbpCost = 0;
sbpCost = 0;
mCost = 0;
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

bar([nbpCost sbpCost mCost]/cost );
set(gca,'xticklabel', {'NBP' 'SBP' 'MFlash'})
ylabel('COST');
figure,
bar([dense_blocks  sparse_blocks]);
set(gca,'xticklabel', {'Dense Blocks' 'Sparse Blocks'} )
end

