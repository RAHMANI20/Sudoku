#ifndef GRID_H
#define GRID_H

#include <stdint.h> 
#include <stdio.h>
#include <stdbool.h>

#define MAX_GRID_SIZE 64
#define EMPTY_CELL '_'

static const char color_table[] = "123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "@" 
                                  "abcdefghijklmnopqrstuvwxyz" "&*_";

/* sudoku grid (forward declaration to hide the implementation) */
typedef struct _grid_t grid_t;

/* sudoku grid choice (forward declaration to hide the implementation) */
typedef struct choice_t choice_t;

/* Allocate a memory needed for grid_t structure */
grid_t *grid_alloc (size_t size);

/* Free the memory allocated by the given grid */
void grid_free (grid_t *grid);

/* Print the given grid */
void grid_print (const grid_t *grid, FILE *fd);

/* Check if the character is valid */
bool grid_check_char (const grid_t *grid, const char c);

/* Check if the size is valid */
bool grid_check_size (const size_t size);

/* Copy totally the grid and return it */
grid_t *grid_copy (const grid_t *grid);

/* Get content of a given cell */
char *grid_get_cell (const grid_t *grid, const size_t row, 
                     const size_t column);

/* Return the size of the grid */
size_t grid_get_size (const grid_t *grid);

/* Set a grid cell to a specific value */
void grid_set_cell (grid_t *grid, const size_t row, 
                    const size_t column, const char color);

/* Check if a grid is solved or not */
bool grid_is_solved (grid_t *grid);

/* Searches for inconsistencies in the given grid */
bool grid_is_consistent (grid_t *grid);

/* Try to solve the grid using heuristics */ 
size_t grid_heuristics (grid_t *grid);

/* It will free a choice_t data structure */
void grid_choice_free (choice_t *choice);

/* Check if the color set of the choice is empty or not */
bool grid_choice_is_empty (const choice_t *choice);

/* Apply the choice to the given grid */ 
void grid_choice_apply (grid_t *grid, const choice_t *choice);

/* It will blank the given choice */
void grid_choice_blank (grid_t *grid, const choice_t *choice);

/* Discard the choice from the grid */
void grid_choice_discard (grid_t *grid, const choice_t *choice);

/* Write the choice on the file descriptor */
void grid_choice_print (const choice_t *choice, FILE *fd);

/* Choose a color in the given grid */
choice_t *grid_choice (grid_t *grid);

/* Generate a grid and fill it randomly */
grid_t *grid_random (size_t size);

/* Remove the given number of cells from the grid */
void grid_remove_cells (grid_t *grid, size_t count_cell);

/* Count number of fill ratio of the given grid */
size_t grid_fill_ratio (grid_t *grid);

#endif /* GRID_H */
