#include <iostream>
#include <unistd.h>
#include "Factory.h"
#include "test_utilities.h"

bool simpleBuyerTest()
{
    Product a(1,1);
    Product b(2,2);
    Factory f ;
    Product pro[2] = {b,a};
    // Checking if the factory is empty and a simple buyer tries to buy
    f.startSimpleBuyer(1);
    ASSERT_TEST(f.finishSimpleBuyer(1) == -1);
    
    // Adding products to factory
    f.startProduction(2, pro, 1);
    f.finishProduction(1);
    std::list<Product> list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 2);
    Product c = list.front();
    ASSERT_TEST(c.getId() == 2);
    list.clear();
    
    // Adding simple buyers
    f.startSimpleBuyer(2);
    sleep(1);
    f.startSimpleBuyer(1);
    ASSERT_TEST(f.finishSimpleBuyer(1) == 1);
    ASSERT_TEST(f.finishSimpleBuyer(2) == 2);
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 0);
    
    // Adding another product
    f.startProduction(1, &a, 1);
    f.finishProduction(1);
    
    // Closing the factory
    f.closeFactory();
    f.startSimpleBuyer(3);
    ASSERT_TEST(f.finishSimpleBuyer(3) == -1);
    
    // Opening the factory
    f.openFactory();       // I had a bug here, forgot to unlock a lock if the
    //                          simple buyer could only take one of the locks
    
    // Again trying to buy
    f.startSimpleBuyer(4);
    ASSERT_TEST(f.finishSimpleBuyer(4) == 1);
    list.clear();
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 0);
    return true;
}

bool companyTest()
{
    Product a(1,1);
    Product b(2,2);
    Product c(3,3);
    Factory f;
    Product pro[2] = {b,a};
    std::list<Product> list;
    // Creating a new company
    f.startCompanyBuyer(2, 1, 1); // This company should wait
    f.startProduction(2, pro, 2); // The company should start buying
    ASSERT_TEST(f.finishCompanyBuyer(1) == 0); // I had a bug here, forgot to broadcast after
                              // making new products
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 0);
    // Closing the factory, adding a new company, adding new products and then opening
    f.closeFactory();
    f.startCompanyBuyer(2, 0, 3);
    f.startProduction(2, pro, 4);
    f.finishProduction(4);
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 2);
    list.clear();
    f.openFactory();
    ASSERT_TEST(f.finishCompanyBuyer(3) == 0);
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 0);
    // Adding a new company and a thief - synchronization test
    f.closeFactory();
    f.startProduction(2, pro, 4);
    f.startThief(1, 5);
    f.startCompanyBuyer(1, 0, 6);
    f.finishProduction(4);
    f.openFactory();
    ASSERT_TEST(f.finishCompanyBuyer(6) == 0);
    f.finishThief(5);
    std::list<std::pair<Product, int>> stolen = f.listStolenProducts();
    ASSERT_TEST(stolen.size() == 1);
    std::pair<Product, int> stolen_product = stolen.front();
    ASSERT_TEST(stolen_product.second == 5);
    ASSERT_TEST(stolen_product.first.getId() == 2);
    // Closing the returning service, adding a new company and produce products which has low value
    f.closeReturningService();
    f.produce(2, pro);
    f.startCompanyBuyer(2, 2, 7);
    sleep(1);
    list.clear();
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 0);
    f.openReturningService();
    f.finishCompanyBuyer(7);
    list.clear();
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 1);
    f.startSimpleBuyer(8);
    f.finishSimpleBuyer(8);
    return true;
}

bool thiefTest()
{
    Product a(1,1);
    Product b(2,2);
    Product c(3,3);
    Factory f;
    Product pro2[3] = {c,b,a};
    // Empty factory test
    f.startThief(100, 1);
    ASSERT_TEST(f.finishThief(1) == 0);
    // There are to many products in company
    f.startProduction(3, pro2, 2);
    f.finishProduction(2);
    f.startThief(1, 3);
    ASSERT_TEST(f.finishThief(3) == 1);
    // There isn't enoght products in the company
    f.startThief(5, 4);
    ASSERT_TEST(f.finishThief(4) == 2);
    // There is exactly the maximum products a thief can carry
    f.startProduction(1, &a, 5);
    f.finishProduction(5);
    f.startThief(1, 6);
    ASSERT_TEST(f.finishThief(6) == 1);
    return true;
}

bool mainThreadTest()
{
    Product a(1,1);
    Product b(2,2);
    Product c(3,3);
    Factory f;
    Product pro[3] = {c,b,a};
    f.produce(3, pro);
    ASSERT_TEST(f.tryBuyOne() == 3);
    std::list<Product> list = f.buyProducts(2);
    f.returnProducts(list, 0);
    return true;
}

bool smallSynchronizationTest()
{
    Product a(1,1);
    Product b(2,2);
    Product c(3,3);
    Product d(4,4);
    Product e(5,5);
    Product g(6,6);
    Product pro[6] = {a,b,c,d,e,g};
    Factory f;
    f.closeFactory();
    f.produce(6, pro);
    f.startCompanyBuyer(4, 10, 1);
    f.startCompanyBuyer(3, 10, 2);
    f.startThief(1, 3);
    f.startThief(1, 4);
    f.startCompanyBuyer(2, 10, 5);
    f.startCompanyBuyer(1, 10, 6);
    f.openFactory();
    f.finishThief(4);
    ASSERT_TEST(f.finishCompanyBuyer(1) == 4);
    ASSERT_TEST(f.finishCompanyBuyer(2) == 3);
    ASSERT_TEST(f.finishCompanyBuyer(5) == 2);
    ASSERT_TEST(f.finishCompanyBuyer(6) == 1);
    f.startCompanyBuyer(4, 0, 7);
    f.finishCompanyBuyer(7);
    return true;
}

bool bigSynchronizationTest()
{
    Product a(1,1);
    Factory f;
    f.closeFactory();
    for (int i = 0; i < 100; i++)
    {
        f.produce(1, &a);
    }
    for (int i = 0; i < 50; i++)
    {
        f.startCompanyBuyer(1, 10, i);
        f.startThief(1, i+50);
    }
    f.openFactory();
    for (int i = 0; i < 50; i++)
    {
        ASSERT_TEST(f.finishCompanyBuyer(i) == 1);
        ASSERT_TEST(f.finishThief(i+50) == 1);
    }
    for (int i = 0; i < 50; i++)
    {
        f.startSimpleBuyer(100+i);
    }
    for (int i = 0; i < 50; i++)
    {
        f.finishSimpleBuyer(100+i); // I can't really check the return value here,
                                     // so just making sure the test isn't crashing
    }
    return true;
}


int main() {
    RUN_TEST(simpleBuyerTest);
    RUN_TEST(companyTest);
    RUN_TEST(thiefTest);
    RUN_TEST(mainThreadTest);
    RUN_TEST(smallSynchronizationTest);
    RUN_TEST(bigSynchronizationTest);
    return 0;
}
