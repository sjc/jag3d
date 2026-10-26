# Misc. Tools

## Tables

Create lookup tables of sine/cosine, tan and movement vectors.

```
python3 tables.py <command> <params>
```

### Commands

#### `vectors`

```
python3 tables.py vectors <steps> <length>
```

Create a table of vectors, for use as eg. movement vectors. `steps` should be `ROTSTEPS` (from `jag3d.h`). Length can be any integer. See `vector.s` for the output of vectors with length 100.

#### `sincos`

```
python3 tables.py sincos <steps>
```

Create a table of alternating sine and cosine values for an entire rotation, divided into `steps` steps. `steps` should be `ROTSTEPS` (from `jag3d.h`). Values will be 2.14 signed fixed-point numbers. These are chiefly used by the `gpumatrix` function to build a matrix from a `Transform`. If you update `ROTSTEPS` you will need to generate a new table for the values and include it in `jag3d/lib/sincos.s`

#### `tan`

```
python3 tables.py tan <steps>
```

Create a table of tan values describing an entire rotation divided into `steps` steps, as 3.13 signed fixed-point numbers.
