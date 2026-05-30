cc = meson.get_compiler('c')

# Common test flags for both unit and concurrent tests.  -DDEBUG enables
# internal asserts; -DSKIPLIST_DIAGNOSTIC enables runtime invariant
# checks inside the implementation.
test_args = ['-DDEBUG', '-DSKIPLIST_DIAGNOSTIC']

# Splay-rebalance variant args: same baseline plus the opt-in flag that
# enables the adaptive-height rebalance pass.  This exercises the
# signature feature of the library.
splay_args = test_args + ['-DSKIPLIST_SPLAY_REBALANCE']

# munit is vendored under tests/.
munit_lib = static_library('munit',
  'munit.c',
  include_directories : include_directories('.'),
  c_args : test_args,
  install : false,
)

# Single-threaded unit tests.
test_unit = executable('test_skiplist',
  'test.c',
  include_directories : [include_directories('.')],
  link_with : munit_lib,
  dependencies : [skiplist_dep, dependency('threads')],
  c_args : test_args,
  link_args : ['-lm'],
)
test('unit', test_unit, timeout : 1200)

# Multi-threaded tests.  Always built; sanitizer choice comes from the
# global -Db_sanitize option (set 'thread' to exercise TSAN).
test_concurrent = executable('test_concurrent',
  'test_concurrent.c',
  include_directories : [include_directories('.')],
  link_with : munit_lib,
  dependencies : [skiplist_dep, dependency('threads')],
  c_args : test_args,
  link_args : ['-lm'],
)
test('concurrent', test_concurrent, timeout : 120)

# Splay variants: identical sources, compiled with SKIPLIST_SPLAY_REBALANCE
# defined so the adaptive-height rebalance pass is exercised in CI.
test_unit_splay = executable('test_skiplist_splay',
  'test.c',
  include_directories : [include_directories('.')],
  link_with : munit_lib,
  dependencies : [skiplist_dep, dependency('threads')],
  c_args : splay_args,
  link_args : ['-lm'],
)
test('unit-splay', test_unit_splay, timeout : 1200)

test_concurrent_splay = executable('test_concurrent_splay',
  'test_concurrent.c',
  include_directories : [include_directories('.')],
  link_with : munit_lib,
  dependencies : [skiplist_dep, dependency('threads')],
  c_args : splay_args,
  link_args : ['-lm'],
)
test('concurrent-splay', test_concurrent_splay, timeout : 120)

# Aksenov 2020 height-target verification.  Confirms hot keys settle
# at the paper's specified depth log2(1/p) below the head, +/-1 level
# tolerance for rebalance-interval lag.  Skipped when the splay flag
# is not defined.
test_splay_verify = executable('test_splay_verify',
  'test_splay_verify.c',
  include_directories : [include_directories('.')],
  link_with : munit_lib,
  dependencies : [skiplist_dep, dependency('threads')],
  c_args : splay_args,
  link_args : ['-lm'],
)
test('splay-verify', test_splay_verify, timeout : 120)

# Single-threaded mode: SKIPLIST_SINGLE_THREADED replaces all C11
# atomics with plain operations and disables EBR.  Builds its own
# munit instance because the flag has global header effect.
test_single = executable('test_single',
  'test_single.c',
  'munit.c',
  include_directories : [include_directories('.')],
  dependencies : [skiplist_dep],
  c_args : test_args,
  link_args : ['-lm'],
)
test('single', test_single, timeout : 120)
