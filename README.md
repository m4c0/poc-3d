# poc-3d

Experimements with 3D

## Personal notes

This is a "brain-dump" because Internet search is engagement-oriented nowadays.

* Z-culling is ranging between 0 and 1 (inclusive), even when min/max is zero.
  The only way to enable different ranges is via Vulkan extensions.
* Z test are ignored when no depth buffer is present. Silently ignored on
  MoltenVK.
* Basic quad is X-right, Y-down, Z-outward (when depth-op is "less").
* We can go from right-hand down-Y to right-hand up-Y with a flipped viewport,
  depth with op-greater and clockwise front-face, or flipping yz via shader.
