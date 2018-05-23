#include "Factory.h"
#include <assert.h>

#define RETURN_PRODUCTS_VALUE -1

Factory::Factory(){
    this->is_open = true;
    this->is_return = true;

    pthread_cond_init(&(this->cond_factory_produce),NULL);

    this->number_of_readers = 0;
    this->number_of_writers = 0;
    pthread_cond_init(&(this->cond_read),NULL);
    pthread_cond_init(&(this->cond_write_factory_open),NULL);
    pthread_cond_init(&(this->cond_write_globals),NULL);
    pthread_mutex_init(&(this->mutex_read_write),NULL);

    this->thief_counter = 0;
    pthread_cond_init(&(this->cond_thief),NULL);

    this->company_counter = 0;
    pthread_cond_init(&(this->cond_company),NULL);

    this->costumer_counter = 0;
    pthread_cond_init(&(this->cond_costumer),NULL);

}

Factory::~Factory(){

    pthread_cond_destroy(&(this->cond_read));
    pthread_cond_destroy(&(this->cond_write_factory_open));
    pthread_cond_destroy(&(this->cond_write_globals));
    pthread_mutex_destroy(&(this->mutex_read_write));

    pthread_cond_destroy(&(this->cond_thief));

    pthread_cond_destroy(&(this->cond_company));

    pthread_cond_destroy(&(this->cond_costumer));


}

void Factory::startProduction(int num_products, Product* products,unsigned int id){
    assert(num_products >0);
    assert(products != nullptr);
    pthread_t p;
    std::list<Product> new_products;
    for(int i = 0; i < num_products; i++){
        new_products.push_back(products[i]);
    }
    pthread_create(&p,NULL,(Factory::produceWrapper),(void*)&new_products);
}

void* Factory::produceWrapper(void * products) {
    std::list<Product>* new_products;
    new_products = (std::list<Product>*)products;
    int num_products = (int) new_products->size();
    Product* product_array = new Product[num_products];
    int i=0;
    for(std::list<Product>::iterator it = new_products->begin(), end = new_products->end(); it != end; ++it){
        product_array[i++] = *it;
    }
    produce(num_products,product_array);
    delete[](product_array);
    pthread_exit(NULL);
}

void Factory::produce(int num_products, Product* products){
    write_lock_factory_produce();
    for(int i = 0; i < num_products; i++){
        this->_listAvailableProducts.push_back(products[i]);
    }
    write_unlock();
}

void Factory::finishProduction(unsigned int id){
}

void Factory::startSimpleBuyer(unsigned int id){
}

int Factory::tryBuyOne(){
    return -1;
}

int Factory::finishSimpleBuyer(unsigned int id){
    return -1;
}

void Factory::startCompanyBuyer(int num_products, int min_value,unsigned int id){
    assert(num_products > 0);
    int args[3] = {num_products,min_value,(int)id};
    pthread_t p;
    pthread_create(&p,NULL,this->companyThreadWrapper,(void*)args);

}

void *Factory::companyThreadWrapper(void * args) {
    int* integer_args =(int*)args;
    int num_products = integer_args[0];
    int min_value = integer_args[1];
    unsigned int id = (unsigned)integer_args[2];
    std::list<Product> company_products = buyProducts(num_products);
    std::list<Product> returned_products;
    for(std::list<Product>::iterator it = company_products.begin(), end = company_products.end(); it != end; ++it){
        if((*it).getValue() < min_value){
            returned_products.push_back((*it));
        }
    }
    returnProducts(returned_products,id);
    pthread_exit(NULL);

}



std::list<Product> Factory::buyProducts(int num_products){
    write_lock_company(num_products);
    assert(num_products <= _listAvailableProducts.size());
    std::list<Product>::iterator it = this->_listAvailableProducts.begin();
    std::list<Product> company_products;
    for(int i = 0; i < num_products ; i++){
        company_products.push_back(*(it));
        this->_listAvailableProducts.pop_front();
        it = this->_listAvailableProducts.begin();
    }
    write_unlock();
    return company_products;
}

void Factory::returnProducts(std::list<Product> products,unsigned int id){
    write_lock_company(RETURN_PRODUCTS_VALUE);// sending negative number to remove dependency in num_product
    for(std::list<Product>::iterator it = products.begin(), end = products.end(); it != end; ++it){
        this->_listAvailableProducts.push_back(*it);
    }
    write_unlock();
}

int Factory::finishCompanyBuyer(unsigned int id){
    return 0;
}

void Factory::startThief(int num_products,unsigned int fake_id){
    assert(num_products > 0);
    int args[2] = {num_products,(int)fake_id};
    pthread_t p;
    pthread_create(&p,NULL,this->thiefWrapper,(void*)args);
}

void *Factory::thiefWrapper(void * args) {
    int* integer_args =(int*)args;
    stealProducts(integer_args[0],(unsigned)integer_args[1]);
    pthread_exit(NULL);
}

int Factory::stealProducts(int num_products,unsigned int fake_id){
    this->write_lock_thieves();
    int list_size =(int)this->_listAvailableProducts.size();
    int min = (num_products > list_size) ? list_size : num_products;
    std::list<Product>::iterator it = this->_listAvailableProducts.begin();
    for(int i = 0; i < min ; i++){
        this->_listStolenProducts.push_back(std::pair<Product,int>(*(it),fake_id));
        this->_listAvailableProducts.pop_front();
        it = this->_listAvailableProducts.begin();
    }
    this->write_unlock();
    return min;
}

int Factory::finishThief(unsigned int fake_id){
    return 0;
}

void Factory::closeFactory(){
    this->is_open = false;
}

void Factory::openFactory(){
    this->is_open = true;
}

void Factory::closeReturningService(){
    this->is_return = false;
}

void Factory::openReturningService(){
    this->is_return = true;
}

std::list<std::pair<Product, int>> Factory::listStolenProducts(){
    read_Lock();
    std::list<std::pair<Product, int>> copy = this->_listStolenProducts;
    read_Unlock();
    return copy;
}

std::list<Product> Factory::listAvailableProducts(){
    read_Lock();
    std::list<Product> copy = this->_listAvailableProducts;
    read_Unlock();
    return copy;
}

void Factory::read_Lock() {
    pthread_mutex_lock(&(this->mutex_read_write));
    while(this->number_of_writers > 0){
        pthread_cond_wait(&(this->cond_read),&(this->mutex_read_write));
    }
    this->number_of_readers++;
    pthread_mutex_unlock(&(this->mutex_read_write));
}

void Factory::read_Unlock() {
    pthread_mutex_lock(&(this->mutex_read_write));
    this->number_of_readers--;
    if(this->number_of_readers == 0){
        pthread_cond_signal(&(this->cond_write_globals));
        if(this->is_open){
            pthread_cond_signal(&(this->cond_write_factory_open));
        }
    }
    pthread_mutex_unlock(&(this->mutex_read_write));
}

void Factory::write_lock_thieves() {
    pthread_mutex_lock(&(this->mutex_read_write));
    while(this->number_of_writers > 0 || this->number_of_readers > 0){
        this->thief_counter++;
        pthread_cond_wait(&(this->cond_thief),&(this->mutex_read_write));
        this->thief_counter--;
    }
    this->number_of_writers++;
    pthread_mutex_unlock(&(this->mutex_read_write));
}

void Factory::write_lock_company(int num_products) {
    pthread_mutex_lock(&(this->mutex_read_write));
    while(this->number_of_writers > 0 || this->number_of_readers > 0|| num_products > this->_listAvailableProducts.size()){
        this->company_counter++;
        pthread_cond_wait(&(this->cond_company),&(this->mutex_read_write));
        this->company_counter--;
    }
    this->number_of_writers++;
    pthread_mutex_unlock(&(this->mutex_read_write));
}

void Factory::write_lock_factory_produce() {
    pthread_mutex_lock(&(this->mutex_read_write));
    while(this->number_of_writers > 0 || this->number_of_readers > 0){
        pthread_cond_wait(&(this->cond_factory_produce),&(this->mutex_read_write));
    }
    this->number_of_writers++;
    pthread_mutex_unlock(&(this->mutex_read_write));
}

void Factory::write_unlock() {
    pthread_mutex_lock(&(this->mutex_read_write));
    this->number_of_writers--;
    if(this->number_of_writers == 0){
        pthread_cond_broadcast(&(this->cond_read));
        pthread_cond_signal(&(this->cond_factory_produce));
        if(this->is_open){           //TODO adding priority
            if(this->thief_counter > 0) {
                pthread_cond_signal(&(this->cond_thief));
                pthread_mutex_unlock(&(this->mutex_read_write));
                return;
            }
        }
    }
}


