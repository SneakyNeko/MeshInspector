#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <signal.h>

#include "nodeUtil.h"

#define PIOVER180 0.017453293
#define RADIUS    6367500.0

double unitScale   = 1.0;
double cutoffValue = 10.0;

double get_area(nodelist * nlist, int n1, int n2, int n3)
{
    double s, area;
    double l1, l2, l3;
    double x1, y1, x2, y2, x3, y3;
    double d1, d2;
    double temp;
    
    x1 = nlist->nodes[find_node(nlist, n1)]->x*unitScale;
    y1 = nlist->nodes[find_node(nlist, n1)]->y*unitScale;
    
    x2 = nlist->nodes[find_node(nlist, n2)]->x*unitScale;
    y2 = nlist->nodes[find_node(nlist, n2)]->y*unitScale;
    
    x3 = nlist->nodes[find_node(nlist, n3)]->x*unitScale;
    y3 = nlist->nodes[find_node(nlist, n3)]->y*unitScale;
    
    d1 = x2 - x1;
    d2 = y2 - y1;
    l1 = sqrt(d1*d1 + d2*d2);
    
    d1 = x3 - x2;
    d2 = y3 - y2;
    l2 = sqrt(d1*d1 + d2*d2);
    
    d1 = x3 - x1;
    d2 = y3 - y1;
    l3 = sqrt(d1*d1 + d2*d2);
    
    // Heron's formula
    s = 0.5*(l1 + l2 + l3);
    temp = s*(s - l1)*(s - l2)*(s - l3);
    if (!isfinite(temp)) {
        printf("Computed area in Cartesian co-ordinates is not a number!\n");
        return(-1);
    }
    if (temp < 0) {
        printf("-- Computed area in Cartesian co-ordinates is sqrt(negative)! %g\n", temp);
        printf("-- Lengths of sides of triangle: %g %g %g\n\n", l1, l2, l3);
        return(-1);
    }
    area = sqrt(temp);
    return(area);
}

double arc_length(double theta1, double phi1, double theta2, double phi2)
{
    double diffphi, difftheta;
    double sinetheta;
    double temp;
    
    diffphi   = fabs(phi2 - phi1);
    difftheta = fabs(theta2 - theta1);
    sinetheta = sin(theta1);
    temp      = RADIUS * RADIUS * (difftheta*difftheta + sinetheta*sinetheta*diffphi*diffphi);
    return sqrt(temp);
}

double get_area_polar(nodelist * nlist, int n1, int n2, int n3)
{
    double theta1, phi1, theta2, phi2, theta3, phi3;
    double l1, l2, l3, s;
    double temp;

    theta1 = PIOVER180*(90.0 - nlist->nodes[find_node(nlist, n1)]->lat);
    phi1   = PIOVER180*(90.0 + nlist->nodes[find_node(nlist, n1)]->lon);
    
    theta2 = PIOVER180*(90.0 - nlist->nodes[find_node(nlist, n2)]->lat);
    phi2   = PIOVER180*(90.0 + nlist->nodes[find_node(nlist, n2)]->lon);
    
    theta3 = PIOVER180*(90.0 - nlist->nodes[find_node(nlist, n3)]->lat);
    phi3   = PIOVER180*(90.0 + nlist->nodes[find_node(nlist, n3)]->lon);
    
    l1 = arc_length(theta2, phi2, theta1, phi1)*unitScale;
    l2 = arc_length(theta3, phi3, theta2, phi2)*unitScale;
    l3 = arc_length(theta3, phi3, theta1, phi1)*unitScale;
    
    if (l1 < 0 || l2 < 0 || l3 < 0) {
        printf("Negative arc length: %g %g %g !\n", l1, l2, l3);
    }
    
    // Heron's formula
    s    = 0.5*(l1 + l2 + l3);
    temp = s*(s - l1)*(s - l2)*(s - l3);
    if (!isfinite(temp)) {
        printf("Computed area in polar co-ordinates is not a number!\n");
        return(-1);
    }
    if (temp < 0) {
        printf("-- Computed area in polar co-ordinates is sqrt(negative)! %g\n", temp);
        printf("-- Lengths of sides of triangle: %g %g %g\n\n", l1, l2, l3);
        return(-1);
    }
    temp = sqrt(temp);
    return(temp);
}

void generate_histogram(int which, double max, double min, double value, int ** histData)
{
    int num;
    double strides[PAGES];
    
    strides[which] = (max - min)/499;
    num = ((value - min)/strides[which]);
    if (num < 0 || num > 499) return;
    histData[which][num] += 1;
}

void side_stats(nodelist * nlist, int ** histData, double ** stats)
{
    // First thing -- we have to actually find the sides
    // Each side will be counted twice
    double max, min, deviation, mean;
    int first;
    double * lengths;
    int numLengths;
    
    lengths    = NULL;
    first      = 0;
    numLengths = 0;
    mean       = deviation = 0;
    min        = max       = 0.0;
    for (unsigned int i = 0; i < nlist->numNodes; i++) {
        node * cur = nlist->nodes[i];
        
        for (short j = 0; j < cur->neighbourCount; j++) {
            if (cur->neighbours[j] < cur->number) continue;
            node * cur2 = nlist->nodes[find_node(nlist,cur->neighbours[j])];
            double x1 = cur->x*unitScale;
            double y1 = cur->y*unitScale;
            double x2 = cur2->x*unitScale;
            double y2 = cur2->y*unitScale;
            double length = sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
            
            if (!first) {
                first = 1;
                min = max = length;
            }
            if (length < min) min = length;
            if (length > max) max = length;
            mean = mean + length;
            lengths = realloc(lengths, sizeof(double)*(++numLengths));
            if (!lengths) panic("Out of memory!", 40);
            lengths[numLengths - 1] = length;
        }
    }
    
    mean = mean/numLengths;
	for (int i = 0; i < numLengths; i++) {
		deviation = deviation + (lengths[i] - mean)*(lengths[i] - mean);
        generate_histogram(3, max, min, lengths[i], histData);
    }
    deviation = sqrt(deviation/(numLengths - 1));
    free(lengths);
    
    stats[3][0] = max;
    stats[3][1] = min;
    stats[3][2] = mean;
    stats[3][3] = deviation;
}

void side_stats_polar(nodelist * nlist, int ** histData, double ** stats)
{
    // First thing -- we have to actually find the sides
    // Each side will be counted twice
    double max, min, deviation, mean;
    int first;
    double * lengths;
    int numLengths;
    
    lengths    = NULL;
    first      = 0;
    numLengths = 0;
    mean       = deviation = 0;
    min        = max       = 0.0;
    for (unsigned int i = 0; i < nlist->numNodes; i++) {
        node * cur = nlist->nodes[i];
        
        for (short j = 0; j < cur->neighbourCount; j++) {
            if (cur->neighbours[j] < cur->number) continue;
            node * cur2 = nlist->nodes[find_node(nlist, cur->neighbours[j])];
            double x1 = PIOVER180*(90 + cur->lon);
            double y1 = PIOVER180*(90 - cur->lat);
            double x2 = PIOVER180*(90 + cur2->lon);
            double y2 = PIOVER180*(90 - cur2->lat);
            double length = arc_length(y2, x2, y1, x1)*unitScale;
            
            if (!first) {
                first = 1;
                min = max = length;
            }
            if (length < min) min = length;
            if (length > max) max = length;
            mean = mean + length;
            lengths = realloc(lengths, sizeof(double)*(++numLengths));
            if (!lengths) panic("Out of memory!", 40);
            lengths[numLengths - 1] = length;
        }
    }
    
    mean = mean/numLengths;
    for(int i = 0; i < numLengths; i++) {
        deviation = deviation + (lengths[i] - mean)*(lengths[i] - mean);
        generate_histogram(3, max, min, lengths[i], histData);
    }
    deviation = sqrt(deviation/(numLengths - 1));
    free(lengths);
    
    stats[3][0] = max;
    stats[3][1] = min;
    stats[3][2] = mean;
    stats[3][3] = deviation;
}

void boundary_stats(nodelist * list, int ** histData, double ** stats, int polarStats)
{
    double max, min, mean, deviation;
    double * areas, area;
    int areacount;
    double h1, h2, h3, dh1, dh2, dh3;
    double * deltah;
    int deltas;
    int imax;
    int error = 0;
    double dt, deepest;
    double * dts;
    int dtcount;
    	
    if (!list) return;
    
    deltah    = NULL;
    areas     = NULL;
    dts       = NULL;
    areacount = 0;
    dtcount   = 0;
    deltas    = 0;
    for (int j = 0; j < PAGES; j++)
        for (int i = 0; i < 500; i++)
            histData[j][i] = 0;
    
    // Find element area's first
    for (unsigned int i = 0; i < list->numNodes; i++) {
        node * cur = list->nodes[i];
        short idx = cur->neighbourCount;
        
        for (short j = 0; j < idx; j++) {
            if (cur->neighbours[j] <= cur->number) continue;
            node * cur2 = list->nodes[find_node(list, cur->neighbours[j])];
            short idx2 = cur2->neighbourCount;
            
            for (short k = 0; k < idx2; k++) {
                for (short l = 0; l < idx; l++) {
                    unsigned int a = cur2->neighbours[k];
                    unsigned int b = cur->neighbours[l];
                    
                    if (a == b && a != cur->number && b != cur2->number && a > cur->number && a > cur2->number) {
                        int first, second, temp;
                        
                        if (j < l) {
                            first  = j;
                            second = l;
                        } else {
                            first  = l;
                            second = j;
                        }
                        if ((j == 0 && l == cur->neighbourCount - 1) || (l == 0 && j == cur->neighbourCount - 1)) {
                            temp   = first;
                            first  = second;
                            second = temp;
                        }
                        if (polarStats)
                            area = get_area_polar(list, cur->number, cur->neighbours[first], cur->neighbours[second]);
                        else
                            area = get_area(list, cur->number, cur->neighbours[first], cur->neighbours[second]);
                        
                        if (area == -1) error = 1;
                        
                        areas = realloc(areas, sizeof(double)*(++areacount));
                        if (!areas) panic("No memory!", 4);
                        areas[areacount - 1] = area;
                        
                        deepest = cur->depth;
                        if (list->nodes[find_node(list, cur->neighbours[first])]->depth > deepest)
                            deepest = list->nodes[find_node(list, cur->neighbours[first])]->depth;
                        if (list->nodes[find_node(list, cur->neighbours[second])]->depth > deepest)
                            deepest = list->nodes[find_node(list, cur->neighbours[second])]->depth;
                        
                        if (deepest < cutoffValue) deepest = cutoffValue;
                        dt = sqrt(area/(9.8*deepest));
                        dts = realloc(dts, sizeof(double)*(++dtcount));
                        dts[dtcount - 1] = dt;
                        
                        h1 = cur->depth;
                        h2 = list->nodes[find_node(list, cur->neighbours[first])]->depth;
                        h3 = list->nodes[find_node(list, cur->neighbours[second])]->depth;
                        
                        min = fabs(h1);
                        if (fabs(h2) < min) min = fabs(h2);
                        if (fabs(h3) < min) min = fabs(h3);
                        
                        if (min < cutoffValue) min = cutoffValue;
                        if (h1 < cutoffValue)  h1 = cutoffValue;
                        if (h2 < cutoffValue)  h2 = cutoffValue;
                        if (h3 < cutoffValue)  h3 = cutoffValue;
                        if (min != 0) {
                            deltah = realloc(deltah, sizeof(double)*(++deltas)*3);
                            if (!deltah) panic("Out of memory!", 5);
                            dh1 = fabs(h2 - h1)/min;
                            dh2 = fabs(h3 - h2)/min;
                            dh3 = fabs(h3 - h1)/min;
                            deltah[(deltas - 1)*3]     = dh1;
                            deltah[(deltas - 1)*3 + 1] = dh2;
                            deltah[(deltas - 1)*3 + 2] = dh3;
                        }
                        
                    }
                }
            }
        }
    }
    list->numElems = areacount;
    
    max  = min = areas[0];
    mean = 0;
    if (error == 0) {
        for (int i = 0; i < areacount; i++) {
            if (areas[i] > max) max = areas[i];
            if (areas[i] < min) min = areas[i];
            mean += areas[i];
        }
        
        mean      = mean/areacount;
        deviation = 0;
        for (int i = 0; i < areacount; i++) {
            deviation = deviation + (areas[i] - mean)*(areas[i] - mean);
            generate_histogram(1, max, min, areas[i], histData);
        }
        deviation = sqrt(deviation/(areacount - 1));
    } else {
        max = min = mean = deviation = HUGE_VAL;
    }
    free(areas);
    
    stats[1][0] = max;
    stats[1][1] = min;
    stats[1][2] = mean;
    stats[1][3] = deviation;
    
    // Now do depth
    max  = min = list->nodes[0]->depth;
    mean = 0;
    for (unsigned int i = 0; i < list->numNodes; i++) {
        double curDepth = list->nodes[i]->depth;
        if (curDepth > max) max = curDepth;
        if (curDepth < min) min = curDepth;
        mean += curDepth;
    }
    
    mean      = mean/list->numNodes;
    deviation = 0;
    for (unsigned int i = 0; i < list->numNodes; i++) {
        double curDepth = list->nodes[i]->depth;
        deviation = deviation + (curDepth - mean)*(curDepth - mean);
        generate_histogram(2, max, min, curDepth, histData);
    }
    deviation = sqrt(deviation/(list->numNodes - 1));
    
    stats[2][0] = max;
    stats[2][1] = min;
    stats[2][2] = mean;
    stats[2][3] = deviation;
    
    // Side stats
    if (polarStats)
        side_stats_polar(list, histData, stats);
    else
        side_stats(list, histData, stats);
    
    // Now find delta h over h
    min  = max = deltah[0];
    mean = 0;
    for (int i = 0; i < deltas*3; i++) {
        if (deltah[i] < min) min = deltah[i];
        if (deltah[i] > max) max = deltah[i];
        mean += deltah[i];
    }
    
    mean      = mean/(deltas*3);
    deviation = 0;
    for (int i = 0; i < (deltas*3); i++) {
        generate_histogram(0, max, min, deltah[i], histData);
        deviation = deviation + (deltah[i] - mean)*(deltah[i] - mean);
    }
    deviation = sqrt(deviation/(deltas*3 - 1));
    free(deltah);
    
    stats[0][0] = max;
    stats[0][1] = min;
    stats[0][2] = mean;
    stats[0][3] = deviation;
    
    // Delta t
    if (error == 0) {
        min  = max = dts[0];
        mean = 0;
        for (int i = 0; i < dtcount; i++) {
            if (dts[i] > max) max = dts[i];
            if (dts[i] < min) min = dts[i];
            mean += dts[i];
        }
        
        mean      = mean/dtcount;
        deviation = 0;
        for (int i = 0; i < dtcount; i++) {
            generate_histogram(4, max, min, dts[i], histData);
            deviation = deviation + (dts[i] - mean)*(dts[i] - mean);
        }
        deviation = sqrt(deviation/(dtcount - 1));
    } else {
        max = min = mean = deviation = HUGE_VAL;
    }
    free(dts);
    
    stats[4][0] = max;
    stats[4][1] = min;
    stats[4][2] = mean;
    stats[4][3] = deviation;
    
	// Finally, do bandwidth
    imax = 0;
    for (unsigned int i = 0; i < list->numNodes; i++) {
        node * cur = list->nodes[i];
        
        for (short j = 0; j < cur->neighbourCount; j++) {
            if (abs((int)cur->number - (int)cur->neighbours[j]) > imax)
                imax = abs((int)cur->number - (int)cur->neighbours[j]);
        }
    }
    
    imax = 2*imax + 1;
    // TODO: Put this (mesh bandwidth) somewhere
}

void print_general(nodelist * list, int polar, int minMax, int which, const char * file)
{
    char mesh[128];
    int count;
    double ma, mad, cad, ddm;
    FILE * fp = fopen(file, "w");
    if (!fp) return;
    
    strcpy(mesh, strrchr(file, '/') + 1);
    *strrchr(mesh, '.') = '\0';
    fprintf(fp, "%s\nFile generated by MeshInspector\n", mesh);
    
    count = 0;
    ma    = 0;
    mad   = cad = 0;
    ddm   = 99999;
    for (unsigned int i = 0; i < list->numNodes; i++) {
        unsigned int firstA = 1;
        node * cur = list->nodes[i];
        
        for (short j = 0; j < cur->neighbourCount; j++) {
            node * cur2 = list->nodes[find_node(list, cur->neighbours[j])];
            
            for (short k = 0; k < cur->neighbourCount; k++) {
                node * cur3 = list->nodes[find_node(list, cur->neighbours[k])];
                unsigned int first = cur->neighbours[k];
                
                for (short l = 0; l < cur2->neighbourCount; l++) {
                    if (k == l) continue;
                    unsigned int second = cur2->neighbours[l];
                    if (second == first) {
                        // We have a winner
                        double ca;
                        if (polar) {
                            ca = get_area_polar(list, cur->number, cur2->number, cur->neighbours[k]);
                        } else {
                            ca = get_area(list, cur->number, cur2->number, cur->neighbours[k]);
                        }
                        
                        double deepest = cur->depth;
                        if (cur2->depth > deepest) deepest = cur2->depth;
                        if (cur3->depth > deepest) deepest = cur3->depth;
                        if (deepest < cutoffValue) deepest = cutoffValue;
                        cad = sqrt(ca/(9.8*deepest));
                        
                        double h1 = cur->depth;
                        double h2 = cur2->depth;
                        double h3 = cur3->depth;
                        double min = fabs(h1);
                        
                        if (fabs(h2) < min) min = fabs(h2);
                        if (fabs(h3) < min) min = fabs(h3);
                        if (min < cutoffValue) min = cutoffValue;
                        
                        if (h1 < cutoffValue) h1 = cutoffValue;
                        if (h2 < cutoffValue) h2 = cutoffValue;
                        if (h3 < cutoffValue) h3 = cutoffValue;
                        
                        double dmin = 0;
                        double dmax = 0;
                        if (min != 0) {
                            double dh1 = fabs(h2 - h1)/min;
                            double dh2 = fabs(h3 - h2)/min;
                            double dh3 = fabs(h3 - h1)/min;
                            
                            dmax = dh1;
                            if (dh2 > dmax) dmax = dh2;
                            if (dh3 > dmax) dmax = dh3;
                            
                            dmin = dh1;
                            if (dh2 < dmin) dmin = dh2;
                            if (dh3 < dmin) dmin = dh3;
                        }
                        
                        if (minMax) {
                            if (ca < ma || firstA)    ma  = ca;
                            if (cad < mad || firstA)  mad = cad;
                            if (dmin < ddm || firstA) ddm = dmin;
                        } else {
                            if (ca > ma || firstA)    ma  = ca;
                            if (cad > mad || firstA)  mad = cad;
                            if (dmax > ddm || firstA) ddm = dmax;
                        }
                        firstA = 0;
                    }
                }
            }
        }
        
        switch (which) {
            case EXPORT_AREA:
                fprintf(fp, "%d\t%g\n", count++, ma);
                break;
            case EXPORT_D_T:
                fprintf(fp, "%d\t%g\n", count++, mad);
                break;
            case EXPORT_D_H:
                fprintf(fp, "%d\t%g\n", count++, ddm);
                break;
        }
    }
    fclose(fp);
}

void print_depth(nodelist * list, const char * file)
{
    char mesh[128];
    int count = 0;
    FILE * fp = fopen(file, "w");
    if (!fp) return;
    
    strcpy(mesh, strrchr(file, '/') + 1);
    *strrchr(mesh, '.') = '\0';
    fprintf(fp, "%s\nFile generated by MeshInspector\n", mesh);
    
    for (unsigned int i = 0; i < list->numNodes; i++) {
        node * cur = list->nodes[i];
        fprintf(fp, "%d\t%g\n", count++, cur->depth);
    }
    fclose(fp);
}

void print_side(nodelist * list, int polar, int minMax, const char * file)
{
    char mesh[128];
    int count = 0;
    double mlen = 0.0;
    FILE * fp = fopen(file, "w");
    if (!fp) return;
    
    strcpy(mesh, strrchr(file, '/') + 1);
    *strrchr(mesh, '.') = '\0';
    fprintf(fp, "%s\nFile generated by MeshInspector\n", mesh);
    
    for (unsigned int i = 0; i < list->numNodes; i++) {
        node * cur = list->nodes[i];
        
        for (short j = 0; j < cur->neighbourCount; j++) {
            double curlen;
            node * cur2 = list->nodes[find_node(list, cur->neighbours[j])];
            
            if (polar) {
                double x1 = PIOVER180*(90 + cur->x);  double y1 = PIOVER180*(90 - cur->y);
                double x2 = PIOVER180*(90 + cur2->x); double y2 = PIOVER180*(90 - cur2->y);
                curlen = arc_length(y2, x2, y1, x1)*unitScale;
            } else {
                double x1 = cur->x*unitScale;  double y1 = cur->y*unitScale;
                double x2 = cur2->x*unitScale; double y2 = cur2->y*unitScale;
                curlen = sqrt((x2 - x1)*(x2 - x1)+(y2 - y1)*(y2 - y1));
            }
            if (minMax) {
                if (curlen < mlen || j == 0) mlen = curlen;
            } else {
                if (curlen > mlen || j == 0) mlen = curlen;
            }
        }
        fprintf(fp, "%d\t%g\n", count++, mlen);
    }
    fclose(fp);
}

