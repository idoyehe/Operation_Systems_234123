#include "Factory.h"
#include <assert.h>

#define NO_INIT -1

typedef struct {
    Factory* factory_pointer;
    void* stl_pointer;
    Product* product_arr;
    int num_products;
    int id;
    int fake_id;
    int min_value;
}Buffer;

void bufferInit(Buffer* buffer){
    if(buffer == NULL){
        return;
    }
    buffer->factory_pointer = NULL;
    buffer->stl_pointer = NULL;
    buffer->product_arr = NULL;
    buffer->fake_id = NO_INIT;
    buffer->id = NO_INIT;
    buffer->num_products = NO_INIT;
    buffer->min_value = NO_INIT;

}

static void* produceThreadWrapper(void *args) {
    Buffer* buffer = (Buffer*)args;
    buffer->factory_pointer->produce(buffer->num_products,buffer->product_arr);
    pthread_exit(NULL);
}

void *companyThreadWrapper(void * args) {
    Buffer* buffer = (Buffer*)args;
    std::list<Product> company_products = buffer->factory_pointer->buyProducts(buffer->num_products);
    std::list<Product> returned_products;
    for(std::list<Product>::iterator it = company_products.begin(), end = company_products.end(); it != end; ++it){
        if((*it).getValue() < buffer->min_value){
            returned_products.push_back((*it));
        }
    }
    buffer->factory_pointer->returnProducts(returned_products,buffer->id);
    pthread_exit(NULL);
}

void *thiefThreadWrapper(void * args) {
    Buffer* buffer = (Buffer*)args;
    buffer->factory_pointer->stealProducts(buffer->num_products,buffer->fake_id);
    pthread_exit(NULL);
}

void *buyerThreadWrapper(void * args) {
    Buffer* buffer = (Buffer*)args;
    buffer->factory_pointer->tryBuyOne();
    pthread_exit(NULL);
}



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

    this->buyer_counter = 0;
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
    Buffer buffer;
    bufferInit(&buffer);
    buffer.factory_pointer = this;
    buffer.num_products = num_products;
    pthread_create(&p, NULL, produceThreadWrapper, &buffer);
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
    pthread_t p;
    Buffer buffer;
    bufferInit(&buffer);
    buffer.factory_pointer = this;
    buffer.id = id;
}

int Factory::tryBuyOne(){
    write_lock_single_buyer();
    if(this->_listAvailableProducts.empty()){
        return -1;
    }
    int product_id = this->_listAvailableProducts.front().getId();
    this->_listAvailableProducts.pop_front();
    write_unlock();
    return product_id;
}

int Factory::finishSimpleBuyer(unsigned int id){
    return -1;
}

void Factory::startCompanyBuyer(int num_products, int min_value,unsigned int id){
    assert(num_products > 0);
    pthread_t p;
    Buffer buffer;
    bufferInit(&buffer);
    buffer.factory_pointer = this;
    buffer.num_products = num_products;
    buffer.min_value = min_value;
    buffer.id = id;
    pthread_create(&p,NULL,companyThreadWrapper,&buffer);
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
    write_lock_company(NO_INIT);// sending negative number to remove dependency in num_product
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
    Buffer buffer;
    bufferInit(&buffer);
    buffer.factory_pointer = this;
    buffer.num_products = num_products;
    buffer.fake_id = fake_id;
    pthread_t p;
    pthread_create(&p,NULL,thiefThreadWrapper,&buffer);
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

void Factory::write_lock_single_buyer() {
    pthread_mutex_lock(&(this->mutex_read_write));
    while(this->number_of_writers > 0 || this->number_of_readers > 0){
        this->buyer_counter++;
        pthread_cond_wait(&(this->cond_costumer),&(this->mutex_read_write));
        this->buyer_counter--;
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
            if(this->company_counter > 0){
                pthread_cond_signal(&(this->cond_company));
                pthread_mutex_unlock(&(this->mutex_read_write));
                return;
            }
            if(this->buyer_counter > 0){
                pthread_cond_signal(&(this->cond_costumer));
                pthread_mutex_unlock(&(this->mutex_read_write));
                return;
            }
        }
    }
    pthread_mutex_unlock(&(this->mutex_read_write));
}
