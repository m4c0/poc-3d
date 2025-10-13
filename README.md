# poc-3d

Experimements with 3D

## Personal notes

This is a "brain-dump" because Internet search is engagement-oriented nowadays.

* Z-culling is ranging between 0 and 1 (inclusive), even when min/max is zero.
* Z test is set to "less" but it is ignored when no depth buffer is present.
* Basic quad (using what I got at the moment) is X-right, Y-down, Z-unknown
  (Z makes no sense without a depth buffer as it is only used for culling)
