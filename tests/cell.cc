#include <gtest/gtest.h>

#include <putty/cell.hh>

using namespace Putty;


TEST(PuttyCell, FlagAttributes)
{
  Cell cell{L"", to_underlying(CellFlags::NARROW) | to_underlying(CellFlags::BOLD) | to_underlying(CellFlags::REVERSE)};
  ASSERT_TRUE(narrow(cell));
  ASSERT_FALSE(wide(cell));
  ASSERT_TRUE(bold(cell));
  ASSERT_FALSE(under(cell));
  ASSERT_TRUE(reverse(cell));
  ASSERT_FALSE(blink(cell));

  cell = {L"", to_underlying(CellFlags::WIDE) | to_underlying(CellFlags::UNDER) | to_underlying(CellFlags::BLINK)};
  ASSERT_FALSE(narrow(cell));
  ASSERT_TRUE(wide(cell));
  ASSERT_FALSE(bold(cell));
  ASSERT_TRUE(under(cell));
  ASSERT_FALSE(reverse(cell));
  ASSERT_TRUE(blink(cell));
}

TEST(PuttyCell, Foreground)
{
  // Check XTerm foreground color with value=29
  Cell cell{L"", (29) << CellHelpers::FgColorShift};
  ASSERT_EQ(29, foreground_code(cell));
  ASSERT_EQ(0, background_code(cell));
}

TEST(PuttyCell, Background)
{
  // Check XTerm background color with value=65
  Cell cell{L"", (65) << CellHelpers::BgColorShift};
  ASSERT_EQ(0, foreground_code(cell));
  ASSERT_EQ(65, background_code(cell));
}
