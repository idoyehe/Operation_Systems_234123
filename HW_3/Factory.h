#ifndef FACTORY_H_
#define FACTORY_H_

#include <pthread.h>
#include <list>
#include "Product.h"

class Factory{
    std::list<std::pair<Product, int>> listStolenProducts;
    std::list<Product> listAvailableProducts;
    pthread_cond_t cond_factory_produce;

    bool is_open;
    bool is_return;

    int number_of_writers;
    int number_of_readers;
    pthread_cond_t cond_read;
    pthread_cond_t cond_write_factory_open;
    pthread_cond_t cond_write_globals;
    pthread_mutex_t mutex_read_write;

    int thief_counter;
    pthread_cond_t cond_thief;
    int company_counter;
    pthread_cond_t cond_company;
    int costumer_counter;
    pthread_cond_t cond_costumer;


    void read_Lock();
    void read_Unlock();
    void write_lock_thieves();
    void write_lock_company(int num_products);
    void write_lock_factory_produce();
    void write_unlock();
    void* produceWrapper(void* args);
    void *thiefWrapper(void *args);
    void *companyThreadWrapper(void *args);




public:
    Factory();
    ~Factory();
    
    void startProduction(int num_products, Product* products, unsigned int id);
    void produce(int num_products, Product* products);
    void finishProduction(unsigned int id);
    
    void startSimpleBuyer(unsigned int id);
    int tryBuyOne();
    int finishSimpleBuyer(unsigned int id);

    void startCompanyBuyer(int num_products, int min_value,unsigned int id);
    std::list<Product> buyProducts(int num_products);
    void returnProducts(std::list<Product> products,unsigned int id);
    int finishCompanyBuyer(unsigned int id);

    void startThief(int num_products,unsigned int fake_id);
    int stealProducts(int num_products,unsigned int fake_id);
    int finishThief(unsigned int fake_id);

    void closeFactory();
    void openFactory();
    
    void closeReturningService();
    void openReturningService();
    
    std::list<std::pair<Product, int>> listStolenProducts();
    std::list<Product> listAvailableProducts();
};
#endif // FACTORY_H_
