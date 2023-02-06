#ifndef COLORS_H
#define COLORS_H

#include <stdint.h> 
#include <stdio.h>
#include <stdbool.h>

#define MAX_COLORS 64

typedef uint64_t colors_t;

/* Initializes the first size colors */
colors_t colors_full (const size_t size);

/* Return empty colors */
colors_t colors_empty (void);

/* Set the color at the given index */
colors_t colors_set (const size_t color_id);

/* Add the given color */
colors_t colors_add (const colors_t colors, const size_t color_id);

/* Discard the given color */
colors_t colors_discard (const colors_t colors, const size_t color_id);

/* Check if the given color is set */
bool colors_is_in (const colors_t colors, const size_t color_id);

/* Bitwise negate the colors */
colors_t colors_negate (const colors_t colors);

/* Compute the intersection between two colors */
colors_t colors_and (const colors_t colors1, const colors_t colors2);

/* Compute the union of two colors */
colors_t colors_or (const colors_t colors1, const colors_t colors2);

/* Compute the XOR of two colors */  
colors_t colors_xor (const colors_t colors1, const colors_t colors2);

/* Compute the substraction between two colors */
colors_t colors_subtract (const colors_t colors1, const colors_t colors2);

/* This function checks the equality of two colors */
bool colors_is_equal (const colors_t colors1, const colors_t colors2);

/* Test the inclusion of colors1 in colors2 */
bool colors_is_subset (const colors_t colors1, const colors_t colors2);

/* Check if there is only one color in the colors */
bool colors_is_singleton (const colors_t colors);

/* Returns the number of colors enclosed in the set */
size_t colors_count (const colors_t colors);

/* Give the rightmost color of the set */
colors_t colors_rightmost (const colors_t colors);

/* Give the leftmost color of the set */
colors_t colors_leftmost (const colors_t colors);

/* Returns a random color chosen from the color set. 
   We should initialize PRNG for this function */
colors_t colors_random (const colors_t colors);

/* Check the consistency of one unique subgrid */
bool subgrid_consistency (colors_t subgrid[], const size_t size);

/* Apply heuristic on a given subgrid */
bool subgrid_heuristics (colors_t *subgrid[], size_t size);

/* Removes all the colors that are already solved */
bool cross_hatching (colors_t *subgrid[], size_t size);

/* Check if one color is present only once */
bool lone_number (colors_t *subgrid[], size_t size);

/* Looking at subsets of N candidates in N cells, then discrads them */
bool naked_subset (colors_t *subgrid[], size_t size);

/* Looking at subsets of N candidates in N cells, 
   then it remove any other colors but the N colors from the N cells */ 
bool hidden_subset (colors_t *subgrid[], size_t size);

#endif /* COLORS_H */
