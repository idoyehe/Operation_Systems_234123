#include "Factory.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include "test_utilities.h"

using namespace std;


bool test_production_behavior() {
    Factory candy_factory;
    Product *product_batch1 = new Product[10];
    int batch = 1;
    for (int i = 0; i < 10; i++) {
        product_batch1[i] = Product(i * batch, 10 * i * batch);
    }

    candy_factory.startProduction(10, product_batch1, 100);//sanity check small production
    candy_factory.finishProduction(100);

    list<Product> export_available;
    list<pair<Product, int>> export_stolen;
    export_available = candy_factory.listAvailableProducts();
    ASSERT_TEST(export_available.size() == 10);
    //verify production
    for (int i = 0; i < 10; i++) {
        ASSERT_TEST(
                product_batch1[i].getId() == export_available.front().getId());
        ASSERT_TEST(product_batch1[i].getValue() ==
                    export_available.front().getValue());
        export_available.pop_front();
    }
    ASSERT_TEST(export_available.empty());


    Product *product_batch2 = new Product[1000000];
    batch = 2;
    for (int i = 0; i < 1000000; i++) {
        product_batch2[i] = Product(i * 100, 10 * i * batch);
    }

    candy_factory.startProduction(1000000, product_batch2, 101);//large production checking factory control

    candy_factory.closeFactory();
    printf("Factory CLOSED\n");
    candy_factory.closeFactory();
    candy_factory.closeReturningService();
    printf("Factory CLOSED RETURN SERVICE\n");
    candy_factory.closeReturningService();
    candy_factory.openFactory();
    printf("Factory OPEN\n");
    candy_factory.openFactory();
    candy_factory.openReturningService();
    printf("Factory OPEN RETURN SERVICE\n");
    candy_factory.openReturningService();
    candy_factory.finishProduction(101);

    export_available = candy_factory.listAvailableProducts();
    ASSERT_TEST(export_available.size() == 1000010);

    Product *product_batch3 = new Product[10000];
    batch = 3;
    for (int i = 0; i < 1000; i++) {
        product_batch2[i] = Product(i * 100, 10 * i * batch);
    }

    candy_factory.startProduction(1000000, product_batch2, 102);//large production checking factory control
    candy_factory.produce(10000,product_batch3);
    candy_factory.finishProduction(102);
    export_available = candy_factory.listAvailableProducts();
    ASSERT_TEST(export_available.size() == 2010010);


    delete[](product_batch1);
    delete[](product_batch2);
    delete[](product_batch3);
    return true;
}

bool test_thieves_behavior(){
    Factory candy_factory;
    Product *product_batch = new Product[10];
    int batch = 1;
    for (int i = 0; i < 10; i++) {
        product_batch[i] = Product(i + (batch *10),i);
    }

    candy_factory.startThief(10,100);
    ASSERT_TEST(candy_factory.finishThief(100) == 0);

    candy_factory.closeFactory();

    candy_factory.startThief(5,100);
    candy_factory.startThief(5,101);

    /*if you want to verify that thieves 100,101 cannot enter the factory when it is closed
     * remove the comment from next 2 lines and it SHOULD be deadlock because thieves
     * are waiting to main process to open factory and main process is waiting them to finish*/
//    candy_factory.finishThief(100);
//    candy_factory.finishThief(101);

    candy_factory.startProduction(10, product_batch,200);//production while factory is closed
    candy_factory.finishProduction(200);

    list<Product> export_available;
    list<pair<Product, int>> export_stolen;

    export_available = candy_factory.listAvailableProducts();
    ASSERT_TEST(export_available.size() == 10);
    //verify production
    for (int i = 0; i < 10; i++) {
        ASSERT_TEST(product_batch[i].getId() == export_available.front().getId());
        ASSERT_TEST(product_batch[i].getValue() == export_available.front().getValue());
        export_available.pop_front();
    }
    ASSERT_TEST(export_available.empty());

    candy_factory.openFactory();
    ASSERT_TEST(candy_factory.finishThief(101) == 5);
    ASSERT_TEST(candy_factory.finishThief(100) == 5);

    export_available = candy_factory.listAvailableProducts();
    ASSERT_TEST(export_available.empty());//thieves stole all
    export_stolen = candy_factory.listStolenProducts();
    ASSERT_TEST(export_stolen.size() == 10);//thieves stole all

    candy_factory.startThief(10,100);
    ASSERT_TEST(candy_factory.finishThief(100) == 0);

    candy_factory.closeReturningService();//check no effecting on thieves

    candy_factory.startProduction(10, product_batch,200);//production while factory is open
    candy_factory.finishProduction(200);

    candy_factory.startThief(20,100);
    ASSERT_TEST(candy_factory.finishThief(100) == 10);

    int stole0 = 0;
    int stole1 = 0;
    int stole2 = 0;
    for (int i = 0; i < 100 ; ++i) {
        candy_factory.startProduction(1,product_batch,100+i); //100 products
        candy_factory.startThief(2,400+i); //100 thieves want to steal 200 products
    }
    /*every thief has 3 options:
     * 1. arrive when two products available and stole them
     * 2. arrive when one product available and stole it
     * 3. arrive when NO products available and leave*/

    for (int i = 0; i < 100 ; ++i) {
        int num_of_stole = candy_factory.finishThief(400+i);
        stole0 += (num_of_stole == 0);
        stole1 += (num_of_stole == 1);
        stole2 += (num_of_stole == 2);
    }
    ASSERT_TEST(stole0 == stole2 && stole1 + (2*stole2) == 100 || stole0 != stole2 && stole1 + (2*stole2) < 100);

    delete[](product_batch);
    return true;
}



int main(){
    RUN_TEST(test_production_behavior);
    RUN_TEST(test_thieves_behavior);
//    RUN_TEST(test_comapnies_behavior);
//    RUN_TEST(test_single_buyer_behavior);
//    RUN_TEST(test_factory_behavior);
}