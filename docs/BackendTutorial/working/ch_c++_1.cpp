// clang -c ch_c++_1.cpp -emit-llvm -o ch_c++_1.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch_c++_1.bc -o -

#include <iostream>
using namespace std;

extern "C" int printf(const char *format, ...);
extern "C" int sprintf(char *out, const char *format, ...);

class CPolygon {
  protected:
    int width, height;
  public:
    void set_values (int a, int b)
    { width=a; height=b; }
//    virtual int area (void) =0;
    virtual int area (void) { return 0;};
    void printarea (void)
//    { printf("%d\n", this->area()); }
    { cout << this->area() << endl; }
  };

class CRectangle: public CPolygon {
  public:
    int area (void)
    { return (width * height); }
};

class CTriangle: public CPolygon {
  public:
    int area (void)
    { return (width * height / 2); }
};

class CAngle1: public CPolygon {
  public:
    int area (void)
    { return (width * height / 4); }
};

int test_cpp_cout() {
  CPolygon * ppoly1 = new CRectangle;
  CPolygon * ppoly2 = new CTriangle;
//  CPolygon * appoly3 = new CXangle;
  CPolygon * appoly3 = new CAngle1;
  ppoly1->set_values (4,5);
  ppoly2->set_values (4,5);
//  ppoly3->set_values (4,5);
  ppoly1->printarea();
  ppoly2->printarea();
//  ppoly3->printarea();
  delete ppoly1;
  delete ppoly2;
//  delete ppoly3;
  return 0;
}

#ifdef TEST_RUN
int main() {
  return test_cpp_cout();
}
#endif

