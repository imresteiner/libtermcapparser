#include <gtest/gtest.h>

#include <putty/state.hh>

using namespace Putty;

#define ABS(x) ((x) > 0 ? (x) : -(x))

class StateMock : public State
{
public:
  using State::set_cell;
  using State::resize;
  using State::set_palette;
};

class TestState : public ::testing::Test
{
public:
  void SetUp()
  {
    palette.colors[1] = RgbColor{2, 3, 4};

    state = new StateMock();
    state->resize(1024, 768, 128);
    state->set_palette(palette);

    for (int row = -128; row != (int)state->get_height(); ++row)
      {
        for (unsigned col = 0; col != state->get_width(); ++col)
          {
            std::wstring characters;
            characters.push_back( (ABS(row) << 8) | col );
            state->set_cell(row, col, characters, (col << 16) | ABS(row));
          }
      }
  }

  void TearDown()
  {
    delete state;
  }

  StateMock *state;
  Palette palette;
};

TEST_F(TestState, Width)
{
  ASSERT_EQ(1024, state->get_width());
}

TEST_F(TestState, Height)
{
  ASSERT_EQ(768, state->get_height());
}

TEST_F(TestState, BufferSize)
{
  ASSERT_EQ(128, state->get_buffer_size());
}

TEST_F(TestState, GetCell)
{
  for (int row = -128; row != (int)state->get_height(); ++row)
    {
      for (unsigned col = 0; col != state->get_width(); ++col)
        {
          const Cell *cell = state->get_cell(row, col);
          ASSERT_TRUE(cell);
          ASSERT_EQ(wchar_t((ABS(row) << 8) | col), cell->characters[0]) << "Invalid cell character in row " << row << " column " << col;
          ASSERT_EQ(((col << 16) | ABS(row)) & CellHelpers::ValidMask, cell->attributes) << "Invalid cell attribute in row " << row << " column " << col;
        }
    }
}

TEST_F(TestState, GetCellForInvalidPosition)
{
  ASSERT_TRUE(state->get_cell(state->get_height() - 1, 0));
  ASSERT_FALSE(state->get_cell(state->get_height(), 0));

  ASSERT_FALSE(state->get_cell(0, state->get_width()));
  ASSERT_TRUE(state->get_cell(0, state->get_width() - 1));

  ASSERT_TRUE(state->get_cell(-(int)state->get_buffer_size(), 0));
  ASSERT_FALSE(state->get_cell(-(int)state->get_buffer_size() - 1, 0));
}

TEST_F(TestState, SetCellForInvalidPosition)
{
  ASSERT_FALSE(state->set_cell(state->get_height(), 0, std::wstring(L"abc"), 0));
  ASSERT_FALSE(state->set_cell(0, state->get_width(), std::wstring(L"abc"), 0));
}

TEST_F(TestState, GetRow)
{
  for (int row = -128; row != (int)state->get_height(); ++row)
    {
      const Row *termrow = state->get_row(row);
      ASSERT_TRUE(termrow);
      for (unsigned col = 0; col != state->get_width(); ++col)
        {
          const Cell &cell = termrow->cells.at(col);
          ASSERT_EQ(wchar_t((ABS(row) << 8) | col), cell.characters[0]) << "Invalid cell character in row " << row << " column " << col;
          ASSERT_EQ(((col << 16) | ABS(row)) & CellHelpers::ValidMask, cell.attributes) << "Invalid cell attribute in row " << row << " column " << col;
        }
    }
}

TEST_F(TestState, GetPalette)
{
  const Palette &pal = state->get_palette();

  ASSERT_EQ((RgbColor{2, 3, 4}), pal.colors.at(1));
}

TEST_F(TestState, ResizeBigger)
{
  int old_height = state->get_height();
  unsigned old_width = state->get_width();
  unsigned old_buffer_size = state->get_buffer_size();

  state->resize(2048, 1024, 256);

  for (int row = -old_buffer_size; row != old_height; ++row)
    {
      const Row *termrow = state->get_row(row);
      ASSERT_TRUE(termrow);
      for (unsigned col = 0; col != old_width; ++col)
        {
          const Cell &cell = termrow->cells.at(col);
          ASSERT_EQ(wchar_t((ABS(row) << 8) | col), cell.characters[0]) << "Invalid cell character in row " << row << " column " << col;
          ASSERT_EQ(((col << 16) | ABS(row)) & CellHelpers::ValidMask, cell.attributes) << "Invalid cell attribute in row " << row << " column " << col;
        }
    }
}

TEST_F(TestState, ResizeSmaller)
{
  state->resize(512, 128, 16);

  for (int row = -16; row != (int)state->get_height(); ++row)
    {
      const Row *termrow = state->get_row(row);
      ASSERT_TRUE(termrow);
      for (unsigned col = 0; col != state->get_width(); ++col)
        {
          const Cell &cell = termrow->cells.at(col);
          ASSERT_EQ(wchar_t((ABS(row) << 8) | col), cell.characters[0]) << "Invalid cell character in row " << row << " column " << col;
          ASSERT_EQ(((col << 16) | ABS(row)) & CellHelpers::ValidMask, cell.attributes) << "Invalid cell attribute in row " << row << " column " << col;
        }
    }
}
