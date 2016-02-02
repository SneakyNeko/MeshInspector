#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <proj_api.h>

#include "nodeUtil.h"

static const char out_of_memory[] = "Out of memory!";
static const char open_file[] = "Couldn't open file!";
static const char read_file[] = "Invallid mesh!";

static const char projFormat[] = "+proj=lcc +lon_0=%0.4f +lat_0=%0.4f +lat_1=%0.2f +lat_2=%0.2f +ellps=WGS84";

int maxNei;

void panic(const char * reason, int num)
{
    fprintf(stderr, "FATAL ERROR %d: %s\n", num, reason);
    exit(num);
}

void guess_proj(nodelist * nlist)
{
    double xmin, xmax;
    double ymin, ymax;
    
    xmin = xmax = nlist->nodes[0]->x;
    ymin = ymax = nlist->nodes[0]->y;
    
    for (unsigned int i = 1; i < nlist->numNodes; i++) {
        node * cur = nlist->nodes[i];
        if (cur->x < xmin) xmin = cur->x;
        else if (cur->x > xmax) xmax = cur->x;
        if (cur->y < ymin) ymin = cur->y;
        else if (cur->y > ymax) ymax = cur->y;
    }
    
    if (xmax - xmin <= 360.0 && ymax - ymin <= 360.0) { // Is the input data in spherical units? (guess)
        // Build a projection refrence string, guessing relevent parameters to fit the input data
        char projTmp[80];
        sprintf(projTmp, projFormat, (xmax - xmin)/2 + xmin, (ymax - ymin)/2 + ymin, ymin, ymax);
        char * projRef = malloc(strlen(projTmp) + 1);
        if (projRef == NULL) panic(out_of_memory, 100);
        nlist->proj = strcpy(projRef, projTmp);
        
        project(nlist);
    }
}

void project(nodelist * nlist)
{
    // Initialize the projection
    projPJ pj_lcc = pj_init_plus(nlist->proj);
    if (pj_lcc == NULL) {
        panic("Failed to create coordinate projection", 101);
        printf("Error: %s\n", pj_strerrno(*pj_get_errno_ref()));
    }
    projPJ pj_latlong = pj_latlong_from_proj(pj_lcc);
    if (pj_latlong == NULL) {
        panic("Failed to create coordinate projection", 101);
        printf("Error: %s\n", pj_strerrno(*pj_get_errno_ref()));
    }

    // Perform the projection
    for (unsigned int i = 0; i < nlist->numNodes; i++) {
        node * cur = nlist->nodes[i];
        cur->lon   = cur->x;
        cur->lat   = cur->y;
        cur->x    *= DEG_TO_RAD;
        cur->y    *= DEG_TO_RAD;
        pj_transform(pj_latlong, pj_lcc, 1, 1, &cur->x, &cur->y, NULL);
    }
    
    // Clean up
    pj_free(pj_latlong);
    pj_free(pj_lcc);
}

void freeList(nodelist * nlist)
{
    if (nlist == NULL) return;
    for (unsigned int i = 0; i < nlist->numNodes; i++) {
        node * cur = nlist->nodes[i];
        free(cur->neighbours);
        free(cur);
    }
    free(nlist->nodes);
    if (nlist->proj != NULL) free(nlist->proj);
}

nodelist * newList(void)
{
    nodelist * temp;
    temp = malloc(sizeof(nodelist));
    if (!temp) return temp;
    temp->numNodes = 0;
    temp->nodes = NULL;
    temp->proj = NULL;
    return temp;
}

int addNode(node * n, nodelist * list)
{
    list->nodes = realloc(list->nodes, sizeof(node)*(list->numNodes + 1));
    if (list->nodes == NULL) return 1;
    list->nodes[list->numNodes] = n;
    list->numNodes++;
    return 0 ;
}

int find_node(nodelist * list, unsigned int num)
{
    // Since our nodes are not necessarily ordered, but usually are,
    // we make this check first, and then do an exhaustive search.
    if (list->nodes[num-1]->number == num) return num - 1;
    
    for (unsigned int i = 0; i < list->numNodes; i++)
        if (list->nodes[i]->number == num) return i;
    return 0;
}

void add_connection(nodelist * nlist, unsigned int n1, unsigned int n2)
{
    node * c1 = nlist->nodes[find_node(nlist,n1)];
    node * c2 = nlist->nodes[find_node(nlist,n2)];
    
    // Check to see if the connection is already there
    if (c1->neighbourCount > 0 && c2->neighbourCount > 0) {
        for (short i = 0; i < c1->neighbourCount; i++)
            if (c1->neighbours[i] == n2) return;
        
        for (short i = 0; i < c2->neighbourCount; i++)
            if (c2->neighbours[i] == n1) return;
    }
    
    // Now, add the connection
    c1->neighbours = realloc(c1->neighbours, sizeof(unsigned int)*(c1->neighbourCount + 1));
    if (!c1->neighbours) panic(out_of_memory, 40);
    c1->neighbours[c1->neighbourCount] = n2;
    c1->neighbourCount++;
    
    c2->neighbours = realloc(c2->neighbours, sizeof(unsigned int)*(c2->neighbourCount + 1));
    if (!c2->neighbours) panic(out_of_memory, 40);
    c2->neighbours[c2->neighbourCount] = n1;
    c2->neighbourCount++;
}

void reorder_nodes(nodelist * nlist)
{
    // We need the nodelist in counter-clockwise order
    double *angles;
    int smallest;
    
    for (unsigned int i = 0; i < nlist->numNodes; i++) {
        node * cur = nlist->nodes[i];
        angles     = malloc(sizeof(double)*cur->neighbourCount);
        if (!angles) panic(out_of_memory, 50);
        
        smallest = 0;
        for (short j = 0; j < cur->neighbourCount; j++) {
            node * cur2 = nlist->nodes[find_node(nlist,cur->neighbours[j])];
            angles[j]   = atan2(cur2->y - cur->y, cur2->x - cur->x);
            if (angles[j] < angles[smallest]) smallest = j;
        }
        
        // selection sort -- array is not big enough to require anything fancier
        for (short j = 0; j < cur->neighbourCount; j++) {
            for (short k = j; k < cur->neighbourCount; k++) {
                if (angles[k] < angles[j]) {
                    double tempang;
                    int tempind;
                    
                    tempang   = angles[k];
                    angles[k] = angles[j];
                    angles[j] = tempang;
                    tempind   = cur->neighbours[k];
                    cur->neighbours[k] = cur->neighbours[j];
                    cur->neighbours[j] = tempind;
                }
            }
        }
        free(angles);
    }
}

void fix_boundaries(nodelist * nlist)
{
    // Need to determine which nodes are boundary nodes, and
    // whether the boundaries are island or exterior
    // Rule: Any node that has a boundary connection is a boundary node
    seg *s;
    int segSize;
    int curInd;
    int leftmost;
    
    segSize = 0;
    s       = NULL;
    for(unsigned int i = 0; i < nlist->numNodes; i++)
        nlist->nodes[i]->type = NOD_INTERIOR;
    
    for (unsigned int i = 0; i < nlist->numNodes; i++) {
        node * cur = nlist->nodes[i];
        for(short j = 0; j < cur->neighbourCount; j++) {
            int m = 0;
            node * cur2 = nlist->nodes[find_node(nlist,cur->neighbours[j])];
            for(short k = 0; k < cur->neighbourCount; k++) {
                for(short l = 0; l < cur2->neighbourCount; l++) {
                    if(cur->neighbours[k] == cur2->neighbours[l]) m++;
                }
            }
            if(m < 2) {
                cur2->type = NOD_ISLAND;
                segSize++;
                s = realloc(s, sizeof(seg)*segSize);
                if(!s) panic(out_of_memory, 61);
                s[segSize-1].n1    = cur->number;
                s[segSize-1].n2    = cur2->number;
                s[segSize-1].index = 0;
            }
            if (m < 1) panic("Miscellaneous unhelpful error message!", 60);
        }
    }
    
    for (int i = 0; i < segSize; i++) {
        if(s[i].n1 > s[i].n2) s[i].index = 5555;
    }
    
    // We should now have a list of segments in s
    // These *should* be loops.  We need to assign an index
    // number to the segments of each loop
    for (curInd = 1; ; curInd++) {
        unsigned int quest, start;
        int done, worked, worked2, i;
        
        for (i = 0; i < segSize && s[i].index != 0; i++);
        if (i == segSize) break;
        
        // 'i' now points to an unindexed segment
        start       = s[i].n1;
        quest       = s[i].n2;
        s[i].index  = curInd;
        done        = 0;
        while (!done) {
            worked = 0;
            for (int j = 0; j < segSize && !done; j++) {
                if (j == i || s[j].index != 0) continue;
                worked2 = 0;
                if (s[j].n1 == quest) {
                    s[j].index  = curInd;
                    quest       = s[j].n2;
                    worked      = 1;
                    worked2     = 1;
                }
                if (s[j].n2 == quest && !worked2) {
                    s[j].index  = curInd;
                    quest       = s[j].n1;
                    worked      = 1;
                }
                if (quest == start) { done = 1; worked = 1; }
            }
            if (worked == 0) panic("Open loops!", 62);
        }
    }
    // If we're lucky, we should now have all the loops indexed
    // We now need to find the loop with the leftmost element
    
    // First find the leftmost element, which must be a boundary node, obviously
    leftmost = 0;
    for (unsigned int i = 1; i < nlist->numNodes; i++)
        if (nlist->nodes[i]->x < nlist->nodes[leftmost]->x)
            leftmost = i;
    
    // Now find the first segment that contains this
    int i;
    for (i = 0; i < segSize; i++)
        if (s[i].n1 == nlist->nodes[leftmost]->number || s[i].n2 == nlist->nodes[leftmost]->number)
            break;
    if (i == segSize) panic("Leftmost point is not a boundary!", 63);
    
    // Now proceed to mark off all the nodes in all the segments with the same index number
    for (int j = 0; j < segSize; j++) {
        if (s[j].index == s[i].index) {
            nlist->nodes[find_node(nlist,s[j].n1)]->type = NOD_EXTERIOR;
            nlist->nodes[find_node(nlist,s[j].n2)]->type = NOD_EXTERIOR;
        }
    }
    free(s);
}

void fix_connections(nodelist * nlist, const char * file)
{
    FILE * ele;
    int result, n;
    int tn, n1, n2, n3;
    triangle * tlist = NULL;
    
    ele = fopen(file, "r");
    if (!ele) panic(open_file, 25);
    
    n = 0;
    while (1) {
        result = fscanf(ele, "%d %d %d %d", &tn, &n1, &n2, &n3);
        if (result == EOF) break;
        tlist = realloc(tlist, sizeof(triangle)*(n + 1));
        if (!tlist) panic(out_of_memory, 30);
        tlist[n].n1 = n1;
        tlist[n].n2 = n2;
        tlist[n].n3 = n3;
        n++;
    }
    fclose(ele);
    
    for (int i = 0; i < n; i++) {
        add_connection(nlist, tlist[i].n2, tlist[i].n3);
        add_connection(nlist, tlist[i].n1, tlist[i].n3);
        add_connection(nlist, tlist[i].n1, tlist[i].n2);
    }
    free(tlist);
    
    // do a sanity check
    for (unsigned int i = 0; i < nlist->numNodes; i++) {
        if ((nlist->nodes)[i]->neighbours == NULL)
            panic("Busted mesh 1!", i);
        if ((nlist->nodes)[i]->neighbourCount < 2) {
            printf("Mesh node %d has %d neighbours!\n", nlist->nodes[i]->number, nlist->nodes[i]->neighbourCount);
            panic("Busted mesh 2!", i);
        }
    }
    
    reorder_nodes(nlist);
    fix_boundaries(nlist);
}

nodelist * read_nei(const char * file)
{
    FILE * in;
    nodelist * nlist;
    node * temp;
    int total;
    double xmax, ymax, xmin, ymin;
    int nm;
    double x, y, z;
    int * nei;
    int type;
    int result;
    int numNei;
    
    in = fopen(file, "r");
    if (in == NULL) panic(open_file, 1);
    nlist = newList();
    if (!nlist) panic(out_of_memory, 2);

    // Read in header
    result = fscanf(in, "%d \n", &total);
    if (result == EOF) panic(read_file, 4);
    result = fscanf(in, "%d \n", &maxNei);
    if (result == EOF) panic(read_file, 4);
    result = fscanf(in, "%lf %lf %lf %lf \n", &xmax, &ymax, &xmin, &ymin);
    if (result == EOF) panic(read_file, 4);
    
    nei = malloc(sizeof(int)*maxNei);
    if (!nei) panic(out_of_memory, 1);
    while (1) {
        result = fscanf(in, "%d %lf %lf %d %lf", &nm, &x, &y, &type, &z);
        if (result == EOF) break;
        numNei = 0;
        for (int i = 0; i < maxNei; i++) {
            result = fscanf(in, "%d", &(nei[i]));
            if (result == EOF) break;
            if (nei[i] != 0 && nei[i] != nm) numNei++;
        }
        
        temp = malloc(sizeof(node));
        if (!temp) panic(out_of_memory, 3);
        temp->number         = nm;
        temp->type           = type;
        temp->x              = x;
        temp->y              = y;
        temp->depth          = z;
        temp->neighbourCount = numNei;
        temp->neighbours     = malloc(sizeof(unsigned int)*numNei);
        for (int i = 0, j = 0; i < maxNei; i++)
            if (nei[i] != 0 && nei[i] != nm) temp->neighbours[j++] = nei[i];
        if (addNode(temp, nlist)) panic(out_of_memory, 4);
    }
    free(nei);
    fclose(in);
    
    guess_proj(nlist);
    
    return(nlist);
}

void write_nei(nodelist * nlist, const char * file)
{
    FILE * f;
    short max;
    double xmin, xmax, ymin, ymax;
    node * cur;
    
    f = fopen(file, "w");
    if (nlist->numNodes == 0) return;
    
    fprintf(f, "%12d\n", nlist->numNodes);
    
    // Find max # of neighbours
    max = 0;
    for (unsigned int i = 0; i < nlist->numNodes; i++)
        if (nlist->nodes[i]->neighbourCount > max)
            max = nlist->nodes[i]->neighbourCount;
    fprintf(f, "%12d\n", max);
    
    // Find coord bounds
    cur  = nlist->nodes[0];
    xmin = xmax = cur->lon;
    ymin = ymax = cur->lat;
    for(unsigned int i = 0; i < nlist->numNodes; i++) {
        cur = (nlist->nodes)[i];
        if(cur->lon > xmax) xmax = cur->lon;
        if(cur->lon < xmin) xmin = cur->lon;
        if(cur->lat > ymax) ymax = cur->lat;
        if(cur->lat < ymin) ymin = cur->lat;
    }
    fprintf(f, "%12.6f %12.6f %12.6f %12.6f\n", xmax, ymax, xmin, ymin);
    
    for (unsigned int i = 0; i < nlist->numNodes; i++) {
        cur = nlist->nodes[i];
        fprintf(f, "%12d %12.6f %12.6f %3d %12.6f", cur->number, cur->lon, cur->lat, cur->type, cur->depth);
        for (short j = 0; j < max; j++)
            if (j < cur->neighbourCount)
                fprintf(f, "%8d", cur->neighbours[j]);
            else
                fprintf(f, "%8d", 0);
        fprintf(f, "\n");
    }
    fclose(f);
}

nodelist * read_neb(const char * filestub)
{
    FILE * nod, * bat;
    nodelist * nlist;
    int n1, n2;
    double x, y, z;
    char filename[256];
    
    nlist = newList();
    if (!nlist) panic(out_of_memory, 21);
    
    strcpy(filename, filestub);
    strcat(filename, ".nod");
    nod = fopen(filename, "r");
    strcpy(filename, filestub);
    strcat(filename, ".bat");
    bat = fopen(filename, "r");
    if (!(nod && bat)) panic(open_file, 20);
    
    // First, create the nodes from the .nod and .bat files,
    // the connections will be delt with after
    while(1) {
        int result = fscanf(nod, "%d %lf %lf", &n1, &x, &y);
        if (result == EOF) break;
        result = fscanf(bat, "%d %lf", &n2, &z);
        if (result == EOF) break;
        if (n1 != n2) panic("Inconsistent nod and bat files!", 21);
        
        node * temp = malloc(sizeof(node));
        if (!temp) panic(out_of_memory, 22);
        
        temp->number         = n1;
        temp->type           = NOD_UNKNOWN;
        temp->x              = x;
        temp->y              = y;
        temp->depth          = z;
        temp->neighbourCount = 0;
        temp->neighbours     = NULL;
        if (addNode(temp, nlist)) panic(out_of_memory, 23);		
    }
    
    strcpy(filename, filestub);
    strcat(filename, ".ele");
    fix_connections(nlist, filename);
    guess_proj(nlist);
    
    return(nlist);
}

void write_neb(nodelist * list, const char * filestub)
{
    char filename[256];
    FILE * f;
    
    strcpy(filename, filestub);
    strcat(filename, ".nod");
    f = fopen(filename, "w");
    if (!f) panic(open_file, 10);
    write_nod(list, f);
    fclose(f);
    
    strcpy(filename, filestub);
    strcat(filename, ".ele");
    f = fopen(filename, "w");
    if (!f) panic(open_file, 11);
    write_ele(list, f);
    fclose(f);
    
    strcpy(filename, filestub);
    strcat(filename, ".bat");
    f = fopen(filename, "w");
    if (!f) panic(open_file, 12);
    write_bat(list, f);
    fclose(f);
}

void write_ele(nodelist * list, FILE * f)
{
    int counter = 1;
    for (unsigned int i = 0; i < list->numNodes; i++) {
        node * cur = list->nodes[i];
        short idx = cur->neighbourCount;
        for (short j = 0; j < idx; j++) {
            if (cur->neighbours[j] <= cur->number) continue;
            node * cur2 = list->nodes[find_node(list, cur->neighbours[j])];
            short idx2 = cur2->neighbourCount;
            for (short k = 0; k < idx2; k++) {
                for (short l = 0; l < idx; l++) {
                    unsigned int a, b;
                    a = cur2->neighbours[k];
                    b = cur->neighbours[l];
                    if (a == b && a != cur->number && b != cur2->number && a > cur->number && a > cur2->number) {
                        int first, second, temp;
                        if ( j<l ) {
                            first = j;
                            second = l;
                        } else {
                            first = l;
                            second = j;
                        }
                        if ((j == 0 && l == cur->neighbourCount - 1) || (l == 0 && j == cur->neighbourCount - 1)) {
                            temp = first;
                            first = second;
                            second = temp;
                        }
                        fprintf(f, "%7d %7d %7d %7d\n", counter++, cur->number, cur->neighbours[first], cur->neighbours[second]);
                    }
                }
            }
        }
    }
}

void write_bat(nodelist * list, FILE * f)
{
    for(unsigned int i = 0; i < list->numNodes; i++) {
        node * cur = list->nodes[i];
        fprintf(f, "%d %f\n", cur->number, cur->depth);
    }
}

void write_nod(nodelist * list, FILE *f)
{
    for (unsigned int i = 0; i < list->numNodes; i++) {
        node * cur = list->nodes[i];
        fprintf(f, "%d %12.6f %12.6f\n", cur->number, cur->lon, cur->lat);
    }
}

