# Examples are built but not run as part of `meson test`; running them
# is left to humans (some print to stdout, some emit GraphViz, etc.).
example_names = [
  'ex01', 'ex02', 'ex03', 'ex04', 'ex05',
  'ex06', 'ex07', 'ex08', 'ex09', 'ex10',
]

foreach name : example_names
  needs_pthread = (name == 'ex09' or name == 'ex10')
  deps = [skiplist_dep]
  if needs_pthread
    deps += dependency('threads')
  endif
  executable(name,
    name + '.c',
    dependencies : deps,
    link_args : ['-lm'],
  )
endforeach
