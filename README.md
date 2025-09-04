# Hybrid-DIBR
Hybrid DIBR is a Depth-Image based renderer that combines traditional mesh based rendering with novel image based rendering. It is hybrid because it features two stages: a precomputation fase that generates a dynamic mesh and a separate rendering fase that renders in real time.

## Dataset
This program uses a dataset as described in [the data folder](data/).

```json
{
"Resolution" : [<width>, <height>],
"Focal" : [<x>, <y>],
"frames" : <number>,
"cameras" : [
    {
    "Principal_point" : [<x>, <y>],
    "Depth_range" : [<near>, <far>],
    "NameColor" : "<name>.mp4",
    "NameDepth" : "<name>.mp4",
    "model" : [ <4x4 matrix> ]
    },
    ...
}
```

## Building & Running

### Build project
```
cmake . -B bin
cmake --build bin --config Debug
```

### Run Hybrid-DIBR
```sh
bin\Debug\HybridDIBR.exe data/EXAMPLE/example.json
```

### Running options
```sh
bin\Debug\HybridDIBR.exe -h
```

## Controls
Move around: **Z**, **Q**, **S**, and **D**

Move up & down: **SPACE** and **SHIFT**

Look around: dragging the mouse

Fine control: hold **CTRL**

Full textured mesh (default): **F**

Point cloud: **P**

Connected point cloud: **L**

Render to image: **E**

