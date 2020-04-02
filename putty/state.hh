#ifndef PUTTY_STATE_HH_INCLUDE
#define PUTTY_STATE_HH_INCLUDE

#include <putty/cell.hh>
#include <putty/row.hh>
#include <putty/palette.hh>

namespace Putty
{

  struct Cursor
  {
    int x{};
    int y{};
    bool enabled{};
  };

  /**
   * Terminal state object. Contains the content of a terminal
   * at any given time.
   */
  struct State
  {
    int width{};
    int height{};
    int buffer_size{};
    bool alternate_screen{};
    Cursor cursor;
    std::vector< Row > rows;
    Palette palette;
  };

  static inline bool is_valid_row(const State &state, size_t row)
  {
    size_t row_index = row + state.buffer_size;
    return row_index < state.rows.size();
  }

  static inline const Row *get_row(const State &state, size_t row)
  {
    if (!is_valid_row(state, row))
      return nullptr;

    return &state.rows[row + state.buffer_size];
  }

  static inline bool is_valid_cell(const State &state, size_t row, size_t col)
  {
    return is_valid_row(state, row) && (col < state.rows[row + state.buffer_size].cells.size());
  }

  static inline const Cell *get_cell(const State &state, size_t row, size_t col)
  {
    if (!is_valid_cell(state, row, col))
      return nullptr;

    return &state.rows[row + state.buffer_size].cells[col];
  }

} // namespace Putty

#endif // PUTTY_STATE_HH_INCLUDE
