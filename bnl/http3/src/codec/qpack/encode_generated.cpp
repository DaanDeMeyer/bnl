#include <bnl/http3/header.hpp>

#include <xxhash.h>

#include <cstdint>

namespace bnl {
namespace http3 {
namespace qpack {
namespace static_table {

enum class index_type { missing, header_only, header_value };

static static_table::index_type find_index(header_view header, uint8_t *index)
{
  uint64_t name_hash = XXH64(header.name().data(), header.name().size(), 0);
  uint64_t value_hash;

  switch (name_hash) {
    case 16895098521482341206U: // :authority
      *index = 0;
      return index_type::header_only;
    case 11800921616074793521U: // :method
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 1969084168692057419U: // CONNECT
          *index = 15;
          return index_type::header_value;
        case 3416820684017772195U: // DELETE
          *index = 16;
          return index_type::header_value;
        case 3137017853320178759U: // GET
          *index = 17;
          return index_type::header_value;
        case 5077457392360958184U: // HEAD
          *index = 18;
          return index_type::header_value;
        case 13449464674369688059U: // OPTIONS
          *index = 19;
          return index_type::header_value;
        case 1393561109570896241U: // POST
          *index = 20;
          return index_type::header_value;
        case 8278045464579020327U: // PUT
          *index = 21;
          return index_type::header_value;
      }
      *index = 15;
      return index_type::header_only;
    case 1432527143127418564U: // :path
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 16761507700594825962U: // /
          *index = 1;
          return index_type::header_value;
      }
      *index = 1;
      return index_type::header_only;
    case 16734001141632695329U: // :scheme
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 13730016396111666430U: // http
          *index = 22;
          return index_type::header_value;
        case 10742537861312705840U: // https
          *index = 23;
          return index_type::header_value;
      }
      *index = 22;
      return index_type::header_only;
    case 11145724770861396798U: // :status
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 2891813017576360282U: // 103
          *index = 24;
          return index_type::header_value;
        case 2685111111418367100U: // 200
          *index = 25;
          return index_type::header_value;
        case 2860949800888200221U: // 304
          *index = 26;
          return index_type::header_value;
        case 5957985645556640011U: // 404
          *index = 27;
          return index_type::header_value;
        case 12373623763172808772U: // 503
          *index = 28;
          return index_type::header_value;
        case 6146904730557115430U: // 100
          *index = 63;
          return index_type::header_value;
        case 11759795502426644276U: // 204
          *index = 64;
          return index_type::header_value;
        case 17033585193496081404U: // 206
          *index = 65;
          return index_type::header_value;
        case 1337354901647857481U: // 302
          *index = 66;
          return index_type::header_value;
        case 11111781720710347155U: // 400
          *index = 67;
          return index_type::header_value;
        case 14993518005534636743U: // 403
          *index = 68;
          return index_type::header_value;
        case 1128097706726417269U: // 421
          *index = 69;
          return index_type::header_value;
        case 17924379407345689140U: // 425
          *index = 70;
          return index_type::header_value;
        case 12667669577372699657U: // 500
          *index = 71;
          return index_type::header_value;
      }
      *index = 24;
      return index_type::header_only;
    case 14776963886974432617U: // accept
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 285960807216420690U: // */*
          *index = 29;
          return index_type::header_value;
        case 6373961066247414572U: // application/dns-message
          *index = 30;
          return index_type::header_value;
      }
      *index = 29;
      return index_type::header_only;
    case 14455144075924042224U: // accept-encoding
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 9032900040511447184U: // gzip, deflate, br
          *index = 31;
          return index_type::header_value;
      }
      *index = 31;
      return index_type::header_only;
    case 12586352021634702904U: // accept-language
      *index = 72;
      return index_type::header_only;
    case 14590555536192441974U: // accept-ranges
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 3279652670845457604U: // bytes
          *index = 32;
          return index_type::header_value;
      }
      *index = 32;
      return index_type::header_only;
    case 5881923794129529227U: // access-control-allow-credentials
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 13702020392090633634U: // FALSE
          *index = 73;
          return index_type::header_value;
        case 13992295306875567947U: // TRUE
          *index = 74;
          return index_type::header_value;
      }
      *index = 73;
      return index_type::header_only;
    case 3800686319607884068U: // access-control-allow-headers
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 6792126209451122079U: // cache-control
          *index = 33;
          return index_type::header_value;
        case 3910122139284074053U: // content-type
          *index = 34;
          return index_type::header_value;
        case 765293966243412708U: // *
          *index = 75;
          return index_type::header_value;
      }
      *index = 33;
      return index_type::header_only;
    case 10756748986426057606U: // access-control-allow-methods
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 2774841317834080824U: // get
          *index = 76;
          return index_type::header_value;
        case 13368036589511723433U: // get, post, options
          *index = 77;
          return index_type::header_value;
        case 16398749137410613924U: // options
          *index = 78;
          return index_type::header_value;
      }
      *index = 76;
      return index_type::header_only;
    case 9436417403599124064U: // access-control-allow-origin
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 765293966243412708U: // *
          *index = 35;
          return index_type::header_value;
      }
      *index = 35;
      return index_type::header_only;
    case 6796384088407939286U: // access-control-expose-headers
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 17031230962583399322U: // content-length
          *index = 79;
          return index_type::header_value;
      }
      *index = 79;
      return index_type::header_only;
    case 4242818872217734295U: // access-control-request-headers
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 3910122139284074053U: // content-type
          *index = 80;
          return index_type::header_value;
      }
      *index = 80;
      return index_type::header_only;
    case 14051338931492133901U: // access-control-request-method
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 2774841317834080824U: // get
          *index = 81;
          return index_type::header_value;
        case 3178169103122520677U: // post
          *index = 82;
          return index_type::header_value;
      }
      *index = 81;
      return index_type::header_only;
    case 5092756294692198100U: // age
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 7148434200721666028U: // 0
          *index = 2;
          return index_type::header_value;
      }
      *index = 2;
      return index_type::header_only;
    case 12092656254601729054U: // alt-svc
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 9623284816163994036U: // clear
          *index = 83;
          return index_type::header_value;
      }
      *index = 83;
      return index_type::header_only;
    case 8846816503791876372U: // authorization
      *index = 84;
      return index_type::header_only;
    case 6792126209451122079U: // cache-control
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 10032073942971583664U: // max-age=0
          *index = 36;
          return index_type::header_value;
        case 3044507098168884424U: // max-age=2592000
          *index = 37;
          return index_type::header_value;
        case 441477139842840241U: // max-age=604800
          *index = 38;
          return index_type::header_value;
        case 8897988708233174624U: // no-cache
          *index = 39;
          return index_type::header_value;
        case 3660001317594812985U: // no-store
          *index = 40;
          return index_type::header_value;
        case 9509473917879564042U: // public, max-age=31536000
          *index = 41;
          return index_type::header_value;
      }
      *index = 36;
      return index_type::header_only;
    case 17156266451297603303U: // content-disposition
      *index = 3;
      return index_type::header_only;
    case 11860813331948357247U: // content-encoding
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 12202383188974104177U: // br
          *index = 42;
          return index_type::header_value;
        case 12621183860903199653U: // gzip
          *index = 43;
          return index_type::header_value;
      }
      *index = 42;
      return index_type::header_only;
    case 17031230962583399322U: // content-length
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 7148434200721666028U: // 0
          *index = 4;
          return index_type::header_value;
      }
      *index = 4;
      return index_type::header_only;
    case 18043880115709070015U: // content-security-policy
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 165232284275397264U: // script-src 'none'; object-src 'none';
                                  // base-uri 'none'
          *index = 85;
          return index_type::header_value;
      }
      *index = 85;
      return index_type::header_only;
    case 3910122139284074053U: // content-type
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 6373961066247414572U: // application/dns-message
          *index = 44;
          return index_type::header_value;
        case 9423039217905102597U: // application/javascript
          *index = 45;
          return index_type::header_value;
        case 116355269824119741U: // application/json
          *index = 46;
          return index_type::header_value;
        case 15711274223642436716U: // application/x-www-form-urlencoded
          *index = 47;
          return index_type::header_value;
        case 12727226202374957143U: // image/gif
          *index = 48;
          return index_type::header_value;
        case 12200045138720289141U: // image/jpeg
          *index = 49;
          return index_type::header_value;
        case 12092489581388341001U: // image/png
          *index = 50;
          return index_type::header_value;
        case 6791077934204364231U: // text/css
          *index = 51;
          return index_type::header_value;
        case 8886412932112875563U: // text/html; charset=utf-8
          *index = 52;
          return index_type::header_value;
        case 16588200704567877914U: // text/plain
          *index = 53;
          return index_type::header_value;
        case 9119350762400467667U: // text/plain;charset=utf-8
          *index = 54;
          return index_type::header_value;
      }
      *index = 44;
      return index_type::header_only;
    case 905780767293800022U: // cookie
      *index = 5;
      return index_type::header_only;
    case 9202271988582577219U: // date
      *index = 6;
      return index_type::header_only;
    case 12326157660028381149U: // early-data
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 13237225503670494420U: // 1
          *index = 86;
          return index_type::header_value;
      }
      *index = 86;
      return index_type::header_only;
    case 6439525432329186999U: // etag
      *index = 7;
      return index_type::header_only;
    case 9568944059520352163U: // expect-ct
      *index = 87;
      return index_type::header_only;
    case 5644212460638603090U: // forwarded
      *index = 88;
      return index_type::header_only;
    case 12678664253902784264U: // if-modified-since
      *index = 8;
      return index_type::header_only;
    case 8836338662272140136U: // if-none-match
      *index = 9;
      return index_type::header_only;
    case 10086758965902305572U: // if-range
      *index = 89;
      return index_type::header_only;
    case 5803729767262815818U: // last-modified
      *index = 10;
      return index_type::header_only;
    case 17841021884467483934U: // link
      *index = 11;
      return index_type::header_only;
    case 14592406158319853782U: // location
      *index = 12;
      return index_type::header_only;
    case 7034821444952201446U: // origin
      *index = 90;
      return index_type::header_only;
    case 4274379369960325767U: // purpose
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 15048569516378897441U: // prefetch
          *index = 91;
          return index_type::header_value;
      }
      *index = 91;
      return index_type::header_only;
    case 12715602975004390753U: // range
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 6181996873389341388U: // bytes=0-
          *index = 55;
          return index_type::header_value;
      }
      *index = 55;
      return index_type::header_only;
    case 14154313121195327909U: // referer
      *index = 13;
      return index_type::header_only;
    case 16290507850670895904U: // server
      *index = 92;
      return index_type::header_only;
    case 666465872145128107U: // set-cookie
      *index = 14;
      return index_type::header_only;
    case 4179674550753269761U: // strict-transport-security
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 13034728143243030830U: // max-age=31536000
          *index = 56;
          return index_type::header_value;
        case 11632417432576482556U: // max-age=31536000; includesubdomains
          *index = 57;
          return index_type::header_value;
        case 16736049223785386895U: // max-age=31536000; includesubdomains;
                                    // preload
          *index = 58;
          return index_type::header_value;
      }
      *index = 56;
      return index_type::header_only;
    case 5374862646944647126U: // timing-allow-origin
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 765293966243412708U: // *
          *index = 93;
          return index_type::header_value;
      }
      *index = 93;
      return index_type::header_only;
    case 18069291607417813553U: // upgrade-insecure-requests
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 13237225503670494420U: // 1
          *index = 94;
          return index_type::header_value;
      }
      *index = 94;
      return index_type::header_only;
    case 16625730634708209664U: // user-agent
      *index = 95;
      return index_type::header_only;
    case 8816542506507529547U: // vary
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 14455144075924042224U: // accept-encoding
          *index = 59;
          return index_type::header_value;
        case 7034821444952201446U: // origin
          *index = 60;
          return index_type::header_value;
      }
      *index = 59;
      return index_type::header_only;
    case 6196956168349306122U: // x-content-type-options
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 4306527442339074562U: // nosniff
          *index = 61;
          return index_type::header_value;
      }
      *index = 61;
      return index_type::header_only;
    case 1042837635865018085U: // x-forwarded-for
      *index = 96;
      return index_type::header_only;
    case 8598773544412918900U: // x-frame-options
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 6492545016868164575U: // deny
          *index = 97;
          return index_type::header_value;
        case 13303836962186157521U: // sameorigin
          *index = 98;
          return index_type::header_value;
      }
      *index = 97;
      return index_type::header_only;
    case 15732293358711069708U: // x-xss-protection
      value_hash = XXH64(header.value().data(), header.value().size(), 0);
      switch (value_hash) {
        case 7412498726110841680U: // 1; mode=block
          *index = 62;
          return index_type::header_value;
      }
      *index = 62;
      return index_type::header_only;
  }

  return index_type::missing;
}

} // namespace static_table
} // namespace qpack
} // namespace http3
} // namespace bnl
