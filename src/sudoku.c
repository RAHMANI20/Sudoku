#include "sudoku.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include <string.h>
#include <getopt.h>
#include <err.h>
#include <time.h>

#include <grid.h>

typedef enum { mode_first, mode_all } mode_t;
static bool verbose = false;
size_t fill_ratio;
size_t count_choice = 0;
size_t count_backtrack = 0;
FILE* stream;
bool solver = true;
size_t count_solution;
static grid_t *file_parser (char *filename)
{  
  FILE *fd = fopen(filename, "r");
  if (fd == NULL)
  {
    errx(EXIT_FAILURE, "error: failed open file");
  }
  
  grid_t *grid;
  char first_row[MAX_GRID_SIZE];
  int ch;
  size_t line = 1;
  size_t column = 0;
  bool comment = false;
    
  while ((ch = fgetc(fd)) != EOF )
  {      
    switch (ch){
      case '#':
        comment = true;
        break;
          
      case '\n':
        if (comment){
          comment = false;
          if (column > 0){
            goto exit_loop1;
          }
        }
        else if (column > 0){
          goto exit_loop1;
        }
        line++;
        break;
          
      default:
        if ((ch != ' ') && (ch != '\t') && (!comment)){
          first_row[column] = ch;
          column++;
        }
        break;        
    }
  }
  exit_loop1:

  if (column == 0){
    warnx("error: %s: no grid found!", filename);
    fclose(fd);
    return NULL;
  }

  if (!grid_check_size(column)){
    warnx("error: size is not supported!");
    fclose(fd);
    return NULL;

  }

  grid = grid_alloc(column);
  if (!grid)
    errx(EXIT_FAILURE, "error: memory allocation failure!");

  for (size_t j = 0; j < column; j++){
    if (grid_check_char(grid, first_row[j])){
      grid_set_cell(grid, 0, j, first_row[j]);
    }
    else{
      warnx("error: wrong character %c at line %ld!", first_row[j], line);
      grid_free(grid);
      fclose(fd);
      return NULL;
    }
  }

  bool eofexit = false;
  size_t row = 1;
  column = 0;
  line++;

  while ((ch = fgetc(fd)) != EOF){
    
    switch (ch){
      case '#':
        comment = true;
        break;
        
      case '\n':
        eofexit = false;
        if (column > 0 && column < grid_get_size(grid)){
          warnx("error: line %ld is malformed! (wrong number of comlumns)",
                line);
          grid_free(grid);
          fclose(fd);
          return NULL;
        }
        else if (comment){
          comment = false;
          if (column > 0){
            row++;
            column = 0;
          }
        }
        else if (column > 0){
          row++;
          column = 0;
        }
        line++;
        continue;
        break;    
          
      default:
        if ((ch != ' ') && (ch != '\t') && (!comment)){
          if (!grid_check_char(grid, ch)){
            warnx("error: wrong character %c at line %ld!", ch, line);
            grid_free(grid);
            fclose(fd);
            return NULL;
          }
          else if (column >= grid_get_size(grid)){
            warnx("error: line %ld is malformed! (wrong number of columns)",
                  line);
            grid_free(grid);
            fclose(fd);
            return NULL;
          }
          else if (row >= grid_get_size(grid)){
            warnx("error: grid does not have the right number of rows!");
            grid_free(grid);
            fclose(fd);
            return NULL;
          }
          else{
            grid_set_cell(grid, row, column, ch);
            column++;
          }
        }
        break;           
    }      
    eofexit = true;
  }

  if (eofexit){
    if ((column == grid_get_size(grid)) && (row == grid_get_size(grid) - 1)){
      row++;
    }
    else if ((column > 0 ) && (column < grid_get_size(grid)) && 
    (row == grid_get_size(grid) - 1))
    {
      warnx("error: line %ld is malformed! (wrong number of comlumns)", line);
      grid_free(grid);
      fclose(fd);
      return NULL;
    }
  }

  if (row < grid_get_size(grid)){
    warnx("error: grid has %ld missing line(s)!", grid_get_size(grid) - row);
    grid_free(grid);
    fclose(fd);
    return NULL;
  }
  fclose(fd);
  return grid;
}

static grid_t *grid_solver (grid_t *grid, const mode_t mode)
{
  if (!grid)
    return NULL;
            
  size_t heuristic_return = grid_heuristics(grid);
  if (heuristic_return == 2){
    grid_free(grid);
    return NULL;
  }
  else if (heuristic_return == 1){  
    if (mode == mode_all){
      printf("Solutuon #%ld:\n", count_solution);
      grid_print(grid, stream);
      count_solution++;
    }
    return grid;
  }
  count_backtrack++;          
  choice_t *choice = grid_choice(grid);
  grid_t *last_solved = NULL;
  while (!grid_choice_is_empty(choice)){
    count_choice++;
    grid_t *copy_grid = grid_copy(grid);
    if (!copy_grid)
      return NULL;
      
    grid_choice_apply(copy_grid, choice);    
    grid_t *solved = grid_solver(copy_grid, mode);
    if (solved){
      if (mode == mode_first){
        grid_choice_free(choice);
        grid_free(grid);
        return solved;
      }
      else{
        if (last_solved){
          grid_free(last_solved);
        }
        last_solved = solved;    
      }        
    } 
    grid_choice_discard(grid, choice);
    grid_choice_free(choice);
    if (!grid_is_consistent(grid)){
      grid_free(grid);
      if (mode == mode_first)
        return NULL;
      else
        return last_solved;  
    }  
    choice = grid_choice(grid);    
  }
  grid_choice_free(choice); 
  grid_free(grid);
  if (mode == mode_all)
    return last_solved;
    
  return NULL;
}

grid_t *check_solution (grid_t *grid)
{
  if (!grid)
    return NULL;
    
  size_t heuristic_return = grid_heuristics(grid);
  if (heuristic_return == 2){
    grid_free(grid);
    return NULL;
  }
  else if (heuristic_return == 1){  
    count_solution++;
    return grid;
  }          
  choice_t *choice = grid_choice(grid);
  grid_t *last_solved = NULL;
  while (!grid_choice_is_empty(choice)){
    grid_t *copy_grid = grid_copy(grid);
    if (!copy_grid)
      return NULL;
      
    grid_choice_apply(copy_grid, choice);    
    grid_t *solved = check_solution(copy_grid);
    if (count_solution > 1){
      if (solved){
        grid_free(solved);         
      }
      if (last_solved){
        grid_free(last_solved);         
      }
      grid_free(grid);
      grid_choice_free(choice);
      return NULL;
    }
    if (solved){
      last_solved = solved;
    }   
    grid_choice_discard(grid, choice);
    grid_choice_free(choice);
    if (!grid_is_consistent(grid)){
      grid_free(grid);
      return last_solved;  
    }
    choice = grid_choice(grid);    
  }
  grid_choice_free(choice); 
  grid_free(grid);
  return last_solved; 
}

void generator (size_t size, FILE *fd, bool unique)
{
  if (fd){
    size_t empties = size * size * 40 / 100; 
    grid_t *grid = NULL;
    grid_t *copy_grid = NULL;
    if (unique){
      do {
        if (grid)
          grid_free(grid);  
        grid = grid_random(size);
        if (!grid)
          errx(EXIT_FAILURE, "error: memory allocation failure!");
        grid = grid_solver(grid, mode_first);
        grid_remove_cells(grid, empties);
        copy_grid = grid_copy(grid);
        if (!copy_grid)
          errx(EXIT_FAILURE, "error: memory allocation failure!");
        count_solution = 0;
        copy_grid = check_solution(copy_grid);
      }while (copy_grid == NULL);
      fprintf(fd, "***** Generating a grid of size %ld "
                              "('unique' mode enabled)...\n\n", size);
    }else{
      grid = grid_random(size);
      if (!grid)
        errx(EXIT_FAILURE, "error: memory allocation failure!");
      grid = grid_solver(grid, mode_first);
      copy_grid = grid_copy(grid);
      if (!copy_grid)
        errx(EXIT_FAILURE, "error: memory allocation failure!");
      grid_remove_cells(grid, empties);
      fprintf(fd, "***** Generating a grid of size %ld...\n\n", size);
    }
    if (verbose){
      fprintf(fd, "Solution:\n");
      grid_print(copy_grid, fd);
      fprintf(fd, "\nGrid:\n");
    }
    grid_print(grid, fd);
    grid_free(grid);
    grid_free(copy_grid);
  }
}

int main (int argc, char* argv[])
{
  char *buffer;
  stream = stdout;
  int optc;
  bool all = false;
  bool unique = false;
  bool success = false;
  bool file_opened = false;
  size_t grid_size_generator = 9;
    
  const struct option long_options[] = {
    { "help" , no_argument, NULL, 'h' },
    { "version" , no_argument, NULL, 'V' },
    { "verbose" , no_argument, NULL, 'v' },
    { "output" , required_argument, NULL, 'o' },
    { "all" , no_argument, NULL, 'a' },
    { "unique" , no_argument, NULL, 'u' },
    { "generate" , optional_argument, NULL, 'g' },
    {  NULL , 0, NULL,  0 }
  };
  
  while ((optc = getopt_long (argc, argv, "o:g::hVvau", long_options, NULL))
         != -1)
    switch (optc){
      case 'h':
        buffer =
          "Usage: sudoku [-a | -o FILE | -v | -V | -h] FILE...\n"
	  "        sudoku -g[SIZE] [-u | -o FILE | -v | -V | -h]\n"
	  "Solve or generate Sudoku grids of various sizes "
	  "(1,4,9,16,25,36,49,64)\n\n"
	  "-a, --all		search for all possible solutions\n"
	  "-g[N], --generate[=N]	"
	  "generate a grid of size NxN (default:9)\n"
	  "-u, --unique		generate a frid with unique solution\n"
	  "-o FILE, --output FILE	write solution to FILE\n"
	  "-v, --verbose		verbose output\n"
	  "-V, --version		display version and exit\n"
	  "-h, --help		display this help and exit\n";
          fputs(buffer, stream);
	  success = true;
	  break;
			  
      case 'V':
        buffer =
          "sudoku %d.%d.%d\n"
	  "Solve/generate Sudoku grids"
	  " (possible sizes: 1,4,9,16,25,36,49,64)\n";
	  fprintf(stream, buffer, VERSION, SUBVERSION, REVISION);
	  success = true;
	  break;
			  
      case 'v':
        verbose = true;
	break;
			  
      case 'o':
        if (!file_opened){
          stream = fopen(optarg, "w");
          if (!stream){
          errx(EXIT_FAILURE, "error: invalid file %s", optarg);
	  }
	  file_opened = true;
        } 		        
	break;
			  
      case 'a':
        all = true;		    
        break;
			  
      case 'u':
        unique = true;
	break;
			  
      case 'g': 
        solver = false;
        if (optarg){
          if (strcmp(optarg, "1") && strcmp(optarg, "4") && strcmp(optarg, "9")
             && strcmp(optarg, "16") && strcmp(optarg, "25") && 
          strcmp(optarg, "36") && strcmp(optarg, "49") && strcmp(optarg, "64"))
          {
            errx(EXIT_FAILURE, "error: invalid gride size,"
                               "only accept (1,4,9,16,25,36,49,64)!");
          }
          
          grid_size_generator = atoi(optarg);
        }
        break;
        
      default:   
        errx(EXIT_FAILURE, "check syntax : './sudoku -h or ./sudoku --help'");
	break;
    }

  FILE *file;
  bool error = false;
  srand(time(NULL));
  if (solver){
    if (unique){
      warnx("waning: option 'unique' conflict with the solver mode"
            ", disabled!");
      warnx("waning: '-u, --unique' is useless without '-g, --generate'!");      
    }  
    if (optind > argc - 1 && !success){
      errx(EXIT_FAILURE, "error: no input grid given!");
    }
    
    for (int i = optind; i <= argc - 1; i++){
      fprintf(stream, "\n***** Solving '%s'... \n\n", argv[i]);
      if ((file = fopen(argv[i], "r")) == NULL){
        errx(EXIT_FAILURE, "error: file %s cannot be read!", argv[i]);
      }
      else{
        grid_t *grid = file_parser(argv[i]);
        if (grid != NULL){
          if (verbose){
            fill_ratio = grid_fill_ratio(grid);
            fprintf(stream, "Input grid:\n");
            grid_print(grid, stream);
          }
          if (!grid_is_consistent(grid)){
            warnx("sudoku-RAHMANI_Faical: error: initial grid is "
                  "inconsistent!\n\n");
            error = true;
            grid_free(grid);
          }
          else{
            grid_t *sol;
            if (!all){
              sol = grid_solver(grid, mode_first);
            }
            else{
              count_solution = 1;
              sol = grid_solver(grid, mode_all);
            }
            if (!sol){
              warnx("sudoku-RAHMANI_Faical: error: solver found this "
                    "grid inconsistent!\n\n");
              error = true;
            }
            else{
              if (!all){
                fprintf(stream, "Solutuon:\n");
                grid_print(sol, stream);
              }
              fprintf(stream, "The grid is solved!\n\n");
              if (verbose){ 
                char *statistics = 
                           "Statistics for the current run:\n"
                           "* Initial fill ratio: %ld%\n"
                           "* Number of choices: %ld\n"
                           "* Number of backtracks: %ld\n";  
                fprintf(stream, statistics, fill_ratio, count_choice,
                                                     count_backtrack);                                     
              }
              if (all)
                  fprintf(stream, "# Number of solutions: %ld\n", 
                                                     count_solution - 1);
              grid_free(sol);
            } 
          }    
        }else{
          error = true;
        }
      }
      fclose(file);
    }
  }else{
    if (all){
      warnx("waning: option 'all' conflict with the generator mode,"
            " disabling it!");
    }
    if (optind != argc)
      errx(EXIT_FAILURE, "error: no argument for generator mode");
              
    generator(grid_size_generator, stream, unique);
  }  
  fclose(stream);  
  if (error){
    return EXIT_FAILURE;    
  } 
  return EXIT_SUCCESS;
}

