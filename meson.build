project('skiplist', 'c',
  version : '1.1.2',
  license : 'ISC OR MIT',
  default_options : [
    'c_std=c11',
    'warning_level=3',
    'werror=true',
    'buildtype=debugoptimized',
    'b_ndebug=if-release',
  ],
  meson_version : '>= 0.60.0',
)

# GCC 14+ emits -Wtsan for the seq_cst atomic_thread_fence used by the EBR
# pin/advance protocol: ThreadSanitizer cannot model a standalone fence.
# The fences are intentional and correct (validated under the Makefile's
# -Og TSAN build), so under -O2 + -fsanitize=thread + werror we silence the
# diagnostic.  Clang has no -Wtsan, so this is gated to GCC.
if meson.get_compiler('c').get_id() == 'gcc' and get_option('b_sanitize').contains('thread')
  add_project_arguments('-Wno-tsan', language : 'c')
endif

# This is a header-only library.  We declare it as a meson dependency
# so downstream projects can `dependency('skiplist')` and inherit the
# include path automatically.
inc = include_directories('include')

skiplist_dep = declare_dependency(
  include_directories : inc,
  # No link libraries: the entire implementation is in include/sl.h.
)

# Install the public header and a pkg-config file for non-meson consumers.
install_headers('include/sl.h')

# Install man pages -- skiplist.7 is the conceptual overview,
# sl.h.3 is the API reference (a single page covering all macros,
# in the style of queue(3) or tree(3)).
install_man('man/skiplist.7', 'man/sl.h.3')

pkg = import('pkgconfig')
pkg.generate(
  name : 'skiplist',
  description : 'Header-only C splay-list with lock-free concurrency',
  version : meson.project_version(),
  url : 'https://codeberg.org/gregburd/skiplist',
)

# Tests, examples, and benchmarks live under their own subdirectories.
subdir('tests')
subdir('examples')
subdir('bench')
