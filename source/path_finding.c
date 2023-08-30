#include <header.h>

Path* create_path(Node *node){
	if(!node) return NULL;

	Path *path = NULL;
	Node *buffer_node;

	while(node){

		Path *new_path = calloc(1,sizeof(Path));
		new_path->x = node->x;
		new_path->y = node->y;

		if(!path){
			new_path->next = NULL;
		}
		else{
			new_path->next = path;
		}
		path = new_path;

		buffer_node = node->previous;
		free(node);
		node = buffer_node;
	}

	return path;
}

double calculate_h(Point_Int start,Point_Int end){
	return sqrt((start.y - end.y) * (start.y - end.y) + (start.x - end.x) * (start.x - end.x));
}

SDL_bool is_valid(Point_Int point){
	return (point.x >= 0 && point.x < MAP_WIDTH && point.y >= 0 && point.y < MAP_HEIGHT);
}

Node* create_node(int x, int y, Node* previous, double g, double h){
	Node* new_node = calloc(1, sizeof(Node));
	new_node->x = x;
	new_node->y = y;
	new_node->previous = previous;
	new_node->g = g;
	new_node->h = h;
	new_node->f = g + h;
	return new_node;
}

Path* a_star_search(int **matriz, Point_Int start, Point_Int end){
	if (!is_valid(start) || !is_valid(end)) {
		printf("caminho invalido\n");
		return NULL;
	}

	if (matriz[start.y][start.x] > -1 && matriz[end.y][end.x] > -1) {
		printf("caminho invalido\n");
		return NULL;
	}

	SDL_bool closed_list[MAP_HEIGHT][MAP_WIDTH];
	memset(closed_list, SDL_FALSE, sizeof(closed_list));

	Node* start_node = create_node(start.x, start.y, NULL, 0.0, calculate_h(start, end));

	Node* open_list[MAP_HEIGHT * MAP_WIDTH];
	int open_list_size = 1;
	open_list[0] = start_node;

	int directions[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};

	while (open_list_size > 0) {
		Node* current_node = open_list[0];
		int current_index = 0;

		for (int i = 1; i < open_list_size; ++i) {
			if (open_list[i]->f < current_node->f || (open_list[i]->f == current_node->f && open_list[i]->h < current_node->h)) {
				current_node = open_list[i];
				current_index = i;
			}
		}

		if (current_node->y == end.y && current_node->x == end.x) {
			return create_path(current_node);
		}

		open_list[current_index] = open_list[open_list_size - 1];
		--open_list_size;

		closed_list[current_node->y][current_node->x] = SDL_TRUE;

		for (int i = 0; i < 4; ++i) {
			Point_Int new_point = {current_node->x + directions[i][0],current_node->y + directions[i][1] };

			if (is_valid(new_point) && matriz[new_point.y][new_point.x] == -1 && !closed_list[new_point.y][new_point.x]) {
				double new_g = current_node->g + 1.0;
				double new_h = calculate_h(new_point, end);
				double new_f = new_g + new_h;

				SDL_bool in_open_list = SDL_FALSE;
				int open_index = -1;

				for (int j = 0; j < open_list_size; ++j) {
					if (open_list[j]->y == new_point.y && open_list[j]->x == new_point.x) {
						in_open_list = SDL_TRUE;
						open_index = j;
						break;
					}
				}

				if (in_open_list && new_f >= open_list[open_index]->f) {
					continue;
				}

				if (!in_open_list || new_f < open_list[open_index]->f) {
					Node* new_node = create_node(new_point.x, new_point.y, current_node, new_g, new_h);

					if (!in_open_list) {
						open_list[open_list_size] = new_node;
						++open_list_size;
					}
					else {
						open_list[open_index] = new_node;
					}
				}
			}
		}
	}

    return NULL;
}