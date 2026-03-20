// NOLINTBEGIN(*-include-cleaner)
#include "jsav/util/AnsiStyles.hpp"

namespace jsv::ansi {

    std::string styled(std::string_view text, std::string_view escape_code) { return FORMAT("{}{}{}", escape_code, text, kReset); }

    std::string red(std::string_view text) { return styled(text, kRed); }

    std::string yellow(std::string_view text) { return styled(text, kYellow); }

    std::string blue(std::string_view text) { return styled(text, kBlue); }

    std::string cyan(std::string_view text) { return styled(text, kCyan); }

    std::string green(std::string_view text) { return styled(text, kGreen); }

    std::string magenta(std::string_view text) { return styled(text, kMagenta); }

    std::string red_bold(std::string_view text) { return styled(text, FORMAT("{}{}", kBold, kRed)); }

    std::string blue_bold(std::string_view text) { return styled(text, FORMAT("{}{}", kBold, kBlue)); }

    std::string cyan_bold(std::string_view text) { return styled(text, FORMAT("{}{}", kBold, kCyan)); }

}  // namespace jsv::ansi
// NOLINTEND(*-include-cleaner)