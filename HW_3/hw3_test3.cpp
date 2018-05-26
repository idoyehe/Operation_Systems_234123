#include "Factory.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <pthread.h>
#include <list>
#include <map>
#include <unistd.h>


bool TestProduction() {
    Factory factory;

    Product* products = new Product[10];
    for (int i = 0; i < 10; i++) {
        Product p(i, i);
        products[i] = p;
    }

    factory.startProduction(10,products,1); //Starting Production with: 10 products and ID: 1

    factory.finishProduction(1); //Finish Production with ID: 1

    std::list<Product> available=factory.listAvailableProducts(); // {0,1,2,3,4,5,6,7,8,9}
    assert(available.size() == 10);
    std::list<Product>::iterator itr;
    int index = 0;
    for(itr=available.begin();itr!=available.end();itr++ , index++){
        Product p = *itr;
        assert(p.getId() == index);
        assert(p.getValue() == index);
    }

    delete[](products);

    //There Are 10 Available Products

    factory.closeFactory(); //Closing the Factory

    products = new Product[5];
    for (int i = 0; i < 5; i++) {
        Product p(i+10, i+10);
        products[i] = p;
    }

    factory.startProduction(5,products,1); //Starting Production with: 5 products and ID: 1
    factory.finishProduction(1); //Finish Production with ID: 1

    available=factory.listAvailableProducts(); // {0,1,2,3,4,5,6,7,8,9}
    assert(available.size() == 15);
    index = 0;
    for(itr=available.begin();itr!=available.end();itr++ , index++){
        Product p = *itr;
        assert(p.getId() == index);
        assert(p.getValue() == index);
    }

    delete[](products);

    //There Are 15 Available Products and the Factory is Closed

    factory.openFactory(); //Opening Factory

    products = new Product[5];
    for (int i = 0; i < 5; i++) {
        Product p(i+15, i+15);
        products[i] = p;
    }
    factory.startProduction(5,products,1); //Starting Production with: 5 products and ID: 1
    factory.finishProduction(1); //Finish Production with ID: 1

    available = factory.listAvailableProducts();
    assert(available.size() == 20);
    index = 0;
    for(itr=available.begin();itr!=available.end();itr++ , index++){
        Product p = *itr;
        assert(p.getId() == index);
        assert(p.getValue() == index);
    }

    delete[](products);

    std::cout << "Test Production: SUCCESS" << std::endl;
    return true;
}

bool TestSimpleBuyer() {
    Factory factory;
    std::list<Product> available;
    std::list<Product>::iterator itr;
    int index = 0;

    factory.startSimpleBuyer(100); //Buyer with ID: 100
    assert(factory.finishSimpleBuyer(100) == -1); //There Aren't products to buy

    Product* products = new Product[10];
    for (int i = 0; i < 10; i++) {
        Product p(i, i);
        products[i] = p;
    }
    factory.startProduction(10,products,1); //Starting Production with: 10 products and ID: 1
    factory.finishProduction(1); //Finish Production with ID: 1

    factory.startSimpleBuyer(100); //Buyer with ID: 100
    assert(factory.finishSimpleBuyer(100) == 0); //Bought product with ID: 0

    available = factory.listAvailableProducts(); // {1,2,3,4,5,6,7,8,9}
    assert(available.size() == 9);
    index = 1;
    for(itr=available.begin();itr!=available.end();itr++ , index++){
        Product p = *itr;
        assert(p.getId() == index);
        assert(p.getValue() == index);
    }

    delete[](products);

    //There Are 9 Available Products

    factory.closeFactory(); //Closing the Factory

    factory.startSimpleBuyer(100); //Buyer with ID: 100
    assert(factory.finishSimpleBuyer(100) == -1); //Cant buy because the Factory is Closed

    factory.startSimpleBuyer(100); //Buyer with ID: 100
    unsigned int microseconds=100000;
    usleep(microseconds);
    factory.openFactory(); //Opening the Factory
    assert(factory.finishSimpleBuyer(100) == -1); //Cant buy because the Factory was Closed


    std::cout << "Test Simple_Buyer: SUCCESS" << std::endl;
    return true;
}

bool TestCompany() {
    Factory factory;
    std::list<Product> available;
    std::list<Product>::iterator itr;
    int index = 0;

    Product* products = new Product[10];
    for (int i = 0; i < 10; i++) {
        Product p(i, i);
        products[i] = p;
    }
    factory.startProduction(10,products,1); //Starting Production with: 10 products and ID: 1
    factory.finishProduction(1); //Finish Production with ID: 1

    delete[](products);

    factory.startCompanyBuyer(1,0,100); //Company with ID: 100 Buy: 1 Products with Min_Value: 0
    assert(factory.finishCompanyBuyer(100) == 0); //The Company Succeed to buy all Products

    available = factory.listAvailableProducts(); // {1,2,3,4,5,6,7,8,9}
    assert(available.size() == 9);
    index = 1;
    for(itr=available.begin();itr!=available.end();itr++ , index++){
        Product p = *itr;
        assert(p.getId() == index);
        assert(p.getValue() == index);
    }

    //There Are 9 Available Products

    factory.startCompanyBuyer(9,100,100); //Company with ID: 100 Buy: 9 Products with Min_Value: 100
    assert(factory.finishCompanyBuyer(100) == 9); //The Company Returned 9 Products

    available = factory.listAvailableProducts(); // {1,2,3,4,5,6,7,8,9}
    assert(available.size() == 9);
    index = 1;
    for(itr=available.begin();itr!=available.end();itr++ , index++){
        Product p = *itr;
        assert(p.getId() == index);
        assert(p.getValue() == index);
    }

    //There Are 9 Available Products: {1,2,3,4,5,6,7,8,9}

    factory.startCompanyBuyer(10,8,100); //Company with ID: 100 Buy: 10 Products with Min_Value: 8

    products = new Product[1];
    for (int i = 0; i < 1; i++) {
        Product p(i+10, i+10);
        products[i] = p;
    }
    factory.startProduction(1,products,1); //Starting Production with: 1 products and ID: 1
    factory.finishProduction(1); //Finish Production with ID: 1

    delete[](products);

    assert(factory.finishCompanyBuyer(100) == 7); //The Company Returned 7 Products

    available = factory.listAvailableProducts(); // {1,2,3,4,5,6,7}
    assert(available.size() == 7);
    index = 1;
    for(itr=available.begin();itr!=available.end();itr++ , index++){
        Product p = *itr;
        assert(p.getId() == index);
        assert(p.getValue() == index);
    }

    //There Are 7 Available Products {1,2,3,4,5,6,7}

    factory.closeFactory(); //Closing the Factory
    unsigned int microseconds=100000;
    usleep(microseconds);

    factory.startCompanyBuyer(8,10,100); //Company with ID: 100 Buy: 2 Products with Min_Value: 10

    available = factory.listAvailableProducts(); // {1,2,3,4,5,6,7}
    assert(available.size() == 7);

    products = new Product[1];
    for (int i = 0; i < 1; i++) {
        Product p(i+10, 10);
        products[i] = p;
    }
    factory.startProduction(1,products,1); //Starting Production with: 1 products and ID: 1
    factory.finishProduction(1); //Finish Production with ID: 1

    //Just to Verify the List is Correct
    available = factory.listAvailableProducts(); // {1,2,3,4,5,6,7,10}
    assert(available.size() == 8);
    index = 1;
    for(itr=available.begin();itr!=available.end();itr++ , index++){
        Product p = *itr;
        if(index == 8) {
            index = 10;
        }
        assert(p.getId() == index);
        assert(p.getValue() == index);
    }

    delete[](products);

    factory.openFactory(); //Opening the Factory
    microseconds=100000;
    usleep(microseconds);

    assert(factory.finishCompanyBuyer(100) == 7); //The Company Succeed to buy 1 Product and Return 1

    available = factory.listAvailableProducts(); // {1,2,3,4,5,6,7}
    assert(available.size() == 7);
    index = 1;
    for(itr=available.begin();itr!=available.end();itr++ , index++){
        Product p = *itr;
        assert(p.getId() == index);
        assert(p.getValue() == index);
    }

    std::cout << "Test Company: SUCCESS" << std::endl;
    return true;
}

int main(){
    TestProduction();
    TestSimpleBuyer();
    TestCompany();
    return 0;
}