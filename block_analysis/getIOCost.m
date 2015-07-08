function [nbp sbp] = getIOCost(vertices, edges, vertex_size, edge_size, partitions)
nbp = vertices  * vertex_size  + (vertices * vertex_size) /partitions  + edges * edge_size;
sbp = edges * edge_size + (2 * vertex_size* vertices) / partitions + 2 * edges * edge_size * vertex_size;
end