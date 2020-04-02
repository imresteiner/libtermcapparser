#ifndef PUTTY_STATE_P_HH_INCLUDE
#define PUTTY_STATE_P_HH_INCLUDE

#include <putty/state.hh>

namespace Putty
{

    static inline bool set_cell(State &state, size_t row, size_t col, const std::wstring &characters, uint64_t attributes)
    {
      if (!is_valid_cell(state, row, col))
        return false;

      state.rows[row + state.buffer_size].cells[col] = Cell{characters, attributes & CellHelpers::ValidMask};
      return true;
    }

    static inline void resize(State &state, int a_width, int a_height, int a_buffer_size)
    {
      // If the dimensions don't change, leave this alone
      if (a_width == state.width && state.height == a_height && state.buffer_size == a_buffer_size)
        return;

      std::vector<Row> new_rows;
      new_rows.resize(a_height + a_buffer_size);

      for (int i = -a_buffer_size; i < a_height; ++i)
        {
          // Copy lines from the original
          if (i >= -state.buffer_size && i < state.height)
            new_rows[i + a_buffer_size] = std::move(state.rows[i + state.buffer_size]);

          new_rows[i + a_buffer_size].cells.resize(a_width);
        }

      state.height = a_height;
      state.width = a_width;
      state.buffer_size = a_buffer_size;

      state.rows = std::move(new_rows);
    }

    static inline void resize_display(State &state, int width, int height)
    {
      resize(state, width, height, state.buffer_size);
    }

}

#endif // PUTTY_STATE_P_HH_INCLUDE
