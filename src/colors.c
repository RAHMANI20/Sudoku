#include "colors.h"

#include <stdlib.h>

colors_t colors_full (const size_t size)
{
  if (size > MAX_COLORS - 1)
    return (colors_t) 0 - 1;
  return (((colors_t) 1 << size) - 1);  
}

colors_t colors_empty (void)
{  
  return ((colors_t) 0);  
}

colors_t colors_set (const size_t color_id)
{
  if (color_id > MAX_COLORS - 1)
    return colors_empty();
  return ((colors_t) 1 << color_id);  
}

colors_t colors_add (const colors_t colors, const size_t color_id)
{
  if (color_id > MAX_COLORS - 1)
    return colors;
  return (colors_set(color_id) | colors);  
}

colors_t colors_discard (const colors_t colors, const size_t color_id)
{
  if (color_id > MAX_COLORS - 1)
    return colors;
  return ((colors_full(MAX_COLORS) ^ colors_set(color_id)) & colors);  
}

bool colors_is_in (const colors_t colors, const size_t color_id)
{
  if (color_id > MAX_COLORS - 1)
    return false;
  return (colors_set(color_id) & colors);  
}

colors_t colors_negate (const colors_t colors)
{  
  return ~colors;  
}

colors_t colors_and (const colors_t colors1, const colors_t colors2)
{  
  return (colors1 & colors2);  
}

colors_t colors_or (const colors_t colors1, const colors_t colors2)
{      
  return (colors1 | colors2);  
}

colors_t colors_xor (const colors_t colors1, const colors_t colors2)
{
  return (colors1 ^ colors2);  
}

colors_t colors_subtract (const colors_t colors1, const colors_t colors2)
{  
  return colors_xor(colors1, colors_and(colors1, colors2));  
}

bool colors_is_equal (const colors_t colors1, const colors_t colors2)
{  
  return (colors_xor(colors1, colors2) == 0);  
}

bool colors_is_subset (const colors_t colors1, const colors_t colors2)
{  
  return (colors_and(colors1, colors2) == colors1);  
}

bool colors_is_singleton (const colors_t colors)
{  
  if (colors == 0)
    return false;    
  return (colors_and(colors, colors - 1) == 0);  
}

size_t colors_count (const colors_t colors)
{  
  colors_t x = colors;
  colors_t b5 = ~((-1ULL) << 32);
  colors_t b4 = b5 ^ (b5 << 16);
  colors_t b3 = b4 ^ (b4 << 8);
  colors_t b2 = b3 ^ (b3 << 4);
  colors_t b1 = b2 ^ (b2 << 2);
  colors_t b0 = b1 ^ (b1 << 1);

  x = ((x >> 1) & b0) + (x & b0);
  x = ((x >> 2) & b1) + (x & b1);
  x = ((x >> 4) + x) & b2;
  x = ((x >> 8) + x) & b3;
  x = ((x >> 16) + x) & b4;
  x = ((x >> 32) + x) & b5;
  return (size_t) x;  
}

colors_t colors_rightmost (const colors_t colors)
{
  if (colors == 0)
    return colors_empty();  
  return colors_and(colors, colors_negate(colors - 1));  
}

colors_t colors_leftmost (const colors_t colors)
{
  if (colors == 0)
    return colors_empty();
        
  colors_t clrs = colors;     
  size_t msb = 0;
  clrs = clrs >> 1;
  while (clrs != 0) {
    clrs = clrs >> 1;
    msb++;
  }
  return colors_set(msb);
}

/* We should initialize PRNG for this function */
colors_t colors_random (const colors_t colors)
{
  if (colors == 0)
    return colors_empty();
      
  while (true){
    size_t color_id = rand() % MAX_COLORS;
    if (colors_is_in(colors, color_id))
      return colors_set(color_id);
  }
}

bool subgrid_consistency (colors_t subgrid[], const size_t size)
{
  if (!subgrid)
    return false;
    
  for (size_t i = 0; i < size; i++){
    if (subgrid[i] == colors_empty())
      return false;
          
    if (colors_is_singleton(subgrid[i]))
      for (size_t j = i + 1; j < size; j++)
        if (colors_is_equal(subgrid[i], subgrid[j]))
          return false;              
  }
  
  for (size_t i = 0; i < size; i++){
    bool color_exist = false;
    for (size_t j = 0; j < size; j++)
      if (colors_is_in(subgrid[j], i)){
        color_exist = true;
        break;
      }
    if (!color_exist)
      return false;               
  }  
  return true;  
}

bool subgrid_heuristics (colors_t *subgrid[], size_t size)
{ 
  if (!subgrid)
    return false;
    
  return naked_subset(subgrid, size) | hidden_subset(subgrid, size) | 
                   cross_hatching(subgrid, size) | lone_number(subgrid, size);
}

bool cross_hatching (colors_t *subgrid[], size_t size)
{
  if (!subgrid)
    return false;
    
  bool changed = false;
  colors_t colors = colors_empty();
  for (size_t i = 0; i < size; i++)
    if (colors_is_singleton(*subgrid[i]))
      colors = colors_or(colors, *subgrid[i]);
  
  for (size_t i = 0; i < size; i++) {
    if (!colors_is_singleton(*subgrid[i])) {
      colors_t new_colors = colors_subtract(*subgrid[i], colors);
      if (!colors_is_equal(*subgrid[i], new_colors)) {
        changed = true;
        *subgrid[i] = new_colors;
      }
    }
  }
  return changed;            
}

bool lone_number (colors_t *subgrid[], size_t size)
{
  if (!subgrid)
    return false;
  
  bool changed = false;
  colors_t appeared_colors = colors_empty();
  colors_t repeated_colors = colors_empty();
  for (size_t i = 0; i < size; i++) {
    repeated_colors = colors_or(repeated_colors, 
                                colors_and(appeared_colors, *subgrid[i]));
    appeared_colors = colors_or(appeared_colors, *subgrid[i]); 
  }
  
  colors_t loned_colors = colors_subtract(appeared_colors, repeated_colors);
  for (size_t i = 0; i < size; i++)
    if (!colors_is_singleton(*subgrid[i])){ 
      colors_t new_colors = colors_and(loned_colors, *subgrid[i]);
      if (new_colors != 0 ) {
        changed = true;
        *subgrid[i] = colors_rightmost(new_colors);
      }
    } 
  
  return changed;
}

bool naked_subset (colors_t *subgrid[], size_t size)
{
  if (!subgrid)
    return false;
    
  bool changed = false;
  for (size_t i = 0; i < size; i++)
    if (!colors_is_singleton(*subgrid[i])){
      size_t n = 1;
      for (size_t j = i + 1; j < size; j++)  
        if (colors_is_equal(*subgrid[i], *subgrid[j]))
          n++; 
    
      if (n == colors_count(*subgrid[i]))
        for (size_t k = 0; k < size; k++)
          if (!colors_is_singleton(*subgrid[k]) && *subgrid[i] != *subgrid[k]){
            colors_t color = colors_subtract(*subgrid[k], *subgrid[i]);
            if (!colors_is_equal(*subgrid[k], color)){
              *subgrid[k] = color;
              changed = true;
            }
          }         
    }    
  return changed; 
}

bool hidden_subset (colors_t *subgrid[], size_t size)
{
  if (!subgrid)
    return false;
      
  bool changed = false;
  for (size_t i = 0; i < size; i++)
    if (!colors_is_singleton(*subgrid[i])){
      size_t n = 0;
      for (size_t j = 0; j < size; j++) 
        if (colors_and(*subgrid[i], *subgrid[j]))
          n++;
        
      if (n == colors_count(*subgrid[i]))
        for (size_t k = 0; k < size; k++) {
          colors_t colors = colors_and(*subgrid[i], *subgrid[k]);
          if (*subgrid[k] != colors && !colors_is_singleton(*subgrid[k]) 
              && colors){
             *subgrid[k] = colors;
             changed = true;
          }
        }
    }
  return changed;
}

