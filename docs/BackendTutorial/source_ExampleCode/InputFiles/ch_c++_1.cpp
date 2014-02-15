// clang -c ch_c++_1.cpp -emit-llvm -DTEST_RUN -o ch_c++_1.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llvm-dis ch_c++_1.bc -o -
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch_c++_1.bc -o -

//#include <iostream>
//using namespace std;

extern "C" int printf(const char *format, ...);
extern "C" int sprintf(char *out, const char *format, ...);

class CPolygon {	// _ZTVN10__cxxabiv117__class_type_infoE for parent class
  protected:
    int width, height;
  public:
    void set_values (int a, int b)
    { width=a; height=b; }
    virtual int area (void) =0; // __cxa_pure_virtual
//    virtual int area (void) { return 0;};
    void printarea (void)
    { printf("%d\n", this->area()); }
//    { cout << this->area() << endl; }
  };

class CRectangle: public CPolygon {	// _ZTVN10__cxxabiv120__si_class_type_infoE for derived class
  public:
    int area (void)
    { return (width * height); }
};

class CTriangle: public CPolygon {
  public:
    int area (void)
    { return (width * height / 2); }
};

class CAngle: public CPolygon {
  public:
    int area (void)
    { return (width * height / 4); }
};
#if 0
int test_cpp_polymorphism() {
  CPolygon * ppoly1 = new CRectangle;	// _Znwm
  CPolygon * ppoly2 = new CTriangle;
  ppoly1->set_values (4,5);
  ppoly2->set_values (4,5);
  ppoly1->printarea();
  ppoly2->printarea();
  delete ppoly1;	// _ZdlPv
  delete ppoly2;
  return 0;
}
#else
int test_cpp_polymorphism() {
  CRectangle poly1;
  CTriangle poly2;
  CAngle poly3;
  
  CPolygon * ppoly1 = &poly1;
  CPolygon * ppoly2 = &poly2;
  CPolygon * ppoly3 = &poly3;
  
  ppoly1->set_values (4,5);
  ppoly2->set_values (4,5);
  ppoly3->set_values (4,5);
  ppoly1->printarea();
  ppoly2->printarea();
  ppoly3->printarea();
  return 0;
}
#endif

#ifdef TEST_RUN
int main() {
  return test_cpp_polymorphism();
}
#endif
