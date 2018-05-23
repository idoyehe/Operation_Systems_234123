#ifndef FACTORY_H_
#define FACTORY_H_

#include <pthread.h>
#include <list>
#include <map>
#include "Product.h"

class Factory {
    std::list<std::pair<Product, int>> _listStolenProducts;
    std::list<Product> _listAvailableProducts;
    std::map<int, pthread_t> _id2threadMAP;

    bool is_open;
    bool is_return;
    pthread_cond_t cond_factory_produce;

    int number_of_map_writers;
    pthread_cond_t cond_map;
    pthread_mutex_t mutex_map;


    int number_of_resource_writers;
    int number_of_resource_readers;
    pthread_cond_t cond_read;

    int waiting_thieves_counter;
    pthread_cond_t cond_thief;
    int waiting_companies_counter;
    pthread_cond_t cond_company;
    int waiting_buyers_counter;
    pthread_cond_t cond_costumer;
    pthread_mutex_t mutex_general_factory;


    void read_Lock();

    void read_Unlock();

    void write_lock_factory_produce();

    void write_lock_thieves();

    void write_lock_company(int num_products, bool is_returned);

    void write_lock_single_buyer();

    void write_unlock();


public:
    Factory();

    ~Factory();

    void startProduction(int num_products, Product *products, unsigned int id);

    void produce(int num_products, Product *products);

    void finishProduction(unsigned int id);

    void startSimpleBuyer(unsigned int id);

    int tryBuyOne();

    int finishSimpleBuyer(unsigned int id);

    void startCompanyBuyer(int num_products, int min_value, unsigned int id);

    std::list<Product> buyProducts(int num_products);

    void returnProducts(std::list<Product> products, unsigned int id);

    int finishCompanyBuyer(unsigned int id);

    void startThief(int num_products, unsigned int fake_id);

    int stealProducts(int num_products, unsigned int fake_id);

    int finishThief(unsigned int fake_id);

    void closeFactory();

    void openFactory();

    void closeReturningService();

    void openReturningService();

    std::list<std::pair<Product, int>> listStolenProducts();

    std::list<Product> listAvailableProducts();

    void write_lock_map();

    void write_unlock_map();

    void insertToMap(int id, pthread_t p);

    void removeFromMap(int id);

    pthread_t getThreadIDMap(int id);
};
#endif // FACTORY_H_
