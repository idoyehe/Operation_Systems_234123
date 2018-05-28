#include "Factory.h"
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <list>
#include <map>
#include "test_utilities.h"
#include <unistd.h>


bool test1() {
    Factory factory1;
    Product* products = new Product[10];
    for (int i = 0; i < 10; i++) {
        Product p(i, i);
        products[i] = p;
    }
    factory1.startProduction(10,products,100);
    factory1.finishProduction(100);

    std::list<Product> available=factory1.listAvailableProducts(); //{0,1,2,3,4,5,6,7,8,9}
    ASSERT_TEST(available.size()==10);

    factory1.startSimpleBuyer(200);
    sleep(1);
    factory1.startSimpleBuyer(201);
    ASSERT_TEST(factory1.finishSimpleBuyer(201) == 1);
    ASSERT_TEST(factory1.finishSimpleBuyer(200) == 0);
    available=factory1.listAvailableProducts(); //{2,3,4,5,6,7,8,9}
    ASSERT_TEST(available.size()==8);

    factory1.startCompanyBuyer(3,3,300);
    ASSERT_TEST(factory1.finishCompanyBuyer(300)==1);
    available=factory1.listAvailableProducts(); //{5,6,7,8,9,2}
    ASSERT_TEST(available.size()==6);

    factory1.startSimpleBuyer(202);
    ASSERT_TEST(factory1.finishSimpleBuyer(202)==5);
    available=factory1.listAvailableProducts(); //{6,7,8,9,2}
    ASSERT_TEST(available.size()==5);

    factory1.startThief(10,400);
    ASSERT_TEST(factory1.finishThief(400)==5);
    available=factory1.listAvailableProducts(); //{}
    ASSERT_TEST(available.empty());
    std::list<std::pair<Product, int> > stolen=factory1.listStolenProducts(); //{(6,400),(7,400),(8,400),(9,400),(2,400)}
    ASSERT_TEST(stolen.size()==5);

    factory1.startSimpleBuyer(203);
    ASSERT_TEST(factory1.finishSimpleBuyer(203)==-1);
    available=factory1.listAvailableProducts(); //{}
    ASSERT_TEST(available.empty());

    delete[] products;
    std::cout << "Test 1: SUCCESS" << std::endl;
    return true;
}

bool test2(){
    Factory factory1;
    Product* products = new Product[10];
    for (int i = 0; i < 10; i++) {
        Product p(i, i);
        products[i] = p;
    }
    factory1.startProduction(10,products,100);
    factory1.finishProduction(100);
    std::list<Product> available=factory1.listAvailableProducts(); //{0,1,2,3,4,5,6,7,8,9}
    ASSERT_TEST(available.size()==10);

    factory1.startCompanyBuyer(5,3,200);
    ASSERT_TEST(factory1.finishCompanyBuyer(200)==3);
    factory1.startSimpleBuyer(300);
    ASSERT_TEST(factory1.finishSimpleBuyer(300)==5);
    available=factory1.listAvailableProducts(); //{6,7,8,9,0,1,2}
    ASSERT_TEST(available.size()==7);

    factory1.startCompanyBuyer(10,0,201);
    sleep(1);
    factory1.startSimpleBuyer(301);
    ASSERT_TEST(factory1.finishSimpleBuyer(301)==6);

    int index=0;
    for (int i = 10; i < 20; i++) {
        Product p(i, i);
        products[index++] = p;
    }
    factory1.startProduction(10,products,101);
    factory1.finishProduction(101);
    available=factory1.listAvailableProducts(); //{14,15,16,17,18,19}
    ASSERT_TEST(factory1.finishCompanyBuyer(201)==0);
    available=factory1.listAvailableProducts(); //{14,15,16,17,18,19}
    ASSERT_TEST(available.size()==6);

    factory1.startSimpleBuyer(302);
    ASSERT_TEST(factory1.finishSimpleBuyer(302)==14);
    available=factory1.listAvailableProducts(); //{15,16,17,18,19}
    ASSERT_TEST(available.size()==5);

    delete[] products;
    std::cout << "Test 2: SUCCESS" << std::endl;
    return true;
}

bool test3(){
    Factory factory1;
    Product* products = new Product[10];
    for (int i = 0; i < 10; i++) {
        Product p(i, i);
        products[i] = p;
    }
    factory1.startProduction(10,products,100);
    factory1.finishProduction(100);
    std::list<Product> available=factory1.listAvailableProducts(); //{0,1,2,3,4,5,6,7,8,9}
    ASSERT_TEST(available.size()==10);

    factory1.closeFactory();
    factory1.startSimpleBuyer(200);
    ASSERT_TEST(factory1.finishSimpleBuyer(200)==-1);

    factory1.startCompanyBuyer(3,4,300);
    factory1.startThief(3,400);
    factory1.openFactory();
    ASSERT_TEST(factory1.finishThief(400)==3);
    ASSERT_TEST(factory1.finishCompanyBuyer(300)==1);
    available=factory1.listAvailableProducts(); //{6,7,8,9,3}
    ASSERT_TEST(available.size()==5);

    factory1.closeFactory();
    factory1.startCompanyBuyer(3,20,301);
    factory1.startThief(5,401);
    factory1.openFactory();
    ASSERT_TEST(factory1.finishThief(401)==5); //{}

    int index=0;
    for (int i = 20; i < 30; i++) {
        Product p(i, i);
        products[index++] = p;
    }
    factory1.startProduction(10,products,102);
    factory1.finishProduction(102);

    ASSERT_TEST(factory1.finishCompanyBuyer(301)==0);
    available=factory1.listAvailableProducts(); //{23,24,25,26,27,28,29}
    ASSERT_TEST(available.size()==7);

    std::list<std::pair<Product, int> > stolen=factory1.listStolenProducts();
    ASSERT_TEST(stolen.size()==8);
    std::list<std::pair<Product, int> >::iterator itr;
    int count=0;
    for(itr=stolen.begin();itr!=stolen.end() && count<3;itr++){
        count++;
        ASSERT_TEST((*itr).second==400);
    }
    for(;itr!=stolen.end();itr++){
        ASSERT_TEST((*itr).second==401);
    }

    delete[] products;
    std::cout << "Test 3: SUCCESS" << std::endl;
    return true;
}

bool test4(){
    Factory factory1;
    Product* products = new Product[10];
    for (int i = 0; i < 10; i++) {
        Product p(i, i);
        products[i] = p;
    }
    factory1.startProduction(10,products,100);
    factory1.finishProduction(100);
    std::list<Product> available=factory1.listAvailableProducts(); //{0,1,2,3,4,5,6,7,8,9}
    ASSERT_TEST(available.size()==10);

    factory1.closeReturningService();
    factory1.startCompanyBuyer(3,0,200); //{3,4,5,6,7,8,9}

    unsigned int microseconds=100000;
    usleep(microseconds);

    factory1.startSimpleBuyer(300);
    ASSERT_TEST(factory1.finishSimpleBuyer(300)==3); //{4,5,6,7,8,9}
    factory1.startSimpleBuyer(301);
    ASSERT_TEST(factory1.finishSimpleBuyer(301)==4); //{5,6,7,8,9}
    factory1.openReturningService();//{5,6,7,8,9}
    factory1.startSimpleBuyer(302);
    ASSERT_TEST(factory1.finishSimpleBuyer(302)==5); //{6,7,8,9}
    ASSERT_TEST(factory1.finishCompanyBuyer(200)==0);

    factory1.closeReturningService(); //{6,7,8,9}

    factory1.startCompanyBuyer(4,10,200); //{}
    usleep(microseconds);
    factory1.startSimpleBuyer(300);
    ASSERT_TEST(factory1.finishSimpleBuyer(300)==-1); //{}
    factory1.startSimpleBuyer(301);
    ASSERT_TEST(factory1.finishSimpleBuyer(301)==-1); //{}

    factory1.openReturningService();//{6,7,8,9}


    usleep(microseconds);

    factory1.startSimpleBuyer(302);
    ASSERT_TEST(factory1.finishSimpleBuyer(302)==6);
    ASSERT_TEST(factory1.finishCompanyBuyer(200)==4);

    available=factory1.listAvailableProducts(); //{7,8,9}
    ASSERT_TEST(available.size()==3);

    delete[] products;
    std::cout << "Test 4: SUCCESS" << std::endl;
    return true;
}

int main(){
    test1();
    test2();
    test3();
    test4();
    return 0;
}