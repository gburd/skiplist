# Benchmark binary.  Built with the project's release-ish defaults
# (debugoptimized + b_ndebug=if-release).  For more aggressive numbers,
# configure with `-Dbuildtype=release`.
executable('bench',
  'bench.c',
  dependencies : [skiplist_dep, dependency('threads')],
  link_args : ['-lm'],
  c_args : ['-DNDEBUG'],
)
