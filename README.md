# Skiplist

This project defines [a skiplist data structure written in
C](https://git.burd.me/greg/skiplist/src/branch/main/include/sl.h).  Implemented
as using macros this code provides a way to essentially "template" (as in C++)
and emit code with types and functions specific to your use case.  You can apply
these macros multiple times safely in your code, once for each application.

While there are lock-free implementations of a skiplist, this implementation is
not (yet) lock-free or designed to manage concurrent access in any way.  Use a
mutex or some other method to serialize access to the API ([until I finish the
lock-free
variant](https://git.burd.me/greg/skiplist/src/branch/gburd/lock-free)).

Study the [example
code](https://git.burd.me/greg/skiplist/src/branch/main/examples/ex1.c) to see
how this works in practice.

## Overview
A skiplist is a sorted list with O(log(n)) on average for most operations.  It
is a probabilistic datastructure, meaning that it does not guarantee O(log(n))
it approximates it over time.  This implementation includes improves the
probability by integrating the splay list algorithm for re-balancing trading off
a bit of computational overhead and code complexity for a nearly always optimal,
or "perfect" skiplist.

Conceptually, the arrangement of a skiplist appears as follows:

```
<head> ----------> [2] --------------------------------------------------> [9] ---------->
<head> ----------> [2] ------------------------------------[7] ----------> [9] ---------->
<head> ----------> [2] ----------> [4] ------------------> [7] ----------> [9] --> [10] ->
<head> --> [1] --> [2] --> [3] --> [4] --> [5] --> [6] --> [7] --> [8] --> [9] --> [10] ->
```

Each node contains at the very least a link to the next element in the list
(corresponding to the lowest level in the above diagram), but it can randomly
contain more links which skip further down the list (the towers in the above
diagram). This allows for the algorithm to move down the list faster than
having to visit every element.

Conceptually, the skiplist can be thought of as a stack of linked lists. At
the very bottom is the full linked list with every element, and each layer
above corresponds to a linked list containing a random subset of the elements
from the layer immediately below it. The probability distribution that
determines this random subset can be customized, but typically a layer will
contain half the nodes from the layer below.

This implementation maintains a doubly-linked list at the bottom layer to
support efficient iteration in either direction.  There is also a guard
node at the tail rather than simply pointing to NULL.

```
<head> <-> [1] <-> [2] <-> [3] <-> [4] <-> [5] <-> [6] <-> [7] <-> <tail>
```

## Safety:

The ordered skiplist relies on a well-behaved comparison
function. Specifically, given some ordering function f(a, b), it must satisfy
the following properties:

1) Be well-defined: f(a, b) should always return the same value
2) Be antisymmetric: f(a, b) == Greater if and only if f(b, a) == Less, and
   f(a, b) == Equal == f(b, a).
3) Be transitive: If f(a, b) == Greater and f(b, c) == Greater than f(a, c)
   == Greater.

Failure to satisfy these properties can result in unexpected behavior at
best, and at worst will cause a segfault, null deref, or some other bad
behavior.

## References:
Sources of information most helpful for this implementation include, but are
not limited to:

 - Skip lists: a probabilistic alternative to balanced trees
   ```@article{10.1145/78973.78977,
     author = {Pugh, William},
     title = {Skip lists: a probabilistic alternative to balanced trees},
     year = {1990}, issue_date = {June 1990},
     publisher = {Association for Computing Machinery},
     address = {New York, NY, USA},
     volume = {33}, number = {6}, issn = {0001-0782},
     url = {https://doi.org/10.1145/78973.78977},
     doi = {10.1145/78973.78977},
     journal = {Commun. ACM}, month = {jun}, pages = {668-676}, numpages = {9},
     keywords = {trees, searching, data structures},
     download = {https://www.cl.cam.ac.uk/teaching/2005/Algorithms/skiplists.pdf}
   }```

 - Tutorial: The Ubiquitous Skiplist, its Variants, and Applications in Modern Big Data Systems
   ```@article{Vadrevu2023TutorialTU,
     title={Tutorial: The Ubiquitous Skiplist, its Variants, and Applications in Modern Big Data Systems},
     author={Venkata Sai Pavan Kumar Vadrevu and Lu Xing and Walid G. Aref},
     journal={ArXiv},
     year={2023},
     volume={abs/2304.09983},
     url={https://api.semanticscholar.org/CorpusID:258236678},
     download={https://arxiv.org/pdf/2304.09983.pdf}
   }```

 - The Splay-List: A Distribution-Adaptive Concurrent Skip-List
   ```@misc{aksenov2020splaylist,
     title={The Splay-List: A Distribution-Adaptive Concurrent Skip-List},
     author={Vitaly Aksenov and Dan Alistarh and Alexandra Drozdova and Amirkeivan Mohtashami},
     year={2020},
     eprint={2008.01009},
     archivePrefix={arXiv},
     primaryClass={cs.DC},
     download={https://arxiv.org/pdf/2008.01009.pdf}
   }```

 - JellyFish: A Fast Skip List with MVCC},
   ```@article{Yeon2020JellyFishAF,
     title={JellyFish: A Fast Skip List with MVCC},
     author={Jeseong Yeon and Leeju Kim and Youil Han and Hyeon Gyu Lee and Eunji Lee and Bryan Suk Joon Kim},
     journal={Proceedings of the 21st International Middleware Conference},
     year={2020},
     url={https://api.semanticscholar.org/CorpusID:228086012}
   }```

## Open Source
I'd like to thank others for thoughtfully licensing their work, the
community of software engineers succeeds when we work together.

Portions of this code are derived from other copyrighted works:

 - _MIT License_
   - https://github.com/greensky00/skiplist
     - 2017-2024 Jung-Sang Ahn <jungsang.ahn@gmail.com>
   - https://github.com/paulross/skiplist
     - Copyright (c) 2017-2023 Paul Ross <paulross@uky.edu>
   - https://github.com/JP-Ellis/rust-skiplist
     - Copyright (c) 2015 Joshua Ellis <github@jpellis.me>
 - _Public Domain_
   - https://gist.github.com/zhpengg/2873424
     - Zhipeng Li <zhpeng.is@gmail.com>

### TODO:
 * The concurrent, lock-free version of this (see [gburd/lock-free](https://git.burd.me/greg/skiplist/src/branch/gburd/lock-free) branch for WIP).
