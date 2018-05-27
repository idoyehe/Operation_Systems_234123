#include "Factory.h"
#include "Product.h"
#include <assert.h>
#include <time.h>
#include <iostream>
#include <unistd.h>
#include "test_utilities.h"
#define N 75

void passTime(int t) {
    time_t t_start_1,t_end_1;
    time(&t_start_1);
    time(&t_end_1);
    while(t_end_1 - t_start_1 < t) {
        time(&t_end_1);
    }
}

void test1() {
    Factory factory1;
    Product* products1 = new Product[10];
    for (int i = 0; i < 10 ; ++i) {
        products1[i] = Product(i,i);
    }

    for (int i = 0; i < N; ++i) {
        factory1.startCompanyBuyer(3,0,300+i); //100 companies with 300 products
    }
    for (int i = 0; i < N ; ++i) {
        factory1.startProduction(1,products1,100+i); //100 products
        factory1.startThief(2,400+i); //100 thieves with 200 products
    }

    for (int i = 0; i < N ; ++i) {
        int num_of_stole = factory1.finishThief(400+i);
        assert(num_of_stole == 1 || num_of_stole == 2 || num_of_stole == 0);
    }

    for (int i = 0; i < N ; ++i) {
        factory1.startProduction(3,products1,100+i);
    }

    for (int i = 0; i < N ; ++i) {
        assert(factory1.finishCompanyBuyer(300+i)==0);
    }
    delete[](products1);
}

void test2() {
    Factory factory1;
    Product* products1 = new Product[40];

    for (int i = 0; i < 40 ; ++i) {
        Product p(i+1,i+1);
        products1[i] = p;
    }

    factory1.startProduction(10,products1,100);
    factory1.finishProduction(100); // {1,2,...,10}
    assert(factory1.listAvailableProducts().size()==10);

    factory1.startThief(3,400);
    assert(factory1.finishThief(400)==3); //{4,5,...,10}
    assert(factory1.listAvailableProducts().size()==7);
    std::list<std::pair<Product, int> > stolen = factory1.listStolenProducts();
    std::list<std::pair<Product, int> >::iterator st_it = stolen.begin();
    assert(stolen.size()==3);

    factory1.startSimpleBuyer(200);
    assert(factory1.finishSimpleBuyer(200)==4); //{5,6,...,10}
    assert(factory1.listAvailableProducts().size()==6);

    factory1.startCompanyBuyer(2,5,300);
    assert(factory1.finishCompanyBuyer(300)==0);//{7,8,...,10}
    assert(factory1.listAvailableProducts().size()==4);

    factory1.startCompanyBuyer(2,8,300);
    assert(factory1.finishCompanyBuyer(300)==1); //{9,10,7}
    assert(factory1.listAvailableProducts().size()==3);
    std::list<Product> avail = factory1.listAvailableProducts();
    std::list<Product>::iterator it = avail.begin();
    assert((*it++).getId()==9);
    assert((*it++).getId()==10);
    assert((*it).getId()==7);

    factory1.closeFactory();

    factory1.startCompanyBuyer(3,0,300);
    factory1.startCompanyBuyer(3,0,301);
    factory1.startCompanyBuyer(3,0,302);
    factory1.startCompanyBuyer(3,0,303);
    factory1.startCompanyBuyer(3,0,304);
    factory1.startThief(1,401);

    factory1.openFactory();

    assert(factory1.finishThief(401)==1); //{10,7}
    avail = factory1.listAvailableProducts();
    assert(avail.size()==2);
    it = avail.begin();
    assert((*it++).getId()==10);
    assert((*it).getId()==7);

	usleep(100000);//Uri added
    factory1.startSimpleBuyer(200); //{7}
    usleep(100000);
    factory1.startSimpleBuyer(201); //{10}
	assert(factory1.finishSimpleBuyer(200)==10);
    assert(factory1.finishSimpleBuyer(201)==7);

    factory1.startProduction(15,products1,100);
    factory1.finishProduction(100);
    factory1.finishCompanyBuyer(300);
    factory1.finishCompanyBuyer(301);
    factory1.finishCompanyBuyer(302);
    factory1.finishCompanyBuyer(303);
    factory1.finishCompanyBuyer(304);
    assert(factory1.listAvailableProducts().size()==0);
    stolen = factory1.listStolenProducts();
    assert(stolen.size()==4);
    st_it = stolen.begin();
    ++++++st_it;
    assert((*(st_it)).first.getId()==9); //{}

    factory1.closeFactory();

    factory1.startCompanyBuyer(3,0,300);
    factory1.startCompanyBuyer(3,0,301);
    factory1.startCompanyBuyer(3,0,302);
    factory1.startCompanyBuyer(3,0,303);
    factory1.startCompanyBuyer(3,0,304);
    factory1.startCompanyBuyer(3,0,305);
    factory1.startCompanyBuyer(3,0,306);
    factory1.startCompanyBuyer(3,0,307);
    factory1.startCompanyBuyer(3,0,308);
    factory1.startCompanyBuyer(3,0,309);
    factory1.startThief(1,402);

    factory1.openFactory();
	usleep(300000);//Uri added
    factory1.startProduction(31,products1,100);
    factory1.finishProduction(100);
    for (int i = 0; i < 10 ; ++i) {
        assert(factory1.finishCompanyBuyer(300+i)==0);
    }
    int res = factory1.finishThief(402);
    assert(res==0);
    assert(factory1.listAvailableProducts().size()==1); //{31}

    factory1.startThief(3,403);
    factory1.finishThief(403);
    stolen = factory1.listStolenProducts();
    assert(stolen.size()==5);
    st_it = stolen.begin();
    ++++++++st_it;
    assert((*st_it).first.getId()==31);
    assert((*st_it).second==403); //{}

    factory1.closeFactory();

    factory1.startProduction(31,products1,100);
    factory1.finishProduction(100);

    factory1.startCompanyBuyer(3,0,300);
    factory1.startCompanyBuyer(3,0,301);
    factory1.startCompanyBuyer(3,0,302);
    factory1.startCompanyBuyer(3,0,303);
    factory1.startCompanyBuyer(3,0,304);
    factory1.startCompanyBuyer(3,0,305);
    factory1.startCompanyBuyer(3,0,306);
    factory1.startCompanyBuyer(3,0,307);
    factory1.startCompanyBuyer(3,0,308);
    factory1.startCompanyBuyer(3,0,309);
    factory1.startThief(1,404);

    factory1.openFactory();

    assert(factory1.finishThief(404)==1);
    for (int i = 0; i < 10 ; ++i) {
        assert(factory1.finishCompanyBuyer(300+i)==0);
    }
    assert(factory1.listAvailableProducts().size()==0); //{}
    stolen = factory1.listStolenProducts();
    assert(stolen.size()==6);
    st_it = stolen.begin();
    ++++++++++st_it;
    assert((*st_it).first.getId()==1);
    assert((*st_it).second==404); //{}

    factory1.startThief(10,400);
    factory1.closeReturningService();
    assert(factory1.finishThief(400)==0);

    factory1.startProduction(32,products1,100);
    factory1.finishProduction(100);

    factory1.startCompanyBuyer(3,100,300);
    factory1.startCompanyBuyer(3,100,301);
    factory1.startCompanyBuyer(3,100,302);
    factory1.startCompanyBuyer(3,100,303);
    factory1.startThief(1,406);
    factory1.startCompanyBuyer(3,100,304);
    factory1.startCompanyBuyer(3,100,305);
    factory1.startCompanyBuyer(3,100,306);
    factory1.startCompanyBuyer(3,100,307);
    factory1.startCompanyBuyer(3,100,308);
    factory1.startCompanyBuyer(3,100,309);
    factory1.startThief(1,405);
    passTime(2);
    assert(factory1.listAvailableProducts().size()==0);

    factory1.openReturningService();

    for (int i = 0; i < 10 ; ++i) {
        assert(factory1.finishCompanyBuyer(300+i)==3);
    }

    assert(factory1.listAvailableProducts().size()==30);
    delete[](products1);
}

int main() {
    test1();
    printf("TEST No 1 SUCCESS\n");
    test2();
    printf("TEST No 2 SUCCESS\n");
    return 0;
}

