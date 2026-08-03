
// struct Collider {
//     virtual float3 support(const float3 &direction) const = 0;
// };


// struct Simplex {
//     float3 points[4];
//     int size;

//     Simplex() : size{0} {}

//     void reduceToPoint(int idx){
//         size = 1;
//         points[0] = points[idx];
//     }

//     void reduceToLine(int a, int b){
//         size = 2;
//         if(a > b) std::swap(a, b);

//         // avoid needing temporary memory 
//         // by always moving the lowest index point
//         points[0] = points[a];
//         points[1] = points[b];
//     }

//     void reduceToTriangle(int a, int b, int c){
//         size = 2;
//         if(a > b) std::swap(a, b);
//         if(a > c) std::swap(a, c);
//         if(b > c) std::swap(b, c);

//         points[0] = points[a];
//         points[1] = points[b];
//         points[2] = points[c];
//     }

//     void push_front(float3 feature){
//         points[3] = points[2];
//         points[2] = points[1];
//         points[1] = points[0];
//         points[0] = feature;
//         size = std::min(size + 1, 4);
//     }
// };

// inline bool testAndUpdateLine(Simplex &simplex, float3 &direction){
//     float3 a = simplex.points[0];
//     float3 b = simplex.points[1];
//     float3 toOrigin = -a;
    
//     // the origin cannot be beyond b or a
//     // they would have been rejected during the cross origin check

//     // search in the perpendicular direction towards the origin 
//     direction = cross(cross(b - a, toOrigin), b - a);

//     // continue search
//     return false;
// }

// // https://medium.com/@mbayburt/walkthrough-of-the-gjk-collision-detection-algorithm-80823ef5c774
// // https://winter.dev/articles/gjk-algorithm/
// // https://www.youtube.com/watch?v=ajv46BSqcK4&t=60s
// bool testAndUpdateTriangle(Simplex &simplex, float3 &direction){
//     float3 a = simplex.points[0];
//     float3 b = simplex.points[1];
//     float3 c = simplex.points[2];
//     float3 toOrigin = -a;

//     // the origin cannot be beyond the line bc
//     float3 face_normal = cross(b - a, c - a);
//     float3 ac_normal = cross(face_normal, c - a);
//     float3 ab_normal = cross(face_normal, b - a);
//     if(dot(toOrigin, ac_normal) > 0){
//         // within the ac edge region
//         simplex.reduceToLine(0, 2);
//         direction = ac_normal;
//     }
//     else if(dot(toOrigin, ab_normal) > 0){
//         // within the ac edge region
//         simplex.reduceToLine(0, 1);
//         direction = ab_normal;
//     }
//     else {
//         // within the triangle
//         // need to check if we are above or below the triangle
//         if(dot(toOrigin, face_normal) > 0){
//             direction = face_normal;
//         } 
//         else {
//             // swap the vertices so that the face normal point towards the origin
//             // i.e. ensure the triangle is wound counter clockwise relative to dir
//             std::swap(simplex.points[1], simplex.points[2]);
//             direction = -face_normal;
//         }
        
//     }

//     return false;
// }

// bool testAndUpdateTetrahedron(Simplex &simplex, float3 &direction){
//     float3 a = simplex.points[0];
//     float3 b = simplex.points[1];
//     float3 c = simplex.points[2];
//     float3 d = simplex.points[3];

//     // we know that the origin cannot be beyond bcd by construction of a
//     // we also know that it is not before a since a would have been rejected

//     // do side tests on each triangle face
//     // assume that the bcd triangle is wound counter clockwise w.r.t d
//     float3 abc_normal = cross(b - a, c - a);
//     float3 acd_normal = cross(c - a, d - a);
//     float3 adb_normal = cross(d - a, b - a);
//     float3 toOrigin = -a;

//     if(dot(toOrigin, abc_normal) > 0) {
//         simplex.reduceToTriangle(0, 1, 2);
//         direction = abc_normal;
//     }
//     else if(dot(toOrigin, acd_normal) > 0) {
//         simplex.reduceToTriangle(0, 2, 3);
//         direction = acd_normal;
//     }



//     return false;
// }


// bool testAndUpdateSimplex(Simplex &simplex, float3 &direction){
//     if(simplex.size == 0){
//         direction = float3(1.0, 0.0, 0.0);
//     }
//     else if(simplex.size == 1){
//         direction = -direction;
//     }
//     else if(simplex.size == 2){
//         return testAndUpdateLine(simplex, direction);
//     }
//     else if(simplex.size == 3){
//         return testAndUpdateTriangle(simplex, direction);
//     }
//     else { // if size is 4
//         return testAndUpdateTetrahedron(simplex, direction);
//     } 
        
//     return false;
// }

// float3 support(const Collider &shapeA, const Collider &shapeB, float3 direction){
//     return shapeA.support(direction) - shapeB.support(-direction);
// }


// bool GJK(const Collider &shapeA, const Collider &shapeB, const float3 &initial_axis){
//     float3 direction(initial_axis);
    
//     Simplex simplex;
//     simplex.push_front(support(shapeA, shapeB, direction));
//     direction = -simplex.points[0];

//     while(true){
//         float3 next_feature = support(shapeA, shapeB, direction);

//         // if the most positive point is negative, 
//         // then the entire region is is the negative half space
//         // thus we can reject the intersection
//         if (dot(next_feature, direction) < 0) return false;

//         // check if the simplex contains the origin
//         // update it if not.
//         simplex.push_front(next_feature);
//         if(testAndUpdateSimplex(simplex, direction)) return true;
//     }
// }

