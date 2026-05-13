project('skiplist', 'c',
  version : '1.0.0',
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
