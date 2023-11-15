#include <map>
#include <iostream>


class A
{
public:

    int value;

    A () {
        std::cout << "contructor\n";
    }

    A (int a) : value(a) {};

    ~A() {
        std::cout << "Destructor\n";
    }

    
};


int main() {


    std::map<int, A> mapTest;

    if (true){
        mapTest[0] = A(5);

        std::cout << "test: |" << &mapTest[0] << std::endl;
    } 

        std::cout << "value: |" <<  mapTest[0].value << std::endl; return 0;



}