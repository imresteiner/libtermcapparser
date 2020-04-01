#include <gtest/gtest.h>

#include <putty/termcapparser.hh>

using namespace Putty;

TEST(TestTermcapParser, Initialize)
{
  // Initial size of the parser should be 80x24
  TermcapParser parser("UTF-8");
  const State &state = parser.get_state();

  ASSERT_EQ(80, state.get_width());
  ASSERT_EQ(24, state.get_height());

  // Each cell should be filled with spaces initially:
  //   character = 0x20
  //   attribute = default bg and default fg
  for (int row = 0; row < (int)state.get_height(); ++row)
    {
      for (unsigned col = 0; col < state.get_width(); ++col)
        {
          const Cell *cell = state.get_cell(row, col);
          ASSERT_TRUE(cell);

          // Check character
          ASSERT_EQ(0x20, cell->characters[0]) << "Cell character not a space in row " << row << " col " << col;

          // Check other valid attributes
          ASSERT_EQ(0, cell->attributes & CellHelpers::FlagMask);
          ASSERT_EQ(256, foreground_code(*cell));
          ASSERT_EQ(258, background_code(*cell));
        }
    }
}

TEST(TestTermcapParser, InputData)
{
  const char testdata[] = "A\033[31mB\033[42mC\033[0mD";
  TermcapParser parser("UTF-8");
  parser.data_input(testdata, sizeof(testdata));

  const State &state = parser.get_state();

  ASSERT_EQ(L"A", state.get_cell(0, 0)->characters);
  ASSERT_EQ(L"B", state.get_cell(0, 1)->characters);
  ASSERT_EQ(L"C", state.get_cell(0, 2)->characters);
  ASSERT_EQ(L"D", state.get_cell(0, 3)->characters);

  ASSERT_EQ(256, foreground_code(*state.get_cell(0, 0)));
  ASSERT_EQ(258, background_code(*state.get_cell(0, 0)));

  ASSERT_EQ(1, foreground_code(*state.get_cell(0, 1)));
  ASSERT_EQ(258, background_code(*state.get_cell(0, 1)));

  ASSERT_EQ(1, foreground_code(*state.get_cell(0, 2)));
  ASSERT_EQ(2, background_code(*state.get_cell(0, 2)));

  ASSERT_EQ(256, foreground_code(*state.get_cell(0, 3)));
  ASSERT_EQ(258, background_code(*state.get_cell(0, 3)));
}

TEST(TestTermcapParser, Resize)
{
  TermcapParser parser("UTF-8");
  parser.set_terminal_size(100, 100);

  const State &state = parser.get_state();

  ASSERT_EQ(100, state.get_width());
  ASSERT_EQ(100, state.get_height());
}

/**
 * Some character-encoding sanity tests.
 */
TEST(TestTermcapParser, EncodingUTF8)
{
  TermcapParser parser("UTF-8");
  // Euro sign in utf-8
  parser.data_input("\xE2\x82\xAC", 3);
  ASSERT_EQ(0x20AC, parser.get_state().get_cell(0, 0)->characters[0]);

  // Another euro sign in three steps
  parser.data_input("\xE2", 1);
  ASSERT_EQ(L" ", parser.get_state().get_cell(0, 1)->characters);
  parser.data_input("\x82", 1);
  ASSERT_EQ(L" ", parser.get_state().get_cell(0, 1)->characters);
  parser.data_input("\xAC", 1);
  ASSERT_EQ(0x20AC, parser.get_state().get_cell(0, 1)->characters[0]);
}

TEST(TestTermcapParser, EncodingLatin2)
{
  TermcapParser parser("iso-8859-2");
  parser.data_input("\xd0", 1);
  ASSERT_EQ(0x110, parser.get_state().get_cell(0, 0)->characters[0]);
}

TEST(TestTermcapParser, EncodingLatin15)
{
  TermcapParser parser("iso-8859-15");
  // Euro sign in iso-8859-15
  parser.data_input("\xa4", 1);
  ASSERT_EQ(0x20AC, parser.get_state().get_cell(0, 0)->characters[0]);
}

/**
 * Ensure proper Putty configuration.
 */
TEST(TestTermcapParser, ColorConfiguration)
{
  static const char ansi_color_input[] = "\033[31;42mA";
  static const char xterm256_color_input[] = "\033[38;5;126;48;5;127mA";
  TermcapParser parser("UTF-8");

  // Check ANSI color
  parser.data_input(ansi_color_input, sizeof(ansi_color_input) - 1);
  ASSERT_EQ(1, foreground_code(*parser.get_state().get_cell(0, 0)));
  ASSERT_EQ(2, background_code(*parser.get_state().get_cell(0, 0)));

  // Check xterm 256 color
  parser.data_input(xterm256_color_input, sizeof(xterm256_color_input) - 1);
  ASSERT_EQ(126, foreground_code(*parser.get_state().get_cell(0, 1)));
  ASSERT_EQ(127, background_code(*parser.get_state().get_cell(0, 1)));
}

TEST(TestTermcapParser, BceConfiguration)
{
  static const char input[] = "\033[31mabc\033[Kdef";
  TermcapParser parser("UTF-8");

  parser.data_input(input, sizeof(input) - 1);
  ASSERT_EQ(1, foreground_code(*parser.get_state().get_cell(0, 35)));
}

TEST(TestTermcapParser, LFhasCRConfiguration)
{
  static const char input[] = "a\nb";
  TermcapParser parser_nolf("UTF-8");

  parser_nolf.data_input(input, sizeof(input) - 1);
  ASSERT_EQ(L"a", parser_nolf.get_state().get_cell(0, 0)->characters);
  ASSERT_EQ(L"b", parser_nolf.get_state().get_cell(1, 1)->characters);

  TermcapParser parser_lfcr("UTF-8");
  parser_lfcr.replace_standalone_linefeeds(true);
  parser_lfcr.data_input(input, sizeof(input) - 1);
  ASSERT_EQ(L"a", parser_lfcr.get_state().get_cell(0, 0)->characters);
  ASSERT_EQ(L"b", parser_lfcr.get_state().get_cell(1, 0)->characters);
}

TEST(TestTermcapParser, FilterControlSequence)
{
  const char testdata[] = "AB\033PCDEFGHIJKLM";
  TermcapParser parser("UTF-8");
  parser.data_input(testdata, sizeof(testdata));

  const State &state = parser.get_state();

  ASSERT_EQ(L"A", state.get_cell(0, 0)->characters);
  ASSERT_EQ(L"B", state.get_cell(0, 1)->characters);
  ASSERT_EQ(L"I", state.get_cell(0, 2)->characters);
  ASSERT_EQ(L"J", state.get_cell(0, 3)->characters);
}

TEST(TestTermcapParser, FilterConsecutiveControlSequences1)
{
  static const char input[] = "abc\033P+q1234\033P+q1234ef";
  TermcapParser parser("UTF-8");

  parser.data_input(input, sizeof(input) - 1);

  char data[6];
  for (unsigned col = 0; col < 5; ++col)
    data[col] = (char)parser.get_state().get_cell(0, col)->characters[0];
  data[5] = '\0';

  ASSERT_STREQ("abcef", data);
}

/**
 * Test device control string filter
 */
TEST(TestTermcapParser, FilterConsecutiveControlSequences2)
{
  static const char input[] = "abc\033P+q1234d\033P+q1234ef";
  TermcapParser parser("UTF-8");

  parser.data_input(input, sizeof(input) - 1);

  char data[7];
  for (unsigned col = 0; col < 6; ++col)
    data[col] = (char)parser.get_state().get_cell(0, col)->characters[0];
  data[6] = '\0';

  ASSERT_STREQ("abcdef", data);
}

TEST(TestTermcapParser, FilterControlSequenceBuffering1)
{
  const char testdata[] = "AB\033PCDEFGHIJKLM";
  TermcapParser parser("UTF-8");

  for (int i = 0; i < (int)sizeof(testdata); ++i)
    parser.data_input(testdata + i, 1);

  const State &state = parser.get_state();

  ASSERT_EQ(L"A", state.get_cell(0, 0)->characters);
  ASSERT_EQ(L"B", state.get_cell(0, 1)->characters);
  ASSERT_EQ(L"I", state.get_cell(0, 2)->characters);
  ASSERT_EQ(L"J", state.get_cell(0, 3)->characters);
}

TEST(TestTermcapParser, FilterControlSequenceBuffering2)
{
  const char testdata[] = "AB\033PCDEFGHIJKLM";
  TermcapParser parser("UTF-8");

  parser.data_input(testdata, 5);
  parser.data_input(testdata + 5, sizeof(testdata) - 5);

  const State &state = parser.get_state();

  ASSERT_EQ(L"A", state.get_cell(0, 0)->characters);
  ASSERT_EQ(L"B", state.get_cell(0, 1)->characters);
  ASSERT_EQ(L"I", state.get_cell(0, 2)->characters);
  ASSERT_EQ(L"J", state.get_cell(0, 3)->characters);
}

TEST(TestTermcapParser, FilterControlSequenceStartingWithControlSequence)
{
  const char testdata[] = "\033PCDEFGHIJKLM";
  TermcapParser parser("UTF-8");

  parser.data_input(testdata, 5);
  parser.data_input(testdata + 5, sizeof(testdata) - 5);

  const State &state = parser.get_state();

  ASSERT_EQ(L"I", state.get_cell(0, 0)->characters);
  ASSERT_EQ(L"J", state.get_cell(0, 1)->characters);
}

TEST(TestTermcapParser, FilterControlSequenceEscapeCharInTheMiddle)
{
  const char testdata[] = "AB\033PC\033EFGHIJKLM";
  TermcapParser parser("UTF-8");

  for (int i = 0; i < (int)sizeof(testdata); ++i)
    parser.data_input(testdata + i, 1);

  const State &state = parser.get_state();

  ASSERT_EQ(L"A", state.get_cell(0, 0)->characters);
  ASSERT_EQ(L"B", state.get_cell(0, 1)->characters);
  ASSERT_EQ(L"I", state.get_cell(0, 2)->characters);
  ASSERT_EQ(L"J", state.get_cell(0, 3)->characters);
}

/**
 * Buffering tests
 */
TEST(TestTermcapParser, Buffer)
{
  static const char input[] = "a\r\nb\r\nc";
  TermcapParser parser("UTF-8");
  parser.set_terminal_size(50, 2);

  parser.data_input(input, sizeof(input) - 1);

  ASSERT_EQ(1, parser.get_state().get_buffer_size());
  ASSERT_EQ(L"c", parser.get_state().get_cell(1, 0)->characters);
  ASSERT_EQ(L"b", parser.get_state().get_cell(0, 0)->characters);
  ASSERT_EQ(L"a", parser.get_state().get_cell(-1, 0)->characters);
}

TEST(TestTermcapParser, ClearBuffer)
{
  static const char input[] = "a\r\nb\r\nc";
  TermcapParser parser("UTF-8");
  parser.set_terminal_size(50, 1);

  parser.data_input(input, sizeof(input) - 1);
  parser.clear_buffer();

  ASSERT_EQ(0, parser.get_state().get_buffer_size());
  ASSERT_EQ(L"c", parser.get_state().get_cell(0, 0)->characters);
}

/**
 * Line attribute tests
 */
TEST(TestTermcapParser, LineAttribute)
{
  // The line will have the top side of a double height line
  static const char input[] = "\033#3HELLO\r\n\033#4HELLO";
  TermcapParser parser("UTF-8");

  parser.data_input(input, sizeof(input) - 1);

  ASSERT_EQ(to_underlying(RowFlags::TOP), parser.get_state().get_row(0)->attributes);
  ASSERT_EQ(to_underlying(RowFlags::BOTTOM), parser.get_state().get_row(1)->attributes);
}

/**
 * Palette tests
 */
TEST(TestTermcapParser, PaletteSetColor)
{
  // This line should change the color value of 0 to #123456
  static const char input[] = "\033]P0123456";
  TermcapParser parser("UTF-8");

  parser.data_input(input, sizeof(input) - 1);

  ASSERT_EQ((RgbColor{0x12, 0x34, 0x56}), parser.get_palette().colors.at(0));
}

TEST(TestTermcapParser, PaletteReset)
{
  // This line should change the color value of 0 to #123456 and reset it afterwards
  static const char input[] = "\033]P0123456\033]R";
  TermcapParser parser("UTF-8");

  parser.data_input(input, sizeof(input) - 1);

  ASSERT_EQ((RgbColor{0, 0, 0}), parser.get_palette().colors.at(0));
}

/**
 * Check if the attributes are proper even in the buffer.
 */
TEST(TestTermcapParser, LattrBuffered)
{
  static const char input[] = "HELLO\r\n\033#3HELLO\r\nHELLO";
  TermcapParser parser("UTF-8");
  parser.set_terminal_size(100, 1);

  parser.data_input(input, sizeof(input) - 1);

  ASSERT_EQ(to_underlying(RowFlags::NORMAL), get_mode(*parser.get_state().get_row(0)));
  ASSERT_EQ(to_underlying(RowFlags::TOP), get_mode(*parser.get_state().get_row(-1)));
}

/**
 * Check if combining characters are displayed properly
 */
TEST(TestTermcapParser, Combining)
{
  static const char input[] = "I\xcc\x80I";
  TermcapParser parser("UTF-8");
  parser.set_terminal_size(3, 1);

  parser.data_input(input, sizeof(input) - 1);

  ASSERT_EQ(L"I\u0300", parser.get_state().get_cell(0, 0)->characters);
  ASSERT_EQ(L"I", parser.get_state().get_cell(0, 1)->characters);
}

namespace
{
  std::string last_log_message;
  void log_callback(const std::string &message)
  {
    last_log_message = message;
  }
}

TEST(TestTermcapParser, UpdateDisplayInvalidColumn)
{
  TermcapParser parser("UTF-8");
  parser.set_log_callback(log_callback);
  parser.update_display(parser.get_state().get_width(), 0, std::wstring(L"abc"), 0, 0);

  ASSERT_STREQ("Invalid position requested to be updated; row='0', col='82'", last_log_message.c_str());
}

TEST(TestTermcapParser, UpdateDisplayInvalidRow)
{
  TermcapParser parser("UTF-8");
  parser.set_log_callback(log_callback);
  parser.update_display(0, parser.get_state().get_height(), std::wstring(L"abc"), 0, 0);

  ASSERT_STREQ("Invalid row requested to be updated; row='24'", last_log_message.c_str());
}
