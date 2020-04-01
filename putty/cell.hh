#ifndef PUTTY_ATTRIBUTES_HH_INCLUDE
#define PUTTY_ATTRIBUTES_HH_INCLUDE

#include <string>
#include <type_traits>

#include <putty/palette.hh>

namespace Putty
{

  /**
   * Represents a cell in the display state of the terminal. A cell
   * contains a character represented as an unsigned long and the
   * cell attributes (foreground color, background color, bold, etc.).
   *
   * A cell is considered valid if the character is not 0, since the 0
   * character is not used to represent anything.
   */
  struct Cell
  {
    std::wstring characters;
    uint64_t attributes;
  };

  /**
   * Character attribute flags.
   */
  enum class CellFlags : uint64_t
  {
    NARROW = 0x800000U,
    WIDE = 0x400000U,
    BOLD = 0x040000U,
    UNDER = 0x080000U,
    REVERSE = 0x100000U,
    BLINK = 0x200000U,
  };

  /**
   * There are some special cases for color values. In these
   * cases we translate it to a "normal" RGB value.
   */
  enum class ColorSettings : uint8_t
  {
    DEFAULT_FOREGROUND = 0,
    DEFAULT_FOREGROUND_BOLD = 1,
    DEFAULT_BACKGROUND = 2,
    DEFAULT_BACKGROUND_BOLD = 3,
    CURSOR_FOREGROUND = 4,
    CURSOR_BACKGROUND = 5,

    COLOR_SETTING_COUNT,
  };

  template <typename E>
  constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
  }

  namespace CellHelpers
  {
    static constexpr inline int FgColorShift = 0;
    static constexpr inline int BgColorShift = 9;

    static constexpr inline uint64_t FlagMask = 0x00FC0000U;
    static constexpr inline uint64_t FgColorMask = 0x0001FFU;
    static constexpr inline uint64_t BgColorMask = 0x03FE00U;
    static constexpr inline uint64_t InvalidColor = 0x03FFFFU;
    static constexpr inline uint64_t ValidMask = FlagMask | BgColorMask | FgColorMask;
    static constexpr inline uint16_t DefaultForeground = 256 + to_underlying(ColorSettings::DEFAULT_FOREGROUND);
    static constexpr inline uint16_t DefaultBackground = 256 + to_underlying(ColorSettings::DEFAULT_BACKGROUND);
    static constexpr inline uint64_t DefaultAttributes = (DefaultForeground << FgColorShift) | (DefaultBackground << BgColorShift);
  }

  /** Return true if the character is narrow. */
  inline bool narrow(const Cell &cell)
  { return cell.attributes & to_underlying(CellFlags::NARROW); }

  /** Return true if the character is wide. */
  inline bool wide(const Cell &cell)
  { return cell.attributes & to_underlying(CellFlags::WIDE); }

  /** Return true if the character is bold. */
  inline bool bold(const Cell &cell)
  { return cell.attributes & to_underlying(CellFlags::BOLD); }

  /** Return true if the character is underscored. */
  inline bool under(const Cell &cell)
  { return cell.attributes & to_underlying(CellFlags::UNDER); }

  /** Return true if the character is reversed. */
  inline bool reverse(const Cell &cell)
  { return cell.attributes & to_underlying(CellFlags::REVERSE); }

  /** Return true if the character is blinking. */
  inline bool blink(const Cell &cell)
  { return cell.attributes & to_underlying(CellFlags::BLINK); }

  /** Return foreground color code of the cell. */
  inline uint16_t foreground_code(const Cell &cell)
  { return static_cast<uint16_t>((cell.attributes & CellHelpers::FgColorMask) >> CellHelpers::FgColorShift); }

  /** Return background color code of the cell. */
  inline uint16_t background_code(const Cell &cell)
  { return static_cast<uint16_t>((cell.attributes & CellHelpers::BgColorMask) >> CellHelpers::BgColorShift); }

} // namespace Putty

#endif // PUTTY_ATTRIBUTES_HH_INCLUDE
