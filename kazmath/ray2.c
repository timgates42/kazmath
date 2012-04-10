#include <assert.h>
#include <stdio.h>
#include "ray2.h"

void kmRay2Fill(kmRay2* ray, kmScalar px, kmScalar py, kmScalar vx, kmScalar vy) {
    ray->start.x = px;
    ray->start.y = py;    
    ray->dir.x = vx;
    ray->dir.y = vy;
}

kmBool kmRay2IntersectLineSegment(const kmRay2* ray, const kmVec2* p1, const kmVec2* p2, kmVec2* intersection) {
    
    float x1 = ray->start.x;
    float y1 = ray->start.y;
    float x2 = ray->start.x + ray->dir.x;
    float y2 = ray->start.y + ray->dir.y;
    float x3 = p1->x;
    float y3 = p1->y;
    float x4 = p2->x;
    float y4 = p2->y;

    float denom = (y4 -y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
    
    //If denom is zero, the lines are parallel
    if(denom > -kmEpsilon && denom < kmEpsilon) {
        return KM_FALSE;
    }
    
    float ua = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denom;
    float ub = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denom;
    
    float x = x1 + ua * (x2 - x1);
    float y = y1 + ua * (y2 - y1);
    
    if(x < min(p1->x, p2->x) - kmEpsilon || 
       x > max(p1->x, p2->x) + kmEpsilon ||
       y < min(p1->y, p2->y) - kmEpsilon || 
       y > max(p1->y, p2->y) + kmEpsilon) {
        //Outside of line
        //printf("Outside of line, %f %f (%f %f)(%f, %f)\n", x, y, p1->x, p1->y, p2->x, p2->y);
        return KM_FALSE;
    }
    
    if(x < min(x1, x2) - kmEpsilon || 
       x > max(x1, x2) + kmEpsilon ||
       y < min(y1, y2) - kmEpsilon || 
       y > max(y1, y2) + kmEpsilon) {
        //printf("Outside of ray, %f %f (%f %f)(%f, %f)\n", x, y, x1, y1, x2, y2);
        return KM_FALSE;
    }
    
    intersection->x = x;
    intersection->y = y;
    
    return KM_TRUE;
    
    
/*    
    kmScalar A1, B1, C1;
    kmScalar A2, B2, C2;
    
    A1 = ray->dir.y;
    B1 = ray->dir.x;
    C1 = A1 * ray->start.x + B1 * ray->start.y;
    
    A2 = p2->y - p1->y;
    B2 = p2->x - p1->x;
    C2 = A2 * p1->x + B2 * p1->y;
    
    double det = (A1 * B2) - (A2 * B1);
    if(det == 0) {
        printf("Parallel\n");
        return KM_FALSE;
    }
    
    double x = (B2*C1 - B1*C2) / det;
    double y = (A1*C2 - A2*C1) / det;
    
    if(x < min(p1->x, p2->x) - kmEpsilon || 
       x > max(p1->x, p2->x) + kmEpsilon ||
       y < min(p1->y, p2->y) - kmEpsilon || 
       y > max(p1->y, p2->y) + kmEpsilon) {
        //Outside of line
        printf("Outside of line, %f %f (%f %f)(%f, %f)\n", x, y, p1->x, p1->y, p2->x, p2->y);
        return KM_FALSE;
    }
    
    kmScalar x1 = ray->start.x;
    kmScalar x2 = ray->start.x + ray->dir.x;
    
    kmScalar y1 = ray->start.y;
    kmScalar y2 = ray->start.y + ray->dir.y;
    
    if(x < min(x1, x2) - kmEpsilon || 
       x > max(x1, x2) + kmEpsilon ||
       y < min(y1, y2) - kmEpsilon || 
       y > max(y1, y2) + kmEpsilon) {
        printf("Outside of ray, %f %f (%f %f)(%f, %f)\n", x, y, x1, y1, x2, y2);
        return KM_FALSE;
    }
    
    intersection->x = x;
    intersection->y = y;
    
    return KM_TRUE;*/
}

void calculate_line_normal(kmVec2 p1, kmVec2 p2, kmVec2 other_point, kmVec2* normal_out) {
    /*
        A = (3,4)
        B = (2,1)
        C = (1,3)

        AB = (2,1) - (3,4) = (-1,-3)
        AC = (1,3) - (3,4) = (-2,-1)
        N = n(AB) = (-3,1)
        D = dot(N,AC) = 6 + -1 = 5

        since D > 0:
          N = -N = (3,-1)
    */
    
    kmVec2 edge, other_edge;
    kmVec2Subtract(&edge, &p2, &p1);
    kmVec2Subtract(&other_edge, &other_point, &p1);
    
    kmVec2 n;
    n.x = edge.y;
    n.y = -edge.x;
    
    float d = kmVec2Dot(&n, &other_edge);
    if(d > 0.0f) {
        n.x = -n.x;
        n.y = -n.y;
    }
    normal_out->x = n.x;
    normal_out->y = n.y;
}

kmBool kmRay2IntersectTriangle(const kmRay2* ray, const kmVec2* p1, const kmVec2* p2, const kmVec2* p3, kmVec2* intersection, kmVec2* normal_out) {
    kmVec2 intersect;
    kmVec2 final_intersect;
    kmVec2 normal;
    kmScalar distance = 10000.0f;
    kmBool intersected = KM_FALSE;
 
    if(kmRay2IntersectLineSegment(ray, p1, p2, &intersect)) {
        intersected = KM_TRUE;                        

        kmVec2 tmp;
        kmScalar this_distance = kmVec2Length(kmVec2Subtract(&tmp, &intersect, &ray->start));
        if(this_distance < distance) {
            final_intersect.x = intersect.x;
            final_intersect.y = intersect.y;
            distance = this_distance;
                        
            calculate_line_normal(*p1, *p2, *p3, &normal);                           
        }
    }
    
    if(kmRay2IntersectLineSegment(ray, p2, p3, &intersect)) {
        intersected = KM_TRUE;                    
        
        kmVec2 tmp;
        kmScalar this_distance = kmVec2Length(kmVec2Subtract(&tmp, &intersect, &ray->start));
        if(this_distance < distance) {
            final_intersect.x = intersect.x;
            final_intersect.y = intersect.y;
            distance = this_distance;
                        
            calculate_line_normal(*p2, *p3, *p1, &normal);                           
        }   
    }
    
    if(kmRay2IntersectLineSegment(ray, p3, p1, &intersect)) {
        intersected = KM_TRUE;                    
        
        kmVec2 tmp;
        kmScalar this_distance = kmVec2Length(kmVec2Subtract(&tmp, &intersect, &ray->start));
        if(this_distance < distance) {
            final_intersect.x = intersect.x;
            final_intersect.y = intersect.y;
            distance = this_distance;

            calculate_line_normal(*p3, *p1, *p2, &normal);                           
        }          
    }
    
    if(intersected) {
        intersection->x = final_intersect.x;
        intersection->y = final_intersect.y;
        if(normal_out) {
            normal_out->x = normal.x;
            normal_out->y = normal.y;
        }
    }
    
    return intersected;
}

kmBool kmRay2IntersectCircle(const kmRay2* ray, const kmVec2 centre, const kmScalar radius, kmVec2* intersection) {
    assert(0 && "Not implemented");
}
