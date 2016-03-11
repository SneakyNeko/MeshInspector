#ifndef NUTIL_H
#define NUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOD_INTERIOR 0
#define NOD_EXTERIOR 1
#define NOD_ISLAND   2
#define NOD_UNKNOWN  11111

#define EXPORT_AREA 0
#define EXPORT_D_T  1
#define EXPORT_D_H  2

#define PAGES 5

typedef struct _node {
    unsigned int   number;
    unsigned short type;
    double         x;     // Cartesian X
    double         y;     // Cartesian Y
    double         lon;   // Longitude
    double         lat;   // Lattitude
    double         depth; // Bathymetric depth
    short          neighbourCount;
    unsigned int * neighbours;
} node;

typedef struct _nodelist {
    unsigned int numNodes;
    unsigned int numElems;
    node **      nodes;
    char *       proj;
} nodelist;

typedef struct _triangle {
    unsigned int n1;
    unsigned int n2;
    unsigned int n3;
} triangle;

typedef struct _seg {
    unsigned int n1;
    unsigned int n2;
    int          index;
} seg;

extern int maxNei;

void panic(const char * reason, int num);
void guess_proj(nodelist * nlist);
void project(nodelist * nlist);
nodelist * newList(void);
int addNode(node * n, nodelist * list);
nodelist * read_nei(const char * file);
nodelist * read_dat(const char * file);
void write_dat(nodelist * nlist, const char * file);
void write_nod(nodelist * list, FILE * f);
int find_node(nodelist * list, unsigned int num);
void write_ele(nodelist * list, FILE * f);
void write_bat(nodelist * list, FILE * f);
void write_neb(nodelist * list, const char * filestub);
void add_connection(nodelist * nlist, unsigned int n1, unsigned int n2);
void reorder_nodes(nodelist * nlist);
void fix_boundaries(nodelist * nlist);
void fix_connections(nodelist * nlist, triangle * tlist, int n);
nodelist * read_neb(const char * filestub);
void write_nei(nodelist * nlist, const char * file);
void freeList(nodelist * plist);

double get_area(nodelist * nlist, int n1, int n2, int n3);
double arc_length(double theta1, double phi1, double theta2, double phi2);
double get_area_polar(nodelist * nlist, int n1, int n2, int n3);
void generate_histogram(int which, double max, double min, double value, int ** histData);
void side_stats(nodelist * nlist, int ** histData, double ** stats);
void side_stats_polar(nodelist * nlist, int ** histData, double ** stats);
void boundary_stats(nodelist * list, int ** histData, double ** stats, int polarStats);
void print_general(nodelist * list, int polar, int minMax, int which, const char * file);
void print_depth(nodelist * list, const char * file);
void print_side(nodelist * list, int polar, int minMax, const char * file);

#endif
