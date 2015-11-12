# MeshInspector
Utility for inspecting the properties of 2D finite element meshes

# About
MeshInspector is a program that calculates and displays several statistics (maximum, minimum, mean and standard deviation) for different parameters of finite element meshes. These parameters are:

* Ratio of bottom slope to the depth (delta-h/h) for each element
* Area of the elements
* Depth
* Lengths of element sides
* Delta-t or wave speed of each element
  * *(square root of g times the depth)*

# Versions
The original version of MeshInspector was produced by Paul Chapman using an early gtk version. This version was later ported by Jason Chaffey ChaffeyJ@mar.dfo-mpo.gc.ca to the Qt3 GUI framework.

The present Version of MeshInspector was created by Tristan Losier tristan.losier@unb.ca and includes a rewrite of MeshInspector's user interface using the Qt5 GUI framework.

* V1.0 ***gtk***
* V2.0 ***Qt3***
* V3.0 ***Qt4***

# Building
In addition to a C++ compiler, MeshInspector requires the following libraries:
* Qt5 *qtbase5-dev*
* PROJ.4 *libproj-dev*

MeshInspector can be built using the standard Qt application build process.

    cd MeshInspector/
    qmake MeshInspector.pro
    make

# Running
The MeshInspector executable may be launched from a GUI environment or a command line terminal. When launched from the command a mesh file may be specified as an optional command argument.

    meshinspect <filename>

# File formats
The following file formats are understood by MeshInspector

## Neighbour file (.nei)
This format stores the mesh as a list of nodes and node-connections.

### File type
text

### Header layout
    Nodes
    MaxNei
    XMax YMax XMin YMin

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

