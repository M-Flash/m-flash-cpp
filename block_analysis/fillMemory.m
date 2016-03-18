function [] = fillMemory(current_size, expected_size)
	vector_size = (current_size - expected_size)/8;
	vector = ones(vector_size, 1);
	pause;
end
