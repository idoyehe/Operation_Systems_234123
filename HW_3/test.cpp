#include "Factory.h"
#include <unistd.h>


int main(){
    Factory factory = Factory();
    Product products[3];
    products[0] = Product(1,10);
    products[1] = Product(2,20);
    products[2] = Product(3,30);

    factory.startProduction(3,products,47);
    factory.startThief(4,48);
    sleep(60);

    return 0;
}