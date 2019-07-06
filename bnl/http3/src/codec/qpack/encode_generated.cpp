#include <bnl/http3/header.hpp>

#include <xxhash.h>

#include <cstdint>
#include <utility>

namespace bnl {
namespace http3 {
namespace qpack {
namespace table {
namespace fixed {

enum class type { header_value, header_only, missing };

static std::pair<type, uint8_t> find_index(header_view header)
{
  uint64_t name_hash = XXH64(header.name().data(), header.name().size(), 0);
  uint64_t value_hash;

  switch (name_hash) {
    case 16895098521482341206U: // :authority
      return { type::header_only, uint8_t(0) };
    case 11800921616074793521U: // :method
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 1969084168692057419U: // CONNECT
          return { type::header_value, uint8_t(15) };
        case 3416820684017772195U: // DELETE
          return { type::header_value, uint8_t(16) };
        case 3137017853320178759U: // GET
          return { type::header_value, uint8_t(17) };
        case 5077457392360958184U: // HEAD
          return { type::header_value, uint8_t(18) };
        case 13449464674369688059U: // OPTIONS
          return { type::header_value, uint8_t(19) };
        case 1393561109570896241U: // POST
          return { type::header_value, uint8_t(20) };
        case 8278045464579020327U: // PUT
          return { type::header_value, uint8_t(21) };
      }
      return { type::header_only, uint8_t(15) };
    case 1432527143127418564U: // :path
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 16761507700594825962U: // /
          return { type::header_value, uint8_t(1) };
      }
      return { type::header_only, uint8_t(1) };
    case 16734001141632695329U: // :scheme
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 13730016396111666430U: // http
          return { type::header_value, uint8_t(22) };
        case 10742537861312705840U: // https
          return { type::header_value, uint8_t(23) };
      }
      return { type::header_only, uint8_t(22) };
    case 11145724770861396798U: // :status
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 2891813017576360282U: // 103
          return { type::header_value, uint8_t(24) };
        case 2685111111418367100U: // 200
          return { type::header_value, uint8_t(25) };
        case 2860949800888200221U: // 304
          return { type::header_value, uint8_t(26) };
        case 5957985645556640011U: // 404
          return { type::header_value, uint8_t(27) };
        case 12373623763172808772U: // 503
          return { type::header_value, uint8_t(28) };
        case 6146904730557115430U: // 100
          return { type::header_value, uint8_t(63) };
        case 11759795502426644276U: // 204
          return { type::header_value, uint8_t(64) };
        case 17033585193496081404U: // 206
          return { type::header_value, uint8_t(65) };
        case 1337354901647857481U: // 302
          return { type::header_value, uint8_t(66) };
        case 11111781720710347155U: // 400
          return { type::header_value, uint8_t(67) };
        case 14993518005534636743U: // 403
          return { type::header_value, uint8_t(68) };
        case 1128097706726417269U: // 421
          return { type::header_value, uint8_t(69) };
        case 17924379407345689140U: // 425
          return { type::header_value, uint8_t(70) };
        case 12667669577372699657U: // 500
          return { type::header_value, uint8_t(71) };
      }
      return { type::header_only, uint8_t(24) };
    case 14776963886974432617U: // accept
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 285960807216420690U: // */*
          return { type::header_value, uint8_t(29) };
        case 6373961066247414572U: // application/dns-message
          return { type::header_value, uint8_t(30) };
      }
      return { type::header_only, uint8_t(29) };
    case 14455144075924042224U: // accept-encoding
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 9032900040511447184U: // gzip, deflate, br
          return { type::header_value, uint8_t(31) };
      }
      return { type::header_only, uint8_t(31) };
    case 12586352021634702904U: // accept-language
      return { type::header_only, uint8_t(72) };
    case 14590555536192441974U: // accept-ranges
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 3279652670845457604U: // bytes
          return { type::header_value, uint8_t(32) };
      }
      return { type::header_only, uint8_t(32) };
    case 5881923794129529227U: // access-control-allow-credentials
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 13702020392090633634U: // FALSE
          return { type::header_value, uint8_t(73) };
        case 13992295306875567947U: // TRUE
          return { type::header_value, uint8_t(74) };
      }
      return { type::header_only, uint8_t(73) };
    case 3800686319607884068U: // access-control-allow-headers
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 6792126209451122079U: // cache-control
          return { type::header_value, uint8_t(33) };
        case 3910122139284074053U: // content-type
          return { type::header_value, uint8_t(34) };
        case 765293966243412708U: // *
          return { type::header_value, uint8_t(75) };
      }
      return { type::header_only, uint8_t(33) };
    case 10756748986426057606U: // access-control-allow-methods
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 2774841317834080824U: // get
          return { type::header_value, uint8_t(76) };
        case 13368036589511723433U: // get, post, options
          return { type::header_value, uint8_t(77) };
        case 16398749137410613924U: // options
          return { type::header_value, uint8_t(78) };
      }
      return { type::header_only, uint8_t(76) };
    case 9436417403599124064U: // access-control-allow-origin
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 765293966243412708U: // *
          return { type::header_value, uint8_t(35) };
      }
      return { type::header_only, uint8_t(35) };
    case 6796384088407939286U: // access-control-expose-headers
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 17031230962583399322U: // content-length
          return { type::header_value, uint8_t(79) };
      }
      return { type::header_only, uint8_t(79) };
    case 4242818872217734295U: // access-control-request-headers
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 3910122139284074053U: // content-type
          return { type::header_value, uint8_t(80) };
      }
      return { type::header_only, uint8_t(80) };
    case 14051338931492133901U: // access-control-request-method
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 2774841317834080824U: // get
          return { type::header_value, uint8_t(81) };
        case 3178169103122520677U: // post
          return { type::header_value, uint8_t(82) };
      }
      return { type::header_only, uint8_t(81) };
    case 5092756294692198100U: // age
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 7148434200721666028U: // 0
          return { type::header_value, uint8_t(2) };
      }
      return { type::header_only, uint8_t(2) };
    case 12092656254601729054U: // alt-svc
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 9623284816163994036U: // clear
          return { type::header_value, uint8_t(83) };
      }
      return { type::header_only, uint8_t(83) };
    case 8846816503791876372U: // authorization
      return { type::header_only, uint8_t(84) };
    case 6792126209451122079U: // cache-control
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 10032073942971583664U: // max-age=0
          return { type::header_value, uint8_t(36) };
        case 3044507098168884424U: // max-age=2592000
          return { type::header_value, uint8_t(37) };
        case 441477139842840241U: // max-age=604800
          return { type::header_value, uint8_t(38) };
        case 8897988708233174624U: // no-cache
          return { type::header_value, uint8_t(39) };
        case 3660001317594812985U: // no-store
          return { type::header_value, uint8_t(40) };
        case 9509473917879564042U: // public, max-age=31536000
          return { type::header_value, uint8_t(41) };
      }
      return { type::header_only, uint8_t(36) };
    case 17156266451297603303U: // content-disposition
      return { type::header_only, uint8_t(3) };
    case 11860813331948357247U: // content-encoding
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 12202383188974104177U: // br
          return { type::header_value, uint8_t(42) };
        case 12621183860903199653U: // gzip
          return { type::header_value, uint8_t(43) };
      }
      return { type::header_only, uint8_t(42) };
    case 17031230962583399322U: // content-length
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 7148434200721666028U: // 0
          return { type::header_value, uint8_t(4) };
      }
      return { type::header_only, uint8_t(4) };
    case 18043880115709070015U: // content-security-policy
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 165232284275397264U: // script-src 'none'; object-src 'none';
                                  // base-uri 'none'
          return { type::header_value, uint8_t(85) };
      }
      return { type::header_only, uint8_t(85) };
    case 3910122139284074053U: // content-type
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 6373961066247414572U: // application/dns-message
          return { type::header_value, uint8_t(44) };
        case 9423039217905102597U: // application/javascript
          return { type::header_value, uint8_t(45) };
        case 116355269824119741U: // application/json
          return { type::header_value, uint8_t(46) };
        case 15711274223642436716U: // application/x-www-form-urlencoded
          return { type::header_value, uint8_t(47) };
        case 12727226202374957143U: // image/gif
          return { type::header_value, uint8_t(48) };
        case 12200045138720289141U: // image/jpeg
          return { type::header_value, uint8_t(49) };
        case 12092489581388341001U: // image/png
          return { type::header_value, uint8_t(50) };
        case 6791077934204364231U: // text/css
          return { type::header_value, uint8_t(51) };
        case 8886412932112875563U: // text/html; charset=utf-8
          return { type::header_value, uint8_t(52) };
        case 16588200704567877914U: // text/plain
          return { type::header_value, uint8_t(53) };
        case 9119350762400467667U: // text/plain;charset=utf-8
          return { type::header_value, uint8_t(54) };
      }
      return { type::header_only, uint8_t(44) };
    case 905780767293800022U: // cookie
      return { type::header_only, uint8_t(5) };
    case 9202271988582577219U: // date
      return { type::header_only, uint8_t(6) };
    case 12326157660028381149U: // early-data
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 13237225503670494420U: // 1
          return { type::header_value, uint8_t(86) };
      }
      return { type::header_only, uint8_t(86) };
    case 6439525432329186999U: // etag
      return { type::header_only, uint8_t(7) };
    case 9568944059520352163U: // expect-ct
      return { type::header_only, uint8_t(87) };
    case 5644212460638603090U: // forwarded
      return { type::header_only, uint8_t(88) };
    case 12678664253902784264U: // if-modified-since
      return { type::header_only, uint8_t(8) };
    case 8836338662272140136U: // if-none-match
      return { type::header_only, uint8_t(9) };
    case 10086758965902305572U: // if-range
      return { type::header_only, uint8_t(89) };
    case 5803729767262815818U: // last-modified
      return { type::header_only, uint8_t(10) };
    case 17841021884467483934U: // link
      return { type::header_only, uint8_t(11) };
    case 14592406158319853782U: // location
      return { type::header_only, uint8_t(12) };
    case 7034821444952201446U: // origin
      return { type::header_only, uint8_t(90) };
    case 4274379369960325767U: // purpose
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 15048569516378897441U: // prefetch
          return { type::header_value, uint8_t(91) };
      }
      return { type::header_only, uint8_t(91) };
    case 12715602975004390753U: // range
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 6181996873389341388U: // bytes=0-
          return { type::header_value, uint8_t(55) };
      }
      return { type::header_only, uint8_t(55) };
    case 14154313121195327909U: // referer
      return { type::header_only, uint8_t(13) };
    case 16290507850670895904U: // server
      return { type::header_only, uint8_t(92) };
    case 666465872145128107U: // set-cookie
      return { type::header_only, uint8_t(14) };
    case 4179674550753269761U: // strict-transport-security
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 13034728143243030830U: // max-age=31536000
          return { type::header_value, uint8_t(56) };
        case 11632417432576482556U: // max-age=31536000; includesubdomains
          return { type::header_value, uint8_t(57) };
        case 16736049223785386895U: // max-age=31536000; includesubdomains;
                                    // preload
          return { type::header_value, uint8_t(58) };
      }
      return { type::header_only, uint8_t(56) };
    case 5374862646944647126U: // timing-allow-origin
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 765293966243412708U: // *
          return { type::header_value, uint8_t(93) };
      }
      return { type::header_only, uint8_t(93) };
    case 18069291607417813553U: // upgrade-insecure-requests
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 13237225503670494420U: // 1
          return { type::header_value, uint8_t(94) };
      }
      return { type::header_only, uint8_t(94) };
    case 16625730634708209664U: // user-agent
      return { type::header_only, uint8_t(95) };
    case 8816542506507529547U: // vary
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 14455144075924042224U: // accept-encoding
          return { type::header_value, uint8_t(59) };
        case 7034821444952201446U: // origin
          return { type::header_value, uint8_t(60) };
      }
      return { type::header_only, uint8_t(59) };
    case 6196956168349306122U: // x-content-type-options
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 4306527442339074562U: // nosniff
          return { type::header_value, uint8_t(61) };
      }
      return { type::header_only, uint8_t(61) };
    case 1042837635865018085U: // x-forwarded-for
      return { type::header_only, uint8_t(96) };
    case 8598773544412918900U: // x-frame-options
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 6492545016868164575U: // deny
          return { type::header_value, uint8_t(97) };
        case 13303836962186157521U: // sameorigin
          return { type::header_value, uint8_t(98) };
      }
      return { type::header_only, uint8_t(97) };
    case 15732293358711069708U: // x-xss-protection
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 7412498726110841680U: // 1; mode=block
          return { type::header_value, uint8_t(62) };
      }
      return { type::header_only, uint8_t(62) };
  }

  return { type::missing, {} };
}

} // namespace fixed
} // namespace table
} // namespace qpack
} // namespace http3
} // namespace bnl
