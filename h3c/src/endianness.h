#pragma once

#if defined(_MSC_VER)
#  define bswap16(x) _byteswap_ushort((x))
#  define bswap32(x) _byteswap_ulong((x))
#  define bswap64(x) _byteswap_uint64((x))
#elif defined(__GNUC__) || defined(__clang__)
#  define bswap16(x) __builtin_bswap16((x))
#  define bswap32(x) __builtin_bswap32((x))
#  define bswap64(x) __builtin_bswap64((x))
#else
#  error "Unsupported compiler"
#endif

#if defined(H3C_LITTLE_ENDIAN)
#  define ntoh16(x) bswap16((x))
#  define hton16(x) bswap16((x))
#  define ntoh32(x) bswap32((x))
#  define hton32(x) bswap32((x))
#  define ntoh64(x) bswap64((x))
#  define hton64(x) bswap64((x))
#elif defined(H3C_BIG_ENDIAN)
#  define ntoh16(x) (x)
#  define hton16(x) (x)
#  define ntoh32(x) (x)
#  define hton32(x) (x)
#  define ntoh64(x) (x)
#  define hton64(x) (x)
#else
#  error "Endianness not defined"
#endif
