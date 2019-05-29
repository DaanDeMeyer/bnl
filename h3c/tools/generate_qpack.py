#!usr/bin/env python3

"""
Generates `static_table_find_index`,
`static_table_find_header_value` and `static_table_find_header_only`
functions and writes them to encode_generated.c and
decode_generated.c.

Dependencies:
- pip install --user xxhash
"""

import xxhash
import itertools
import os

static_table_raw = """\
0 	:authority
1 	:path 	/
2 	age 	0
3 	content-disposition
4 	content-length 	0
5 	cookie
6 	date
7 	etag
8 	if-modified-since
9 	if-none-match
10 	last-modified
12 	location
11 	link
13 	referer
14 	set-cookie
15 	:method 	CONNECT
16 	:method 	DELETE
17 	:method 	GET
18 	:method 	HEAD
19 	:method 	OPTIONS
20 	:method 	POST
21 	:method 	PUT
22 	:scheme 	http
23 	:scheme 	https
24 	:status 	103
25 	:status 	200
26 	:status 	304
27 	:status 	404
28 	:status 	503
29 	accept 	*/*
30 	accept 	application/dns-message
31 	accept-encoding 	gzip, deflate, br
32 	accept-ranges 	bytes
33 	access-control-allow-headers 	cache-control
34 	access-control-allow-headers 	content-type
35 	access-control-allow-origin 	*
36 	cache-control 	max-age=0
37 	cache-control 	max-age=2592000
38 	cache-control 	max-age=604800
39 	cache-control 	no-cache
40 	cache-control 	no-store
41 	cache-control 	public, max-age=31536000
42 	content-encoding 	br
43 	content-encoding 	gzip
44 	content-type 	application/dns-message
45 	content-type 	application/javascript
46 	content-type 	application/json
47 	content-type 	application/x-www-form-urlencoded
48 	content-type 	image/gif
49 	content-type 	image/jpeg
50 	content-type 	image/png
51 	content-type 	text/css
52 	content-type 	text/html; charset=utf-8
53 	content-type 	text/plain
54 	content-type 	text/plain;charset=utf-8
55 	range 	bytes=0-
56 	strict-transport-security 	max-age=31536000
57 	strict-transport-security 	max-age=31536000; includesubdomains
58 	strict-transport-security 	max-age=31536000; includesubdomains; preload
59 	vary 	accept-encoding
60 	vary 	origin
61 	x-content-type-options 	nosniff
62 	x-xss-protection 	1; mode=block
63 	:status 	100
64 	:status 	204
65 	:status 	206
66 	:status 	302
67 	:status 	400
68 	:status 	403
69 	:status 	421
70 	:status 	425
71 	:status 	500
72 	accept-language
73 	access-control-allow-credentials 	FALSE
74 	access-control-allow-credentials 	TRUE
75 	access-control-allow-headers 	*
76 	access-control-allow-methods 	get
77 	access-control-allow-methods 	get, post, options
78 	access-control-allow-methods 	options
79 	access-control-expose-headers 	content-length
80 	access-control-request-headers 	content-type
81 	access-control-request-method 	get
82 	access-control-request-method 	post
83 	alt-svc 	clear
84 	authorization
85 	content-security-policy 	script-src 'none'; object-src 'none'; base-uri 'none'
86 	early-data 	1
87 	expect-ct
88 	forwarded
89 	if-range
90 	origin
91 	purpose 	prefetch
92 	server
93 	timing-allow-origin 	*
94 	upgrade-insecure-requests 	1
95 	user-agent
96 	x-forwarded-for
97 	x-frame-options 	deny
98 	x-frame-options 	sameorigin
"""

find_index_template = """\
static static_table::index_type find_index(const header &header, uint8_t *index)
{{
  uint64_t name_hash = XXH64(header.name.data, header.name.size, 0);
  uint64_t value_hash;

  switch (name_hash) {{
    {}
  }}

  return index_type::missing;
}}\
"""

no_values_template = """\
case {}U: // {}
  *index = {};
  return index_type::header_only;\
"""

values_template = """\
case {}U: // {}
  value_hash = XXH64(header.value.data, header.value.size, 0);
  switch(value_hash) {{
    {}
  }}
  *index = {};
  return index_type::header_only;\
"""

value_template = """\
case {}U: // {}
  *index = {};
  return index_type::header_value;\
"""

static_table = [line.split() for line in static_table_raw.splitlines()]
static_table = filter(lambda entry: len(entry) >= 2, static_table)
static_table = sorted(static_table, key=lambda x: x[1])

cases = ""

for header, entry in itertools.groupby(static_table, lambda x: x[1]):
    entry = list(entry)

    indices = [nested[0] for nested in entry]
    values = [str.join(" ", nested[2:]) for nested in entry if len(nested) > 2]

    if len(values) == 0:
        index = entry[0][0]
        cases += no_values_template.format(xxhash.xxh64_intdigest(header), header, index)
    else:
        value_cases = ""

        for index, value in zip(indices, values):
            value_cases += value_template.format(xxhash.xxh64_intdigest(value), value, index)

        cases += values_template.format(xxhash.xxh64_intdigest(header), header, value_cases, indices[0])

find_index = find_index_template.format(cases)

encode_generated_template = """\
#include <h3c/http.hpp>

#include <xxhash.h>

#include <cstdint>

namespace h3c {{
namespace qpack {{
namespace static_table {{

enum class index_type {{
  missing,
  header_only,
  header_value
}};

{}

}}
}}
}}
"""

encode_generated = encode_generated_template.format(find_index)

file = open("encode_generated.cpp", "w+")
file.write(encode_generated)
file.close()

find_header_value_template = """\
bool find_header_value(uint8_t index, header *header)
{{
  switch(index) {{
    {}
  }}

  return false;
}}\
"""

header_value_case_template = """\
case {0}:
  *header = {{
    {{
      "{1}",
      sizeof("{1}") - 1
    }},
    {{
      "{2}",
      sizeof("{2}") - 1
    }}
  }};
  return true;\
"""

cases = ""

for entry in static_table:
    if len(entry) > 2:
        values = str.join(" ", entry[2:])
        cases += header_value_case_template.format(entry[0], entry[1], values)

find_header_value = find_header_value_template.format(cases)

find_header_only_template = """\
bool find_header_only(uint8_t index, header *header)
{{
  switch(index) {{
    {}
  }}

  return false;
}}\
"""

header_only_case_template = """\
case {0}:
  *header = {{
    {{
      "{1}",
      sizeof("{1}") - 1
    }},
    {{
      nullptr,
      0
    }}
  }};
  return true;\
"""

cases = ""

for entry in static_table:
    cases += header_only_case_template.format(entry[0], entry[1])

find_header_only = find_header_only_template.format(cases)

decode_generated_template = """\
#include <h3c/http.hpp>

#include <cstdint>

namespace h3c {{
namespace qpack {{
namespace static_table {{

{}

{}

}}
}}
}}
"""

decode_generated = decode_generated_template.format(find_header_value, find_header_only)

file = open("decode_generated.cpp", "w+")
file.write(decode_generated)
file.close()