#ifndef TOOLS_VECTORS_HPP
#define TOOLS_VECTORS_HPP

#include <cmath>
#include <iostream>

inline float sq (float x) {return x*x;}

#define tpl template <int n, typename T>
#define VecnT Vec<n,T>

template <int n, typename T=float> class Vec;

template<typename T> class Vec<2,T> {
public:
    union {
        T data[2];
        struct { T x, y; };
    };
    Vec() : x(0), y(0) {}
    explicit Vec(T v) : x(v), y(v) {}
    explicit Vec(T x, T y) : x(x), y(y) {}
    inline T &operator[] (int i) {return data[i];}
    inline const T &operator[] (int i) const {return data[i];}
};

template<typename T> class Vec<3,T> {
public:
    union {
        T data[3];
        struct { T x, y, z; };
    };
    Vec() : x(0), y(0) {}
    explicit Vec(T v) : x(v), y(v), z(v) {}
    explicit Vec(T x, T y, T z) : x(x), y(y), z(z) {}
    inline T &operator[] (int i) {return data[i];}
    inline const T &operator[] (int i) const {return data[i];}
};

template<typename T> class Vec<4,T> {
public:
    union {
        T data[4];
        struct { T x, y, z, w; };
    };
    Vec() : x(0), y(0) {}
    explicit Vec(T v) : x(v), y(v), z(v), w(v) {}
    explicit Vec(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    inline T &operator[] (int i) {return data[i];}
    inline const T &operator[] (int i) const {return data[i];}
};

tpl VecnT operator+ (const VecnT &u) {return u;}
tpl VecnT operator+ (const VecnT &u, const VecnT &v) {VecnT w; for (int i = 0; i < n; i++) w[i] = u[i] + v[i]; return w;}
tpl VecnT &operator+= (VecnT &u, const VecnT &v) {return u = u + v;}
tpl VecnT operator- (const VecnT &u) {VecnT v; for (int i = 0; i < n; i++) v[i] = -u[i]; return v;}
tpl VecnT operator- (const VecnT &u, const VecnT &v) {return u + (-v);}
tpl VecnT &operator-= (VecnT &u, const VecnT &v) {return u = u - v;}
tpl VecnT operator* (const T &a, const VecnT &u) {VecnT v; for (int i = 0; i < n; i++) v[i] = a*u[i]; return v;}
tpl VecnT operator* (const VecnT &u, const T &a) {return a*u;}
tpl VecnT &operator*= (VecnT &u, const T &a) {return u = u*a;}
tpl VecnT operator/ (const VecnT &u, const T &a) {return u*(1/a);}
tpl VecnT &operator/= (VecnT &u, const T &a) {return u = u/a;}
tpl bool operator== (const VecnT &u, const VecnT &v) {for(int i=0; i<n; ++i) if(u[i] != v[i]) return false; return true;}
tpl bool operator!= (const VecnT &u, const VecnT &v) {return !(u==v);}
tpl T dot (const VecnT &u, const VecnT &v) {T d = 0; for (int i = 0; i < n; i++) d += u[i]*v[i]; return d;}
tpl T norm2 (const VecnT &u) {return dot(u,u);}
tpl T norm (const VecnT &u) {return sqrt(norm2(u));}
tpl VecnT normalize (const VecnT &u) {T m = norm(u); return m==0 ? VecnT(0) : u/m;}
tpl std::ostream &operator<< (std::ostream &out, const VecnT &u) {out << "("; for (int i = 0; i < n; i++) out << (i==0?"":", ") << u[i]; out << ")"; return out;}
template <typename T> Vec<3,T> cross (const Vec<3,T> &u, const Vec<3,T> &v) {Vec<3,T> w; w[0] = u[1]*v[2] - u[2]*v[1]; w[1] = u[2]*v[0] - u[0]*v[2]; w[2] = u[0]*v[1] - u[1]*v[0]; return w;}
template <typename T> T stp (const Vec<3,T> &u, const Vec<3,T> &v, const Vec<3,T> &w) {return dot(u,cross(v,w));}
template <int m, int n, typename T> Vec<m,T> project (const VecnT &u) {Vec<m,T> v; for (int i = 0; i < m; i++) v[i] = (i<n) ? u[i] : 0; return v;}
template <typename T> Vec<2,T> perp (const Vec<2,T> &u) {return Vec<2,T>(-u[1],u[0]);}
tpl VecnT elmult(const VecnT& a, const VecnT& b) { VecnT c; for (int i=0; i<n; i++) c[i] = a[i]*b[i]; return c; }
template<int n> Vec<n> toVec(const Vec<n,int>& v) { Vec<n> a; for (int i=0; i<n; i++) a[i] = v[i]; return a; }
template<int n> Vec<n, int> toVeci(const Vec<n>& v) { Vec<n, int> a; for (int i=0; i<n; i++) a[i] = floor(v[i]); return a; }

#undef tpl
#undef VecnT

typedef Vec<2> Vec2;
typedef Vec<3> Vec3;
typedef Vec<4> Vec4;
typedef Vec<2,int> Vec2i;
typedef Vec<3,int> Vec3i;
typedef Vec<4,int> Vec4i;

#define tpl template <int m, int n, typename T>
#define MatmnT Mat<m,n,T>
#define MatnmT Mat<n,m,T>
#define MatnnT Mat<n,n,T>
#define VecmT Vec<m,T>
#define VecnT Vec<n,T>

template <int m, int n, typename T=double> class Mat {
private:
    VecmT c[n];
public:
    Mat () {for (int j = 0; j < n; j++) c[j] = VecmT(0);}
    explicit Mat (T x) {for (int j = 0; j < n; j++) {c[j] = VecmT(0); if (j < m) c[j][j] = x;}}
    explicit Mat (VecmT x, VecmT y) {static_assert(n==2, ""); c[0] = x; c[1] = y;}
    explicit Mat (VecmT x, VecmT y, VecmT z) {static_assert(n==3, ""); c[0] = x; c[1] = y; c[2] = z;}
    explicit Mat (VecmT x, VecmT y, VecmT z, VecmT w) {static_assert(n==4, ""); c[0] = x; c[1] = y; c[2] = z; c[3] = w;}
//aa:    static Mat rows (VecnT x, VecnT y) {return Mat<n,2,T>(x,y).t();}
//aa:    static Mat rows (VecnT x, VecnT y, VecnT z) {return Mat<n,3,T>(x,y,z).t();}
	static Mat rows (VecnT x, VecnT y) { Mat<2,n,T> M; for(int i = 0; i < n; i++) { M.col(i)[0] = x[i]; M.col(i)[1] = y[i]; } return M; }
    static Mat rows (VecnT x, VecnT y, VecnT z) { Mat<3,n,T> M; for(int i = 0; i < n; i++) { M.col(i)[0] = x[i]; M.col(i)[1] = y[i]; M.col(i)[2] = z[i];} return M; }
    static Mat rows (VecnT x, VecnT y, VecnT z, VecnT w) { Mat<4,n,T> M; for(int i = 0; i < n; i++) { M.col(i)[0] = x[i]; M.col(i)[1] = y[i]; M.col(i)[2] = z[i]; M.col(i)[3] = w[i];} return M; }
    VecnT row (int i) const {VecnT R; for(int col = 0; col < n; ++col) { R[col] = c[col][i]; } return R; }


    T &operator() (int i, int j) {return c[j][i];}
    const T &operator() (int i, int j) const {return c[j][i];}
    VecmT &col (int j) {return c[j];}
    const VecmT &col (int j) const {return c[j];}
    MatnmT t () const {return transpose(*this);}
	// const MatTransposed<m,n,T>& t () const {return reinterpret_cast<const MatTransposed<m,n,T>&>(*this);}
    MatmnT inv () const {return inverse(*this);}
};
tpl MatmnT operator+ (const MatmnT &A) {return A;}
tpl MatmnT operator+ (const MatmnT &A, const MatmnT &B) {MatmnT C; for (int j = 0; j < n; j++) C.col(j) = A.col(j) + B.col(j); return C;}
tpl MatmnT &operator+= (MatmnT &A, const MatmnT &B) {return A = A + B;}
tpl MatmnT operator- (const MatmnT &A) {MatmnT B; for (int j = 0; j < n; j++) B.col(j) = -A.col(j); return B;}
tpl MatmnT operator- (const MatmnT &A, const MatmnT &B) {return A + (-B);}
tpl MatmnT &operator-= (MatmnT &A, const MatmnT &B) {return A = A - B;}
tpl MatmnT operator* (const T &a, const MatmnT &A) {MatmnT B; for (int j = 0; j < n; j++) B.col(j) = a*A.col(j); return B;}
tpl MatmnT operator* (const MatmnT &A, const T &a) {return a*A;}
tpl MatmnT &operator*= (MatmnT &A, const T &a) {return A = A*a;}
tpl MatmnT operator/ (const MatmnT &A, const T &a) {return A*(1/a);}
tpl MatmnT &operator/= (MatmnT &A, const T &a) {return A = A/a;}
tpl VecmT operator* (const MatmnT &A, const VecnT &u) {VecmT v = VecmT(0); for (int j = 0; j < n; j++) v += A.col(j)*u[j]; return v;}
template <int m, int n, int o, typename T> Mat<m,o,T> operator* (const Mat<m,n,T> &A, const Mat<n,o,T> &B) {Mat<m,o,T> C; for (int k = 0; k < o; k++) C.col(k) = A*B.col(k); return C;}
tpl MatmnT *operator*= (const MatmnT &A, const MatnnT &B) {return A = A*B;}
tpl MatnmT transpose (const MatmnT &A) {MatnmT B; for (int i = 0; i < m; i++) for (int j = 0; j < n; j++) B(j,i) = A(i,j); return B;}
template <int n, typename T> VecnT diag (const MatnnT &A) {VecnT u; for (int j = 0; j < n; j++) u[j] = A(j,j); return u;}
template <int n, typename T> T trace (const MatnnT &A) {T t = 0; for (int j = 0; j < n; j++) t += A(j,j); return t;}
template <typename T> T det (const Mat<2,2,T> &A) {return A(0,0)*A(1,1) - A(0,1)*A(1,0);}
template <typename T> T det (const Mat<3,3,T> &A) {return stp(A.col(0), A.col(1), A.col(2));}
template <typename T> Mat<2,2,T> inverse (const Mat<2,2,T> &A) {return Mat<2,2,T>(Vec<2,T>(A(1,1), -A(1,0)), Vec<2,T>(-A(0,1), A(0,0)))/det(A);}
template <typename T> T wedge (const Vec<2,T> &u, const Vec<2,T> &v) {return u[0]*v[1] - u[1]*v[0];}
template <typename T> Mat<3,3,T> inverse (const Mat<3,3,T> &A) {return Mat<3,3,T>(cross(A.col(1),A.col(2)), cross(A.col(2),A.col(0)), cross(A.col(0),A.col(1))).t()/det(A);}
template <int n, typename T> MatnnT diag (const VecnT &u) {MatnnT A = MatnnT(0); for (int j = 0; j < n; j++) A(j,j) = u[j]; return A;}
tpl MatmnT outer (const VecmT &u, const VecnT &v) {MatmnT A; for (int j = 0; j < n; j++) A.col(j) = u*v[j]; return A;}
tpl std::ostream &operator<< (std::ostream &out, const MatmnT &A) {MatnmT At = transpose(A); out << "(" << std::endl; for (int i = 0; i < m; i++) out << "    " << At.col(i) << (i+1==m?"":",") << std::endl; out << ")"; return out;}
template <typename T> bool right_handed (const Vec<3,T> &u, const Vec<3,T> &v, const Vec<3,T> &w) {return stp(u,v,w) >= 0;}

// Frobenius norm
tpl T norm2_F (const MatmnT &A) {T a = 0; for (int j = 0; j < n; j++) a += norm2(A.col(j)); return a;}
tpl T norm_F (const MatmnT &A) {return sqrt(norm2_F(A));}

template <int m1, int n1, int m2, int n2, typename T> Mat<m1,n1,T> project (const Mat<m2,n2,T> &A) {Mat<m1,n1,T> B; for (int j = 0; j < n1; j++) B.col(j) = (j<n2) ? project<m1>(A.col(j)) : Vec<m1,T>(0); return B;}

#undef tpl
#undef MatmnT
#undef MatnnT
#undef VecmT
#undef VecnT

typedef Mat<2,2> Mat2x2;
typedef Mat<3,3> Mat3x3;
typedef Mat<3,2> Mat3x2;
typedef Mat<2,3> Mat2x3;
typedef Mat<3,4> Mat3x4;

#endif
