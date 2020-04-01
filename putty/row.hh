#ifndef PUTTY_ROW_HH_INCLUDE
#define PUTTY_ROW_HH_INCLUDE

#include <putty/cell.hh>

#include <vector>
#include <cstdint>

namespace Putty
{

  /**
   * Represents a row in the display state of the terminal. A row
   * contains a number of cells and the row attributes.
   */
  struct Row
  {
    std::vector< Cell > cells;
    uint64_t attributes;
  };

  enum class RowFlags : uint64_t
  {
    NORMAL = 0x00000000UL,        /**< Normal text */
    WIDE = 0x00000001UL,          /**< Double width text */
    TOP = 0x00000002UL,           /**< Top half of double height text */
    BOTTOM = 0x00000003UL,        /**< Bottom half of double height text */
    MASK_MODE = 0x00000003UL,     /**< Text mode mask */

    WRAPPED = 0x00000010UL,       /**< The row wraps to the next one (is continued) */
    WRAPPED_WIDE = 0x00000020UL,  /**< Used with WRAPPED: wide character wrapped to next line */
  };

  inline uint64_t get_mode(const Row &row)
  { return row.attributes & to_underlying(RowFlags::MASK_MODE); }

  inline bool is_wrapped(const Row &row)
  { return row.attributes & to_underlying(RowFlags::WRAPPED); }

} // namespace Putty

#endif // PUTTY_LINE_HH_INCLUDE
