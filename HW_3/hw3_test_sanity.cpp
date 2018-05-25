#include "Factory.h"
#include "test_utilities.h"

#define NUM_PRODUCTS 50
#define MIN_PURCHESE 3

using namespace std;

int main(){
    Factory factory = Factory();
    Product products[NUM_PRODUCTS];

    for(int i =0; i < NUM_PRODUCTS; i++){
        products[i] = Product(i,i*10);
    }
    int num_local_products = 0;
    factory.produce(NUM_PRODUCTS,products);
    ASSERT_TEST(factory.tryBuyOne() == 0);
    num_local_products++;
    ASSERT_TEST(factory.listAvailableProducts().size() == NUM_PRODUCTS - num_local_products);

    list<Product> purchase = factory.buyProducts(MIN_PURCHESE);
    num_local_products += MIN_PURCHESE;
    ASSERT_TEST(purchase.size() == MIN_PURCHESE);
    ASSERT_TEST(factory.listAvailableProducts().size() == NUM_PRODUCTS-num_local_products);

    factory.returnProducts(purchase,0);
    num_local_products -= purchase.size();
    ASSERT_TEST(factory.listAvailableProducts().size() == NUM_PRODUCTS - num_local_products);


    ASSERT_TEST(factory.stealProducts(10,47) == 10);
    num_local_products +=10;
    ASSERT_TEST(factory.listStolenProducts().size() == 10);
    ASSERT_TEST(factory.listAvailableProducts().size() == NUM_PRODUCTS - num_local_products);

    int current_available =(int)factory.listAvailableProducts().size();

    ASSERT_TEST(factory.stealProducts(NUM_PRODUCTS,48) == current_available);
    num_local_products  = NUM_PRODUCTS;
    ASSERT_TEST(factory.listStolenProducts().size() == NUM_PRODUCTS - 1);
    ASSERT_TEST(factory.listAvailableProducts().size() == 0);
    return 0;
}