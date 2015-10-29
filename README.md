# MeshInspector
Utility for inspecting the properties of 2D finite element meshes

# Building
In addition to a C++ compiler, MeshInspector requires the following libraries:
* Qt5
* PROJ.4

MeshInspector can be built using the standard Qt application build process.
```bash
cd MeshInspector/
qmake MeshInspector.pro
make
```

# File formats
The following file formats are understood by MeshInspector

## Neighbour file (.nei)
This format stores the mesh as a list of nodes and node-connections.
### File type
text
### Header layout
```
Nodes
MaxNei
XMax YMax XMin YMin
```
### Header data
 Variable | Units   | Description
 -------- | ------- | -----------
 `Nodes`  | *NA*    | Number of nodes in the mesh
 `MaxNei` | *NA*    | Maximum number of neighbour (connecting) nodes surrounding each node
 `XMax`   | Degrees | Eastern-most node position (Longitude)
 `YMax`   | Degrees | Northern-most node position (Latitude)
 `XMin`   | Degrees | Western-most node position (Longitude)
 `YMin`   | Degrees | Southern-most node position (Latitude)

### Node list
After the 3 header lines, the remainder of the neighbour file is a list of the position and connections describing each node. Each line in the file describes a unique node, and the file must contain at least `Nodes + 3` lines, and `MaxNei + 5` columns.

 Column | Name      | Units   | Description
 ------ | --------- | ------- | -----------
 1      | Number    | *NA*    | Node number (starting from 1)
 2      | Longitude | degrees | X position of the node
 3      | Latitude  | degrees | Y position of the node
 4      | Type      | *NA*    | Node type (see table bellow)
 5      | Depth     | meters  | Mean water depth at the node (positive downward)
 6      | Neighbour | *NA*    | First neighbour (connecting) node
 ...    | Neighbour | *NA*    | Additional neighbours
### Node types
 Type     | Value | Description
 -------- | ----- | -----------
 Interior | 0     | Node inside the mesh
 Exterior | 1     | Node on the external mesh boundary
 Island   | 2     | Node on an internal (island) boundary

## Split element files (.nod .ele .bat)
This format stores the mesh by storing the different parts of the mesh in separate text files. A file containing the positions of each node (.nod), one listing the triangular elements (.ele), and a file containing the bathymetry data (.bat).
### File type
text
### File contents
#### .nod
 Column | Name      | Units
 ------ | --------- | -----
 1      | Node #    | *NA*
 2      | Longitude | degrees
 3      | Latitude  | degrees
#### .ele
 Column | Name     | Units
 ------ | -------- | -----
 1      | Triangle | *NA*
 2      | Node 1   | *NA*
 3      | Node 2   | *NA*
 4      | Node 3   | *NA*
#### .bat
 Column | Name   | Units
 ------ | ------ | -----
 1      | Node # | *NA*
 2      | Depth  | meters

