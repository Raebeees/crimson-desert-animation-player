#pragma once
#include <array>
#include <cstdint>
namespace boots204 {
struct HookEntryPin204 {const char* role;const char* moduleHash;std::uintptr_t rva;const char* hex;unsigned nearSpan,absoluteSpan;};
inline constexpr std::array<HookEntryPin204,5> hookEntryPins204{{
{"movement","c94ad6c3ea54970a6d2da7a2719753210a7728a91698ed3405dd9125470ed98f",0x4586350,"488bc44c89482048895010554156",7,14},
{"effect_update","c94ad6c3ea54970a6d2da7a2719753210a7728a91698ed3405dd9125470ed98f",0x57e020,"488bc4488948085553565741544155",7,15},
{"effect_retirement","c94ad6c3ea54970a6d2da7a2719753210a7728a91698ed3405dd9125470ed98f",0x8bdff10,"448844241889542410535556574154",5,15},
{"native_stop_not_hook","c94ad6c3ea54970a6d2da7a2719753210a7728a91698ed3405dd9125470ed98f",0x8bc7c90,"48895c241048896c24184889742420",5,15},
{"cc_selector","f82e8d845d131e3f355e92585649687aa8cb4928c6e7ec9d262b45a8bf050a08",0x358f0,"4053555657415641574883ec28448b7118",5,17},
}};
} // namespace boots204
