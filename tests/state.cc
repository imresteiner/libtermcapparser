#include <gtest/gtest.h>

#include <putty/state.hh>
#include <state_p.hh>

using namespace Putty;

class TestState : public ::testing::Test
{
public:
  TestState()
  {
    palette.colors[1] = RgbColor{2, 3, 4};

    resize(state, 1024, 768, 128);
    state.palette = palette;

    for (int row = -128; row != state.height; ++row)
      {
        for (int col = 0; col != state.width; ++col)
          {
            std::wstring characters;
            characters.push_back( (std::abs(row) << 8) | col );
            set_cell(state, row, col, characters, (col << 16) | std::abs(row));
          }
      }
  }

  State state;
  Palette palette;
};

TEST_F(TestState, Width)
{
  ASSERT_EQ(1024, state.width);
}

TEST_F(TestState, Height)
{
  ASSERT_EQ(768, state.height);
}

TEST_F(TestState, BufferSize)
{
  ASSERT_EQ(128, state.buffer_size);
}

TEST_F(TestState, GetCell)
{
  for (int row = -128; row != state.height; ++row)
    {
      for (int col = 0; col != state.width; ++col)
        {
          const Cell *cell = get_cell(state, row, col);
          ASSERT_TRUE(cell);
          ASSERT_EQ(wchar_t((std::abs(row) << 8) | col), cell->characters[0]) << "Invalid cell character in row " << row << " column " << col;
          ASSERT_EQ(((col << 16) | std::abs(row)) & CellHelpers::ValidMask, cell->attributes) << "Invalid cell attribute in row " << row << " column " << col;
        }
    }
}

TEST_F(TestState, GetCellForInvalidPosition)
{
  ASSERT_TRUE(get_cell(state, state.height - 1, 0));
  ASSERT_FALSE(get_cell(state, state.height, 0));

  ASSERT_FALSE(get_cell(state, 0, state.width));
  ASSERT_TRUE(get_cell(state, 0, state.width - 1));

  ASSERT_TRUE(get_cell(state, -state.buffer_size, 0));
  ASSERT_FALSE(get_cell(state, -state.buffer_size - 1, 0));
}

TEST_F(TestState, SetCellForInvalidPosition)
{
  ASSERT_FALSE(set_cell(state, state.height, 0, std::wstring(L"abc"), 0));
  ASSERT_FALSE(set_cell(state, 0, state.width, std::wstring(L"abc"), 0));
}

TEST_F(TestState, GetRow)
{
  for (int row = -128; row != (int)state.height; ++row)
    {
      const Row *termrow = get_row(state, row);
      ASSERT_TRUE(termrow);
      for (int col = 0; col != state.width; ++col)
        {
          const Cell &cell = termrow->cells.at(col);
          ASSERT_EQ(wchar_t((std::abs(row) << 8) | col), cell.characters[0]) << "Invalid cell character in row " << row << " column " << col;
          ASSERT_EQ(((col << 16) | std::abs(row)) & CellHelpers::ValidMask, cell.attributes) << "Invalid cell attribute in row " << row << " column " << col;
        }
    }
}

TEST_F(TestState, GetPalette)
{
  const Palette &pal = state.palette;

  ASSERT_EQ((RgbColor{2, 3, 4}), pal.colors.at(1));
}

TEST_F(TestState, ResizeBigger)
{
  int old_height = state.height;
  unsigned old_width = state.width;
  unsigned old_buffer_size = state.buffer_size;

  resize(state, 2048, 1024, 256);

  for (int row = -old_buffer_size; row != old_height; ++row)
    {
      const Row *termrow = get_row(state, row);
      ASSERT_TRUE(termrow);
      for (unsigned col = 0; col != old_width; ++col)
        {
          const Cell &cell = termrow->cells.at(col);
          ASSERT_EQ(wchar_t((std::abs(row) << 8) | col), cell.characters[0]) << "Invalid cell character in row " << row << " column " << col;
          ASSERT_EQ(((col << 16) | std::abs(row)) & CellHelpers::ValidMask, cell.attributes) << "Invalid cell attribute in row " << row << " column " << col;
        }
    }
}

TEST_F(TestState, ResizeSmaller)
{
  resize(state, 512, 128, 16);

  for (int row = -16; row != (int)state.height; ++row)
    {
      const Row *termrow = get_row(state, row);
      ASSERT_TRUE(termrow);
      for (int col = 0; col != state.width; ++col)
        {
          const Cell &cell = termrow->cells.at(col);
          ASSERT_EQ(wchar_t((std::abs(row) << 8) | col), cell.characters[0]) << "Invalid cell character in row " << row << " column " << col;
          ASSERT_EQ(((col << 16) | std::abs(row)) & CellHelpers::ValidMask, cell.attributes) << "Invalid cell attribute in row " << row << " column " << col;
        }
    }
}
