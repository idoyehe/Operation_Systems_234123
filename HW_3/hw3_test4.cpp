//
// Created by USER1 on 22-May-18.
// I need the main for testing
//

#include <iostream>
#include "Factory.h"
#include "test_utilities.h"
#include <unistd.h>

static void printProductList(std::list<Product> l){
    if(l.size() == 0){
        std::cout << "product list is empty\n";
    }else{
        std::list<Product>::iterator it;
        for(it = l.begin(); it != l.end(); ++it)
            std::cout << "id: " << (*it).getId()<< "\tvalue: " << (*it).getValue() << '\n';
    }
}

/*
 * Tests synchronously the following functions:
 * listAvailableProducts(), startProduction(), finishProduction()
 */
bool basicProduceTest(){
    Factory f;
    std::list<Product> list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 0); //No products yet
    Product p (234123, 10);
    unsigned int THREAD_ID = 1;
    f.startProduction(1, &p, THREAD_ID);
    f.finishProduction(THREAD_ID);
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 1);
    ASSERT_TEST(list.begin()->getId() == 234123 && list.begin()->getValue() == 10);

    //Now checking that the returned list is a copy - modifying it should not alter the factory
    list.clear();
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 1);

    //Same element twice
    f.startProduction(1, &p, THREAD_ID);
    f.finishProduction(THREAD_ID);
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 2);
    ASSERT_TEST(list.begin()->getId() == 234123 && list.begin()->getValue() == 10);
    ASSERT_TEST((list.begin()++)->getId() == 234123 && (list.begin()++)->getValue() == 10);

    //Now a different element
    Product p2 (234124, 55);
    f.startProduction(1, &p2, THREAD_ID);
    f.finishProduction(THREAD_ID);
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 3);
    std::list<Product>::iterator it = list.begin();
    ASSERT_TEST(it->getId() == 234123 && it->getValue() == 10);
    it++;
    ASSERT_TEST(it->getId() == 234123 && it->getValue() == 10);
    it++;
    ASSERT_TEST(it->getId() == 234124 && it->getValue() == 55);


    return true;
}

/*
 * Tests synchronously the following functions:
 * startSimpleBuyer(), finishSimpleBuyer()
 */
bool basicBuyerTest(){
    //Empty factory, buying should return -1
    Factory f;
    unsigned int THREAD_ID_ZERO = 0; //checks this corner case
    f.startSimpleBuyer(THREAD_ID_ZERO);
    ASSERT_TEST(f.finishSimpleBuyer(THREAD_ID_ZERO) == -1); //No products in factory yet

    //Now we will add two products
    Product p1 (1, 10);
    Product p2 (2, 20);
    unsigned int THREAD_ID = 111111;
    f.startProduction(1, &p1, THREAD_ID);
    f.finishProduction(THREAD_ID);
    f.startProduction(1, &p2, THREAD_ID);
    f.finishProduction(THREAD_ID);

    //now buyer should buy first product successfully
    f.startSimpleBuyer(THREAD_ID_ZERO);
    ASSERT_TEST(f.finishSimpleBuyer(THREAD_ID_ZERO) == 1); //First product
    f.startSimpleBuyer(THREAD_ID);
    ASSERT_TEST(f.finishSimpleBuyer(THREAD_ID) == 2); //Second product
    f.startSimpleBuyer(THREAD_ID);
    ASSERT_TEST(f.finishSimpleBuyer(THREAD_ID) == -1); //No products

    return true;
}

/*
 * Tests the following functions synchronously:
 * startCompanyBuyer(), finishCompanyBuyer()
 */
bool basicCompanyTest(){
    //1st stage - fill with products
    Factory f;
    unsigned int THREAD_ID = 777;
    Product p_array[] =
            {Product(11,100), Product(12, 200), Product(20, 5), Product(300, 0), Product(400, -1)};
    f.startProduction(5, p_array, THREAD_ID);
    f.finishProduction(THREAD_ID);

    //Now let's check company
    f.startCompanyBuyer(5, 100, THREAD_ID);
    //Expected - bought 2 items, returned 3
    ASSERT_TEST(f.finishCompanyBuyer(THREAD_ID) == 3);
    std::list<Product> list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 3);
    std::list<Product>::iterator it = list.begin();
    ASSERT_TEST(it->getId() == 20 && it->getValue() == 5);
    it++;
    ASSERT_TEST(it->getId() == 300 && it->getValue() == 0);
    it++;
    ASSERT_TEST(it->getId() == 400 && it->getValue() == -1);

    //Now we have (20,5), (300, 0), (400, -1)
    //We test buying a single product successfully, returning nothing
    f.startCompanyBuyer(1, 4, THREAD_ID);
    ASSERT_TEST(f.finishCompanyBuyer(THREAD_ID) == 0);
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 2);
    it = list.begin();
    ASSERT_TEST(it->getId() == 300 && it->getValue() == 0);
    it++;
    ASSERT_TEST(it->getId() == 400 && it->getValue() == -1);

    //Now we have (300, 0), (400, -1)
    //Test buying successfully 0 products, returning everything
    f.startCompanyBuyer(2, 100, THREAD_ID);
    ASSERT_TEST(f.finishCompanyBuyer(THREAD_ID) == 2);
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 2);
    it = list.begin();
    ASSERT_TEST(it->getId() == 300 && it->getValue() == 0);
    it++;
    ASSERT_TEST(it->getId() == 400 && it->getValue() == -1);

    //Now we have (300, 0), (400, -1)
    //Test buying one (less than the amount of all items with rice higher than minimum)
    f.startCompanyBuyer(1, -100, THREAD_ID);
    ASSERT_TEST(f.finishCompanyBuyer(THREAD_ID) == 0);
    list = f.listAvailableProducts();
    ASSERT_TEST(list.size() == 1);
    it = list.begin();
    ASSERT_TEST(it->getId() == 400 && it->getValue() == -1);

    return true;
}

/*
 * Tests the following functions synchronously:
 * listStolenProducts, startThief, finishThief
 */
bool basicThiefTest(){
    Factory f;
    //produce 5 products
    unsigned int THREAD_ID = 99;
    Product p_array[] =
            {Product(11,100), Product(12, 200), Product(20, 5), Product(300, 0), Product(400, -1)};
    f.startProduction(5, p_array, THREAD_ID);
    f.finishProduction(THREAD_ID);

    //Test 1 - try to steal less products than there are in the factory
    f.startThief(3, THREAD_ID);
    ASSERT_TEST(f.finishThief(THREAD_ID) == 3);
    std::list<std::pair<Product, int>> loot = f.listStolenProducts();
    ASSERT_TEST(loot.size() == 3);
    std::list<std::pair<Product, int>>::iterator pair_it = loot.begin();
    ASSERT_TEST(pair_it->first.getId() == 11 && pair_it->first.getValue() == 100);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 12 && pair_it->first.getValue() == 200);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 20 && pair_it->first.getValue() == 5);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    //We check that the correct 2 are left inside the factory
    std::list<Product> fabricated = f.listAvailableProducts();
    ASSERT_TEST(fabricated.size() == 2);
    std::list<Product>::iterator prod_it = fabricated.begin();
    ASSERT_TEST(prod_it->getId() == 300 && prod_it->getValue() == 0);
    prod_it++;
    ASSERT_TEST(prod_it->getId() == 400 && prod_it->getValue() == -1);

    //Test 2 - try to steal more products than there are in the factory
    f.startThief(3, THREAD_ID);
    ASSERT_TEST(f.finishThief(THREAD_ID) == 2);
    loot = f.listStolenProducts();
    ASSERT_TEST(loot.size() == 5); //5 TOTAL products were stolen
    pair_it = loot.begin();
    ASSERT_TEST(pair_it->first.getId() == 11 && pair_it->first.getValue() == 100);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 12 && pair_it->first.getValue() == 200);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 20 && pair_it->first.getValue() == 5);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 300 && pair_it->first.getValue() == 0);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 400 && pair_it->first.getValue() == -1);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    fabricated = f.listAvailableProducts();
    ASSERT_TEST(fabricated.size() == 0);

    //Test 3 - Try to steal from an empty factory. Nothing happens
    f.startThief(3, THREAD_ID);
    ASSERT_TEST(f.finishThief(THREAD_ID) == 0);
    loot = f.listStolenProducts();
    ASSERT_TEST(loot.size() == 5); //5 TOTAL products were stolen
    pair_it = loot.begin();
    ASSERT_TEST(pair_it->first.getId() == 11 && pair_it->first.getValue() == 100);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 12 && pair_it->first.getValue() == 200);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 20 && pair_it->first.getValue() == 5);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 300 && pair_it->first.getValue() == 0);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 400 && pair_it->first.getValue() == -1);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    fabricated = f.listAvailableProducts();
    ASSERT_TEST(fabricated.size() == 0);

    //Test 4 - Try to steal exactly the number of products that there is in the factory
    Product p_array2[] =
            {Product(77, 777)};
    f.startProduction(1, p_array2, THREAD_ID);
    f.finishProduction(THREAD_ID);
    f.startThief(1, THREAD_ID);
    ASSERT_TEST(f.finishThief(THREAD_ID) == 1);
    loot = f.listStolenProducts();
    ASSERT_TEST(loot.size() == 6); //7 TOTAL products were stolen
    pair_it = loot.begin();
    ASSERT_TEST(pair_it->first.getId() == 11 && pair_it->first.getValue() == 100);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 12 && pair_it->first.getValue() == 200);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 20 && pair_it->first.getValue() == 5);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 300 && pair_it->first.getValue() == 0);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 400 && pair_it->first.getValue() == -1);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    pair_it++;
    ASSERT_TEST(pair_it->first.getId() == 77 && pair_it->first.getValue() == 777);
    ASSERT_TEST(pair_it->second == THREAD_ID);
    fabricated = f.listAvailableProducts();
    ASSERT_TEST(fabricated.size() == 0);

    return true;
}

/*
 * Tests that the following functions behave well when are they are not called from within their tests:
 * (Note: no threads at all exist in the system)
 * produce, tryBuyOne, buyProducts, returnProducts, stealProducts
 */
bool threadlessTest(){
    Factory f;
    ASSERT_TEST(f.tryBuyOne() == -1); //No products in f yet
    Product p_array[] =
            {Product(11,100), Product(12, 200), Product(20, 5), Product(300, 0), Product(400, -1)};
    f.produce(5, p_array);

    ASSERT_TEST(f.tryBuyOne() == 11);//First product bought
    std::list<Product> fabricated = f.listAvailableProducts();
    ASSERT_TEST(fabricated.size() == 4);

    std::list<Product> acquired = f.buyProducts(2);
    ASSERT_TEST(acquired.size() == 2);
    std::list<Product>::iterator acquired_it = acquired.begin();
    ASSERT_TEST(acquired_it->getId() == 12 && acquired_it->getValue() == 200);
    acquired_it++;
    ASSERT_TEST(acquired_it->getId() == 20 && acquired_it->getValue() == 5);
    fabricated = f.listAvailableProducts();
    ASSERT_TEST(fabricated.size() == 2);

    //Return all bought products
    f.returnProducts(acquired, 0);
    fabricated = f.listAvailableProducts();
    ASSERT_TEST(fabricated.size() == 4);
    std::list<Product>::iterator fab_it = fabricated.begin();
    ASSERT_TEST(fab_it->getId() == 300 && fab_it->getValue() == 0);
    fab_it++;
    ASSERT_TEST(fab_it->getId() == 400 && fab_it->getValue() == -1);
    fab_it++;
    ASSERT_TEST(fab_it->getId() == 12 && fab_it->getValue() == 200);
    fab_it++;
    ASSERT_TEST(fab_it->getId() == 20 && fab_it->getValue() == 5);

    //First, sanity check - no products have been stolen yet
    std::list<std::pair<Product, int>> loot = f.listStolenProducts();
    ASSERT_TEST(loot.size() == 0);

    //Test stealing less products than there are in the factory
    ASSERT_TEST(f.stealProducts(1, 606) == 1);
    fabricated = f.listAvailableProducts();
    ASSERT_TEST(fabricated.size() == 3);
    fab_it = fabricated.begin();
    ASSERT_TEST(fab_it->getId() == 400 && fab_it->getValue() == -1);
    fab_it++;
    ASSERT_TEST(fab_it->getId() == 12 && fab_it->getValue() == 200);
    fab_it++;
    ASSERT_TEST(fab_it->getId() == 20 && fab_it->getValue() == 5);
    loot = f.listStolenProducts();
    ASSERT_TEST(loot.size() == 1);
    std::list<std::pair<Product, int>>::iterator loot_it = loot.begin();
    ASSERT_TEST(loot_it->first.getId() == 300 && loot_it->first.getValue() == 0);
    ASSERT_TEST(loot_it->second == 606);

    //Test stealing more products than there are in the factory
    ASSERT_TEST(f.stealProducts(5555, 505) == 3);
    fabricated = f.listAvailableProducts();
    ASSERT_TEST(fabricated.size() == 0);
    loot = f.listStolenProducts();
    ASSERT_TEST(loot.size() == 4);
    loot_it = loot.begin();
    ASSERT_TEST(loot_it->first.getId() == 300 && loot_it->first.getValue() == 0);
    ASSERT_TEST(loot_it->second == 606);
    loot_it++;
    ASSERT_TEST(loot_it->first.getId() == 400 && loot_it->first.getValue() == -1);
    ASSERT_TEST(loot_it->second == 505);
    loot_it++;
    ASSERT_TEST(loot_it->first.getId() == 12 && loot_it->first.getValue() == 200);
    ASSERT_TEST(loot_it->second == 505);
    loot_it++;
    ASSERT_TEST(loot_it->first.getId() == 20 && loot_it->first.getValue() == 5);
    ASSERT_TEST(loot_it->second == 505);
    loot_it++;

    //Test stealing from an empty factory
    ASSERT_TEST(f.stealProducts(22, 9099) == 0);
    fabricated = f.listAvailableProducts();
    ASSERT_TEST(fabricated.size() == 0);

    //for fun
    f.closeFactory();
    f.closeFactory();
    f.openFactory();
    f.openFactory();
    f.closeFactory();
    f.openFactory();
    f.openReturningService();
    f.openFactory();
    f.closeFactory();
    f.closeReturningService();
    f.closeReturningService();
    f.openReturningService();

    return true;
}

bool testAccessOrder(){
    //Test 1 - a little of everything
    Factory f;
    f.closeFactory();
    Product p_array[] =
            {Product(1,100), Product(2, 200), Product(3, 300), Product(4, 400), Product(5, 500)};
    unsigned int prod_id = 12, thief_id = 66, comp_id = 55, buyer_id = 1;
    f.startProduction(5, p_array, prod_id);
    f.startThief(2, thief_id);
    f.startCompanyBuyer(3, 400, comp_id);
    f.finishProduction(prod_id);

    f.openFactory();

    ASSERT_TEST(f.finishThief(thief_id) == 2);
    int compBuyerRes = f.finishCompanyBuyer(comp_id);
    ASSERT_TEST(compBuyerRes == 1);
    //Now hopefully everything is correct
    std::list<Product> p_list = f.listAvailableProducts();
    ASSERT_TEST(p_list.size() == 1);
    ASSERT_TEST(p_list.front().getId() == 3 && p_list.front().getValue() == 300);
    std::list<std::pair<Product, int>> s_list = f.listStolenProducts();
    ASSERT_TEST(s_list.size() == 2);
    std::list<std::pair<Product, int>>::iterator s_list_it = s_list.begin();
    ASSERT_TEST(s_list_it->first.getId() == 1 && s_list_it->first.getValue() == 100);
    ASSERT_TEST(s_list_it->second == thief_id);
    s_list_it++;
    ASSERT_TEST(s_list_it->first.getId() == 2 && s_list_it->first.getValue() == 200);
    ASSERT_TEST(s_list_it->second == thief_id);

    //Now multiple thieves, check that they don't interfere with each other. We cannot know the exact ordering
    //of the thieves. Also multiple companies buy, make sure they don't interfere.
    f.closeFactory();
    f.startProduction(5, p_array, prod_id);
    f.startThief(1, thief_id);
    f.startThief(1, thief_id+1);
    f.startThief(1, thief_id+2);
    f.startThief(1, thief_id+3);
    f.startThief(1, thief_id+4);
    f.startThief(1, thief_id+5);
    s_list = f.listStolenProducts();
    ASSERT_TEST(s_list.size() == 2); //Didn't increase because factory is closed
    f.finishProduction(prod_id);
    p_list = f.listAvailableProducts();
    ASSERT_TEST(p_list.size() == 6); //No products stolen yet because factory is closed. 1 was here before + 5
    f.openFactory();
    ASSERT_TEST(f.finishThief(thief_id) == 1);
    ASSERT_TEST(f.finishThief(thief_id+1) == 1);
    ASSERT_TEST(f.finishThief(thief_id+2) == 1);
    ASSERT_TEST(f.finishThief(thief_id+3) == 1);
    ASSERT_TEST(f.finishThief(thief_id+4) == 1);
    ASSERT_TEST(f.finishThief(thief_id+5) == 1);
    p_list = f.listAvailableProducts();
    ASSERT_TEST(p_list.size() == 0); //All stolen
    s_list = f.listStolenProducts();
    ASSERT_TEST(s_list.size() == 8); //2 from before + 5 now

    //Now check that multiple companies don't interfere
    f.closeFactory();
    f.startProduction(5, p_array, prod_id);
    f.startCompanyBuyer(1, -100, comp_id);
    f.startCompanyBuyer(1, -100, comp_id+1);
    f.startCompanyBuyer(1, -100, comp_id+2);
    f.startCompanyBuyer(1, -100, comp_id+3);
    f.startCompanyBuyer(1, -100, comp_id+4);
    f.finishProduction(prod_id);
    p_list = f.listAvailableProducts();
    ASSERT_TEST(p_list.size() == 5);
    f.openFactory();
    ASSERT_TEST(f.finishCompanyBuyer(comp_id) == 0);
    ASSERT_TEST(f.finishCompanyBuyer(comp_id+1) == 0);
    ASSERT_TEST(f.finishCompanyBuyer(comp_id+2) == 0);
    ASSERT_TEST(f.finishCompanyBuyer(comp_id+3) == 0);
    ASSERT_TEST(f.finishCompanyBuyer(comp_id+4) == 0);
    p_list = f.listAvailableProducts();
    ASSERT_TEST(p_list.size() == 0);

    //Now let a company stumble on production, and see that a buyer thread can buy despite it.
    f.startCompanyBuyer(9, -100, comp_id);
    f.startProduction(5, p_array, prod_id);
    f.finishProduction(prod_id);
    usleep(300000); //So that comp buyer for sure will reach waiting status
    f.startSimpleBuyer(buyer_id);
    int simpleBuyerRes = f.finishSimpleBuyer(buyer_id);
    ASSERT_TEST(simpleBuyerRes == 1); //Bought product with id 1
    f.startProduction(5, p_array, prod_id);
    f.finishProduction(prod_id);
    ASSERT_TEST(f.finishCompanyBuyer(comp_id) == 0);
    p_list = f.listAvailableProducts();
    ASSERT_TEST(p_list.size() == 0);

    //Now let a company stumble on returns and see that a buyer thread can buy
    f.closeReturningService();
    f.startCompanyBuyer(4, 300, comp_id); //Last product stays in factory so buyer will buy it
    f.startProduction(5, p_array, prod_id);
    f.finishProduction(prod_id);
    p_list = f.listAvailableProducts();
    usleep(300000); //So that comp buyer for sure will reach waiting status
    f.startSimpleBuyer(buyer_id);
    ASSERT_TEST(f.finishSimpleBuyer(buyer_id) == 5); //ID of product bought is 500
    f.openReturningService();
    ASSERT_TEST(f.finishCompanyBuyer(comp_id) == 2);
    p_list = f.listAvailableProducts();
    ASSERT_TEST(p_list.size() == 2);

    //Now we will see that buyers are blocked
    f.closeFactory();
    f.startProduction(5, p_array, prod_id);
    f.finishProduction(prod_id);
    p_list = f.listAvailableProducts();
    ASSERT_TEST(p_list.size() == 7);
    f.startThief(5, thief_id);
    f.startCompanyBuyer(2, 0, comp_id);
    //All buyer threads will fail
    f.startSimpleBuyer(buyer_id);
    f.startSimpleBuyer(buyer_id+1);
    f.startSimpleBuyer(buyer_id+2);
    usleep(300000); //So that all buyers will try and go away
    f.openFactory();
    ASSERT_TEST(f.finishSimpleBuyer(buyer_id) == -1);
    ASSERT_TEST(f.finishSimpleBuyer(buyer_id+1) == -1);
    ASSERT_TEST(f.finishSimpleBuyer(buyer_id+2) == -1);
    ASSERT_TEST(f.finishCompanyBuyer(comp_id) == 0);
    ASSERT_TEST(f.finishThief(thief_id) == 5);

    return true;
}

int main(){
    RUN_TEST(basicProduceTest);
    RUN_TEST(basicBuyerTest);
    RUN_TEST(basicCompanyTest);
    RUN_TEST(basicThiefTest);
    RUN_TEST(threadlessTest);
    RUN_TEST(testAccessOrder);
}
