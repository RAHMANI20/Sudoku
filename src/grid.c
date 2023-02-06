#include "grid.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <colors.h>
#include <string.h>
#include <math.h>

/* Internal structure (hiden from outside) to represent a sudoku grid */
struct _grid_t
{
  size_t size;
  colors_t **cells;
};

/* Internal structure (hiden from outside) to represent a sudoku grid choice */
struct choice_t
{
  size_t row;
  size_t column;
  colors_t color;
};

grid_t *grid_alloc (size_t size)
{  
  if (!grid_check_size(size))
    return NULL;
    
  grid_t *grid = malloc(sizeof(grid_t));
  
  if (grid == NULL)
    return NULL;
  
  grid->size = size;
  grid->cells = calloc(grid->size, sizeof(colors_t*));
  
  if (!grid->cells)
    return NULL;

  for (size_t i = 0; i < grid->size; i++){
    grid->cells[i] = calloc(grid->size, sizeof(colors_t));
    if (!grid->cells[i]){
      for (size_t j = 0; j < i; j++){
        free(grid->cells[j]);
      }
      free(grid->cells);
      return NULL;
    }
  }

  for (size_t i = 0; i < grid->size; i++){
    for (size_t j = 0; j < grid->size; j++){
      grid->cells[i][j] = colors_full(grid->size);
    }
  }
  return grid;
}

void grid_free (grid_t *grid)
{
  if (grid){
    if (grid->cells){
      for (size_t i = 0; i < grid->size; i++){
        free(grid->cells[i]);
      }
      free(grid->cells);
    }
    free(grid);
  }
}

void grid_print (const grid_t *grid, FILE *fd)
{  
  if (grid && fd){
    for (size_t i = 0; i < grid->size; i++){
      for (size_t j = 0; j < grid->size; j++){
        char *cell = grid_get_cell(grid, i, j);
        if (cell){
          if (strlen(cell) > 1)
            fprintf(fd, "%c ", EMPTY_CELL); 
          else
            fprintf(fd,"%s ", cell);
          
          free(cell);
        }
      }
      fprintf(fd, "\n");
    }
    fprintf(fd, "\n");
  }  
}

bool grid_check_char (const grid_t *grid, const char ch)
{
  if (!grid)
    return false;
          
  char possible_char[] = "_123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ@abcdefg"
                         "hijklmnopqrstuvwxyz&*";
  for (size_t i = 0; i < grid->size + 1; i++)
    if (ch == possible_char[i])
      return true;
			
  return false;
}

bool grid_check_size (const size_t size)
{
  return (size == 1 || size == 4 || size == 9 || size == 16 || size == 25
         || size == 36 || size == 49 || size == 64);
}

grid_t *grid_copy (const grid_t *grid)
{
  if (grid == NULL)
    return NULL;
  
  grid_t *copy_grid = grid_alloc(grid->size);
  if (copy_grid == NULL)
    return NULL;
    
  for (size_t i = 0; i < grid->size; i++)
    for (size_t j = 0; j < grid->size; j++)
      copy_grid->cells[i][j] = grid->cells[i][j];
      
  return copy_grid;      
}

char *grid_get_cell (const grid_t *grid, const size_t row, const size_t column)
{  
  if (grid == NULL || row >= grid->size || column >= grid->size)
    return NULL;
    
  char *color = calloc(colors_count(grid->cells[row][column]) + 1, 
                       sizeof(char));
  if (color == NULL)
    return NULL;
                                          
  size_t i = 0;
  for (size_t color_id = 0; color_id < MAX_GRID_SIZE; color_id++){
    if (colors_is_in(grid->cells[row][column], color_id)){
      color[i] = color_table[color_id];
      i++;
    }
  }
  return color;
}

size_t grid_get_size (const grid_t *grid)
{
  if (!grid)
    return 0;
    
  return grid->size;
}

void grid_set_cell (grid_t *grid, const size_t row, const size_t column,
     const char color)
{  
  if (grid != NULL && row < grid->size && column < grid->size){
    char *address; 
    address = strchr(color_table, color);
    size_t color_id = (size_t)(address - color_table);
    if (color_id < grid->size){
      grid->cells[row][column] = colors_set(color_id);
    } else {
      grid->cells[row][column] = colors_full(grid->size);
    }
  }
}

bool grid_is_solved (grid_t *grid)
{
  if (!grid)
    return false;
    
  for (size_t i = 0; i < grid->size; i++)
    for (size_t j = 0; j < grid->size; j++)
      if (!colors_is_singleton(grid->cells[i][j]))
        return false;
  
  return true;
}

bool grid_is_consistent (grid_t *grid)
{  
  if (!grid)
    return false;
    
  colors_t subgrid[grid->size];  
  for (size_t i = 0; i < grid->size; i++){
    for (size_t j = 0; j < grid->size; j++)
      subgrid[j] = grid->cells[i][j];
    
    if (!subgrid_consistency(subgrid, grid->size))
      return false;
  }
  
  for (size_t j = 0; j < grid->size; j++){
    for (size_t i = 0; i < grid->size; i++) 
      subgrid[i] = grid->cells[i][j];
    
    if (!subgrid_consistency(subgrid, grid->size))
      return false;
  } 
    
  size_t block_size = sqrt(grid->size);
  for (size_t start_row = 0; start_row < grid->size; start_row += block_size)
    for (size_t start_column = 0; start_column < grid->size; start_column += 
      block_size){
      size_t i = 0;
      for (size_t r = start_row; r < start_row + block_size; r++)
        for (size_t c = start_column; c < start_column + block_size; c++)
          subgrid[i++] = grid->cells[r][c];
            
      if (!subgrid_consistency(subgrid, grid->size))
        return false;        
    }
  return true;
}

size_t grid_heuristics (grid_t *grid)
{
  if (!grid)
    return 2;
  
  if (grid->size == 1)
    return 1;  
  
  colors_t *subgrid[grid->size];
  bool grid_changed = true;
  while (grid_changed){
    grid_changed = false;  
    for (size_t i = 0; i < grid->size; i++){
      for (size_t j = 0; j < grid->size; j++)
        subgrid[j] = &(grid->cells[i][j]);
    
      grid_changed |= subgrid_heuristics(subgrid, grid->size); 
    }
  
    for (size_t j = 0; j < grid->size; j++){
      for (size_t i = 0; i < grid->size; i++) 
        subgrid[i] = &(grid->cells[i][j]);
    
      grid_changed |= subgrid_heuristics(subgrid, grid->size);
    }
  
    size_t block_size = sqrt(grid->size);
    for (size_t start_row = 0; start_row < grid->size; start_row += block_size)
      for (size_t start_column = 0; start_column < grid->size; start_column +=
        block_size){
        size_t i = 0;
        for (size_t r = start_row; r < start_row + block_size; r++)
          for (size_t c = start_column; c < start_column + block_size; c++)
            subgrid[i++] = &(grid->cells[r][c]);
      
        grid_changed |= subgrid_heuristics(subgrid, grid->size);  
      }
  } 
  if (!grid_is_consistent(grid))
    return 2;
    
  if (!grid_is_solved(grid))
    return 0;
  
  return 1;  
}

void grid_choice_free (choice_t *choice)
{
  if (choice)
    free(choice);
}

bool grid_choice_is_empty (const choice_t *choice)
{
  if (!choice)
    return false;
  
  return choice->color == 0;
}

void grid_choice_apply (grid_t *grid, const choice_t *choice)
{
  if (grid && choice){
    grid->cells[choice->row][choice->column] = choice->color;  
  }
}

void grid_choice_blank (grid_t *grid, const choice_t *choice)
{
  if (grid && choice){
    grid->cells[choice->row][choice->column] = colors_full(grid->size); 
  }  
}

void grid_choice_discard (grid_t *grid, const choice_t *choice)
{
  if (grid && choice){
    grid->cells[choice->row][choice->column] = 
    colors_subtract(grid->cells[choice->row][choice->column], choice->color);
  }
}

void grid_choice_print (const choice_t *choice, FILE *fd)
{
  if (choice && fd){
    char *colors = calloc(colors_count(choice->color) + 1, sizeof(char));                 
    if (colors){
      size_t i = 0;
      for (size_t color_id = 0; color_id < MAX_GRID_SIZE; color_id++)
        if (colors_is_in(choice->color, color_id)){
          colors[i] = color_table[color_id];
          i++;
        }
      free(colors);
    }
  }
}

choice_t *grid_choice (grid_t *grid)
{ 
  choice_t *choice = malloc(sizeof(choice_t));
  if (!grid || !choice)
    return NULL;
  
  choice->color = colors_full(grid->size + 1);
  choice->row = 0;
  choice->column = 0;
  if (grid->size == 1){
    choice->color = 1;
    return choice; 
  }
  for (size_t i = 0; i < grid->size; i++)
    for (size_t j = 0; j < grid->size; j++)
      if (!colors_is_singleton(grid->cells[i][j]) && 
        (colors_count(grid->cells[i][j]) < colors_count(choice->color))){
        choice->color = grid->cells[i][j];
        choice->row = i;
        choice->column = j;
      }
  choice->color = colors_leftmost(choice->color);
  return choice;       
}

grid_t *grid_random (size_t size)
{
  grid_t *grid = grid_alloc(size);
  if (!grid)
    return NULL;
    
  colors_t colors = colors_full(size);
  for (size_t i = 0; i < size; i++){
    grid->cells[0][i] = colors_random(colors);
    colors = colors_subtract(colors, grid->cells[0][i]);    
  }
  return grid;
}

void grid_remove_cells (grid_t *grid, size_t count_cell)
{
  if (grid){
    while (count_cell != 0)
    {
      size_t row = rand() % grid->size;
      size_t col = rand() % grid->size;
      if (colors_is_singleton(grid->cells[row][col])){
        grid->cells[row][col] = colors_full(grid->size);
        count_cell--;
      }
    }
  }
}

size_t grid_fill_ratio (grid_t *grid)
{
  if (!grid)
    return 0;
    
  size_t count_fill = 0;
  for (size_t i = 0; i < grid->size; i++)
    for (size_t j = 0; j < grid->size; j++)
      if (colors_is_singleton(grid->cells[i][j]))
        count_fill++;
  return  count_fill * 100 / (grid->size * grid->size);  
}

