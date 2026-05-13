cc = meson.get_compiler('c')

# Common test flags for both unit and concurrent tests.  -DDEBUG enables
# internal asserts; -DSKIPLIST_DIAGNOSTIC enables runtime invariant
# checks inside the implementation.
test_args = ['-DDEBUG', '-DSKIPLIST_DIAGNOSTIC']

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
test('unit', test_unit, timeout : 120)

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
