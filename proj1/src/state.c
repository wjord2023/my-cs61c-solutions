#include "state.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row,
                         unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);

/* Task 1 */
game_state_t *create_default_state() {
  // TODO: Implement this function.
  const unsigned int init_num_board_rows = 18;
  const unsigned int init_num_board_cols = 20;
  // clang-format off
  const char *init_board[] =
  { "####################",
    "#                  #",
    "# d>D    *         #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "####################",
  };
  // clang-format on

  char **board = (char **)malloc(sizeof(char *) * init_num_board_rows);
  for (int i = 0; i < init_num_board_rows; i++) {
    board[i] = (char *)malloc(sizeof(char) * (init_num_board_cols + 1));
    strcpy(board[i], init_board[i]);
  }

  snake_t *snake_ptr = (snake_t *)malloc(sizeof(snake_t));
  snake_ptr[0].tail_col = 2, snake_ptr[0].tail_row = 2,
  snake_ptr[0].head_col = 4, snake_ptr[0].head_row = 2,
  snake_ptr[0].live = true;

  game_state_t *game_state_ptr = (game_state_t *)malloc(sizeof(game_state_t));
  game_state_ptr->snakes = snake_ptr;
  game_state_ptr->num_snakes = 1;
  game_state_ptr->board = board;
  game_state_ptr->num_rows = init_num_board_rows;

  return game_state_ptr;
}

/* Task 2 */
void free_state(game_state_t *state) {
  // TODO: Implement this function.
  for (int i = 0; i < state->num_rows; i++) {
    free(state->board[i]);
  }
  free(state->board);
  free(state->snakes);
  free(state);
  return;
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp) {
  // TODO: Implement this function.
  for (int i = 0; i < state->num_rows; i++) {
    fprintf(fp, "%s\n", state->board[i]);
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t *state, unsigned int row,
                         unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  // TODO: Implement this function.
  if (strchr("wasd", c))
    return true;
  else
    return false;
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  // TODO: Implement this function.
  if (strchr("WASDx", c))
    return true;
  else
    return false;
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  // TODO: Implement this function.
  if (strchr("wasd^<v>WASDx", c))
    return true;
  else
    return false;
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  // TODO: Implement this function.
  switch (c) {
  case '>':
    return 'd';
  case '<':
    return 'a';
  case '^':
    return 'w';
  case 'v':
    return 's';
  default:
    return 0;
  }
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  // TODO: Implement this function.
  switch (c) {
  case 'D':
    return '>';
  case 'A':
    return '<';
  case 'W':
    return '^';
  case 'S':
    return 'v';
  default:
    return 0;
  }
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  // TODO: Implement this function.
  if (strchr("vsS", c))
    return cur_row + 1;
  if (strchr("^wW", c))
    return cur_row - 1;
  return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  // TODO: Implement this function.
  if (strchr(">dD", c))
    return cur_col + 1;
  if (strchr("<aA", c))
    return cur_col - 1;
  return cur_col;
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake
  is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  unsigned int head_col = state->snakes[snum].head_col;
  unsigned int head_row = state->snakes[snum].head_row;
  char head = get_board_at(state, head_row, head_col);

  unsigned int next_col = get_next_col(head_col, head);
  unsigned int next_row = get_next_row(head_row, head);
  char next = get_board_at(state, next_row, next_col);

  return next;
}
/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the
  head.
*/
static void update_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  snake_t *snake = state->snakes + snum;
  char head = get_board_at(state, snake->head_row, snake->head_col);

  set_board_at(state, snake->head_row, snake->head_col, head_to_body(head));

  snake->head_col = get_next_col(snake->head_col, head);
  snake->head_row = get_next_row(snake->head_row, head);

  set_board_at(state, snake->head_row, snake->head_col, head);
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  snake_t *snake = state->snakes + snum;
  char tail = get_board_at(state, snake->tail_row, snake->tail_col);

  set_board_at(state, snake->tail_row, snake->tail_col, ' ');

  snake->tail_col = get_next_col(snake->tail_col, tail);
  snake->tail_row = get_next_row(snake->tail_row, tail);

  char new_tail = get_board_at(state, snake->tail_row, snake->tail_col);

  set_board_at(state, snake->tail_row, snake->tail_col, body_to_tail(new_tail));
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
  // TODO: Implement this function.
  for (unsigned int i = 0; i < state->num_snakes; i++) {
    char next = next_square(state, i);
    snake_t *snake = state->snakes + i;
    if (next == '#' || is_snake(next)) {
      snake->live = false;
      set_board_at(state, snake->head_row, snake->head_col, 'x');
    } else if (next == '*') {
      update_head(state, i);
      add_food(state);
    } else if (next == ' ') {
      update_head(state, i);
      update_tail(state, i);
    } else {
      assert(0);
    }
  }
  return;
}

/* Task 5.1 */
char *read_line(FILE *fp) {
  // TODO: Implement this function.
  size_t size = 16;
  char *buffer = (char *)malloc(sizeof(char) * size);
  if (buffer == NULL)
    return NULL;

  size_t len = 0;
  while (fgets(buffer + len, (int)(size - len), fp)) {
    len = strlen(buffer);
    if (buffer[len - 1] == '\n' || feof(fp)) {
      return buffer;
    }

    size *= 2;
    char *new_buffer = (char *)realloc(buffer, sizeof(char) * size);
    if (new_buffer == NULL) {
      free(buffer);
      return NULL;
    }

    buffer = new_buffer;
  }

  if (len == 0) {
    free(buffer);
    return NULL;
  }
  return buffer;
}

/* Task 5.2 */
game_state_t *load_board(FILE *fp) {
  // TODO: Implement this function.
  size_t size = 16;
  char **board = (char **)malloc(sizeof(char *) * size);
  if (board == NULL)
    return NULL;

  unsigned int num_rows = 0;
  char *line = read_line(fp);
  while (line) {
    if (num_rows >= size) {
      size *= 2;
      char **temp_board = (char **)realloc(board, sizeof(char *) * size);
      if (temp_board == NULL) {
        for (size_t i = 0; i < num_rows; i++) {
          free(board[i]);
        }
        free(board);
        free(line);
        return NULL;
      }
      board = temp_board;
    }

    size_t line_size = strlen(line) + 1;
    if (line[line_size - 2] == '\n') {
      line[line_size - 2] = '\0';
    }
    board[num_rows] = (char *)malloc(sizeof(char) * line_size);
    if (board[num_rows] == NULL) {
      for (size_t i = 0; i < num_rows; i++) {
        free(board[i]);
      }
      free(board);
      free(line);
      return NULL;
    }
    strcpy(board[num_rows], line);
    free(line);

    num_rows++;
    line = read_line(fp);
  }

  game_state_t *state = (game_state_t *)malloc(sizeof(game_state_t));
  if (state == NULL) {
    for (size_t i = 0; i < num_rows; i++) {
      free(board[i]);
    }
    free(board);
    return NULL;
  }

  state->board = board;
  state->num_rows = num_rows;
  state->snakes = NULL;
  state->num_snakes = 0;
  return state;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  snake_t *snake = state->snakes + snum;
  unsigned int cur_row = snake->tail_row;
  unsigned int cur_col = snake->tail_col;
  while (true) {
    char cur = state->board[cur_row][cur_col];
    unsigned int next_col = get_next_col(cur_col, cur);
    unsigned int next_row = get_next_row(cur_row, cur);
    char next = state->board[next_row][next_col];
    if (strchr("WASD", next)) {
      snake->head_row = next_row;
      snake->head_col = next_col;
      return;
    }
    cur_row = next_row;
    cur_col = next_col;
  }
}

/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state) {
  // TODO: Implement this function.
  unsigned int init_snake_num = 4;
  snake_t *snakes = (snake_t *)malloc(sizeof(snake_t) * init_snake_num);
  state->snakes = snakes;

  unsigned int snakes_count = 0;
  for (unsigned int i = 0; i < state->num_rows; i++) {
    size_t row_length = strlen(state->board[i]);
    for (unsigned int j = 0; j < row_length; j++) {
      if (strchr("wasd", state->board[i][j])) {
        if (snakes_count >= init_snake_num) {
          init_snake_num *= 2;
          snake_t *new_snakes =
              (snake_t *)realloc(snakes, sizeof(snake_t) * init_snake_num);
          if (new_snakes == NULL) {
            free(snakes);
            return NULL;
          }
          snakes = new_snakes;
          state->snakes = snakes;
        }
        snakes[snakes_count].tail_col = j;
        snakes[snakes_count].tail_row = i;
        snakes[snakes_count].live = true;
        find_head(state, snakes_count);
        snakes_count++;
      }
    }
  }

  state->num_snakes = snakes_count;
  state->snakes = snakes;
  return state;
}
