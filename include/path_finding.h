#ifndef PATH_FINDING_H_INCLUDE
#define PATH_FINDING_H_INCLUDE

#include <structs.h>

Path* create_path(Node *node);
double calculate_h(Point_Int start,Point_Int end);
SDL_bool is_valid(Point_Int point);
Node* create_node(int x, int y, Node* previous, double g, double h);
Path* a_star_search(int **matriz, Point_Int start, Point_Int end);

#endif