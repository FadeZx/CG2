class HalfSpace {
public:
    float a, b, c, d; // Plane equation Ax + By + Cz + D = 0

    // Default constructor
    HalfSpace() : a(0), b(0), c(0), d(0) {}

    // Constructor to initialize with coefficients directly
    HalfSpace(float A, float B, float C, float D) : a(A), b(B), c(C), d(D) {}

    // Constructor that defines a plane from three points
    HalfSpace(const Point& p1, const Point& p2, const Point& p3) {
        Vector u = p2 - p1;
        Vector v = p3 - p1;
        Vector n = cross(u, v);
        n.Normalize();
        a = n.x;
        b = n.y;
        c = n.z;
        d = -(n.x * p1.x + n.y * p1.y + n.z * p1.z);
    }

    // Constructor that defines a plane from a normal and a point
    HalfSpace(const Vector& normal, const Point& point) {
        Vector n = normal;
        n.Normalize(); // Ensure the normal is unit length
        a = n.x;
        b = n.y;
        c = n.z;
        d = -(n.x * point.x + n.y * point.y + n.z * point.z);
    }

    // Access the plane equation as an Hcoords (for easier calculations)
    Hcoords equation() const {
        return Hcoords(a, b, c, d);
    }
};
