#include "Factory.h"
#include <unistd.h>


int main(){
    Factory factory = Factory();
    Product products[3];
    products[0] = Product(1,10);
    products[1] = Product(2,20);
    products[2] = Product(3,30);

    factory.startProduction(3,products,47);
    sleep(3);
    factory.finishProduction(47);
    sleep(3);
    factory.startThief(2,48);
    sleep(3);
    factory.finishThief(48);

    sleep(10);

    return 0;
}