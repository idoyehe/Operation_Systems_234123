#include "Factory.h"
#include <assert.h>
#include <iostream>

#define NO_INIT -1

pthread_mutex_t mutex_printer;//TODO: delete after debug
pthread_cond_t cond_printer;//TODO: delete after debug
int number_of_printers = 0;

void print_lock() {//TODO: delete after debug
    pthread_mutex_lock(&mutex_printer);//TODO: delete after debug
    while (number_of_printers > 0) {//TODO: delete after debug
        pthread_cond_wait(&cond_printer, &mutex_printer);//TODO: delete after debug
    }//TODO: delete after debug
    number_of_printers++;//TODO: delete after debug
    pthread_mutex_unlock(&mutex_printer);//TODO: delete after debug
}//TODO: delete after debug

void print_unlock() {//TODO: delete after debug
    pthread_mutex_lock(&mutex_printer);//TODO: delete after debug
    number_of_printers--;//TODO: delete after debug
    if (number_of_printers== 0) {//TODO: delete after debug
        pthread_cond_signal(&cond_printer);//TODO: delete after debug
    }//TODO: delete after debug
    pthread_mutex_unlock(&mutex_printer);//TODO: delete after debug
}//TODO: delete after debug

void safePrint(const char * str){
    print_lock();
    std::cout << "\n-------------------------\n";
    std::cout << str;
    std::cout << "\n-------------------------\n";
    print_unlock();
}

class Buffer {
    public:
    Factory* factory_pointer;
    void* stl_pointer;
    Product* product_arr;
    int num_products;
    int id;
    int fake_id;
    int min_value;

    Buffer(){
        buffer->factory_pointer = NULL;
        buffer->stl_pointer = NULL;
        buffer->product_arr = NULL;
        buffer->fake_id = NO_INIT;
        buffer->id = NO_INIT;
        buffer->num_products = NO_INIT;
        buffer->min_value = NO_INIT;
    }
};

static void* produceThreadWrapper(void *args) {
    Buffer* buffer = (Buffer*)args;

    buffer->factory_pointer->write_lock_map();
    buffer->factory_pointer->insertToMap(buffer->id,pthread_self());
    buffer->factory_pointer->write_unlock_map();

    safePrint("produce updated map");//TODO: delete after debug
    buffer->factory_pointer->produce(buffer->num_products,buffer->product_arr);
    safePrint("produce Done");//TODO: delete after debug
    delete buffer;
    pthread_exit(NULL);
}

void *thiefThreadWrapper(void * args) {
    Buffer* buffer = (Buffer*)args;

    buffer->factory_pointer->write_lock_map();
    buffer->factory_pointer->insertToMap(buffer->fake_id,pthread_self());
    buffer->factory_pointer->write_unlock_map();

    safePrint("thief updated map");//TODO: delete after debug

    int *number_of_stolen = new int;
    *number_of_stolen = buffer->factory_pointer->stealProducts(buffer->num_products,buffer->fake_id);
    safePrint("thief Done");//TODO: delete after debug
    delete buffer;
    pthread_exit(number_of_stolen);
}

void *companyThreadWrapper(void * args) {
    Buffer* buffer = (Buffer*)args;

    buffer->factory_pointer->write_lock_map();
    buffer->factory_pointer->insertToMap(buffer->id,pthread_self());
    buffer->factory_pointer->write_unlock_map();

    std::list<Product> company_products = buffer->factory_pointer->buyProducts(buffer->num_products);
    std::list<Product> returned_products;
    for(std::list<Product>::iterator it = company_products.begin(), end = company_products.end(); it != end; ++it){
        if((*it).getValue() < buffer->min_value){
            returned_products.push_back((*it));
        }
    }
    int *number_of_returned = new int;
    *number_of_returned =(int)returned_products.size();
    buffer->factory_pointer->returnProducts(returned_products,buffer->id);
    delete buffer;
    pthread_exit(number_of_returned);
}

void *buyerThreadWrapper(void * args) {
    Buffer* buffer = (Buffer*)args;

    buffer->factory_pointer->write_lock_map();
    buffer->factory_pointer->insertToMap(buffer->id,pthread_self());
    buffer->factory_pointer->write_unlock_map();

    int *procuct_id = new int;
    *procuct_id = buffer->factory_pointer->tryBuyOne();
    delete buffer;
    pthread_exit(procuct_id);
}

Factory::Factory(){
    pthread_cond_init(&cond_printer,NULL);//TODO: delete after debug
    pthread_mutex_init(&mutex_printer,NULL);//TODO: delete after debug

    this->is_open = true;
    this->is_return = true;

    pthread_cond_init(&(this->cond_factory_produce),NULL);

    this->number_of_map_writers = 0;
    pthread_cond_init(&(this->cond_map),NULL);
    pthread_mutex_init(&(this->mutex_map),NULL);

    this->number_of_resource_readers = 0;
    this->number_of_resource_writers = 0;
    pthread_cond_init(&(this->cond_read),NULL);

    this->waiting_thieves_counter = 0;
    pthread_cond_init(&(this->cond_thief),NULL);

    this->waiting_companies_counter = 0;
    pthread_cond_init(&(this->cond_company),NULL);

    this->waiting_buyers_counter = 0;
    pthread_cond_init(&(this->cond_costumer),NULL);

    pthread_mutex_init(&(this->mutex_general_factory),NULL);
}

Factory::~Factory(){
    pthread_cond_destroy(&(this->cond_read));
    pthread_cond_destroy(&(this->cond_factory_produce));
    pthread_cond_destroy(&(this->cond_thief));
    pthread_cond_destroy(&(this->cond_company));
    pthread_cond_destroy(&(this->cond_costumer));
    pthread_mutex_destroy(&(this->mutex_general_factory));

    pthread_cond_destroy(&(this->cond_map));
    pthread_mutex_destroy(&(this->mutex_map));

}

void Factory::startProduction(int num_products, Product* products,unsigned int id){
    assert(num_products >0);
    assert(products != nullptr);
    pthread_t p;
    Buffer *buffer = new Buffer();
    buffer->factory_pointer = this;
    buffer->num_products = num_products;
    buffer->product_arr = products;
    buffer->id = id;
    pthread_create(&p, NULL, produceThreadWrapper, buffer);
}

void Factory::produce(int num_products, Product* products){
    write_lock_factory_produce();
    for(int i = 0; i < num_products; i++){
        this->_listAvailableProducts.push_back(products[i]);
    }
    write_unlock();
}

void Factory::finishProduction(unsigned int id){
    write_lock_map();
    pthread_t p = this->getThreadIDMap(id);
    this->removeFromMap(id);
    write_unlock_map();
    pthread_join(p,NULL);
}

void Factory::startSimpleBuyer(unsigned int id){
    pthread_t p;
    Buffer *buffer = new Buffer();
    buffer->factory_pointer = this;
    buffer->id = id;
    pthread_create(&p,NULL,buyerThreadWrapper,buffer);
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
    write_lock_map();
    pthread_t p = this->getThreadIDMap(id);
    this->removeFromMap(id);
    write_unlock_map();
    void *buffer;
    pthread_join(p,&buffer);
    assert(buffer != NULL);
    int* product_id = (int*)(buffer);
    int return_id = *product_id;
    delete product_id;
    return return_id;
}

void Factory::startCompanyBuyer(int num_products, int min_value,unsigned int id){
    assert(num_products > 0);
    pthread_t p;
    Buffer *buffer = new Buffer();
    buffer->factory_pointer = this;
    buffer->num_products = num_products;
    buffer->min_value = min_value;
    buffer->id = id;
    pthread_create(&p,NULL,companyThreadWrapper,&buffer);
}

std::list<Product> Factory::buyProducts(int num_products){
    write_lock_company(num_products, true);
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
    write_lock_company(NO_INIT,this->is_return);// sending negative number to remove dependency in num_product
    for(std::list<Product>::iterator it = products.begin(), end = products.end(); it != end; ++it){
        this->_listAvailableProducts.push_back(*it);
    }
    write_unlock();
}

int Factory::finishCompanyBuyer(unsigned int id){
    write_lock_map();
    pthread_t p = this->getThreadIDMap(id);
    this->removeFromMap(id);
    write_unlock_map();
    void *buffer;
    pthread_join(p,&buffer);
    assert(buffer != NULL);
    int* num_returned = (int*)(buffer);
    int number_of_returns = *num_returned;
    delete num_returned;
    return number_of_returns;
}

void Factory::startThief(int num_products,unsigned int fake_id){
    assert(num_products > 0);
    Buffer *buffer = new Buffer();
    buffer->factory_pointer = this;
    buffer->num_products = num_products;
    buffer->fake_id = fake_id;
    pthread_t p;
    pthread_create(&p,NULL,thiefThreadWrapper,buffer);
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
    write_lock_map();
    pthread_t p = this->getThreadIDMap(fake_id);
    this->removeFromMap(fake_id);
    write_unlock_map();
    void *buffer;
    pthread_join(p,&buffer);
    assert(buffer != NULL);
    int* num_stolen = (int*)(buffer);
    int number_of_stolen = *num_stolen;
    delete num_stolen;
    return number_of_stolen;
}

void Factory::closeFactory(){
    this->is_open = false;
}

void Factory::openFactory(){
    this->is_open = true;
    if(this->number_of_resource_writers == 0 && this->number_of_resource_readers == 0){
        pthread_cond_broadcast(&(this->cond_read));
        pthread_cond_signal(&(this->cond_factory_produce));
        if(this->is_open){
            if(this->waiting_thieves_counter > 0) {
                pthread_cond_signal(&(this->cond_thief));
                pthread_mutex_unlock(&(this->mutex_general_factory));
                return;
            }
            if(this->waiting_companies_counter > 0){
                pthread_cond_signal(&(this->cond_company));
                pthread_mutex_unlock(&(this->mutex_general_factory));
                return;
            }
            if(this->waiting_buyers_counter > 0){
                pthread_cond_signal(&(this->cond_costumer));
                pthread_mutex_unlock(&(this->mutex_general_factory));
                return;
            }
        }
    }

void Factory::closeReturningService(){
    this->is_return = false;
}

void Factory::openReturningService(){
    this->is_return = true;
    if(this->number_of_resource_writers == 0 && this->number_of_resource_readers == 0){
        pthread_cond_broadcast(&(this->cond_read));
        pthread_cond_signal(&(this->cond_factory_produce));
        if(this->is_open){
            if(this->waiting_thieves_counter > 0) {
                pthread_cond_signal(&(this->cond_thief));
                pthread_mutex_unlock(&(this->mutex_general_factory));
                return;
            }
            if(this->waiting_companies_counter > 0){
                pthread_cond_signal(&(this->cond_company));
                pthread_mutex_unlock(&(this->mutex_general_factory));
                return;
            }
            if(this->waiting_buyers_counter > 0){
                pthread_cond_signal(&(this->cond_costumer));
                pthread_mutex_unlock(&(this->mutex_general_factory));
                return;
            }
        }
    }
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
    pthread_mutex_lock(&(this->mutex_general_factory));
    while(this->number_of_resource_writers > 0){
        pthread_cond_wait(&(this->cond_read),&(this->mutex_general_factory));
    }
    this->number_of_resource_readers++;
    pthread_mutex_unlock(&(this->mutex_general_factory));
}

void Factory::read_Unlock() {
    pthread_mutex_lock(&(this->mutex_general_factory));
    this->number_of_resource_readers--;
    if (this->number_of_resource_readers == 0) {
        pthread_cond_signal(&(this->cond_factory_produce));
        if (this->is_open) {
            if (this->waiting_thieves_counter > 0) {
                pthread_cond_signal(&(this->cond_thief));
                pthread_mutex_unlock(&(this->mutex_general_factory));
                return;
            }
            if (this->waiting_companies_counter > 0) {
                pthread_cond_signal(&(this->cond_company));
                pthread_mutex_unlock(&(this->mutex_general_factory));
                return;
            }
            if (this->waiting_buyers_counter > 0) {
                pthread_cond_signal(&(this->cond_costumer));
                pthread_mutex_unlock(&(this->mutex_general_factory));
                return;
            }
        }
    }
    pthread_mutex_unlock(&(this->mutex_general_factory));
}

void Factory::write_lock_thieves() {
    pthread_mutex_lock(&(this->mutex_general_factory));
    this->waiting_thieves_counter++;
    while(!this->is_open||this->number_of_resource_writers > 0 || this->number_of_resource_readers > 0){
        pthread_cond_wait(&(this->cond_thief),&(this->mutex_general_factory));
    }
    this->waiting_thieves_counter--;
    this->number_of_resource_writers++;
    pthread_mutex_unlock(&(this->mutex_general_factory));
}

void Factory::write_lock_company(int num_products,bool is_returned) {
    pthread_mutex_lock(&(this->mutex_general_factory));
    this->waiting_companies_counter++;
    while(!this->is_open || this->number_of_resource_writers > 0 || this->number_of_resource_readers > 0
          || num_products > this->_listAvailableProducts.size() || !is_returned){
        pthread_cond_wait(&(this->cond_company),&(this->mutex_general_factory));
    }
    this->waiting_companies_counter--;
    this->number_of_resource_writers++;
    pthread_mutex_unlock(&(this->mutex_general_factory));
}

void Factory::write_lock_single_buyer() {
    pthread_mutex_lock(&(this->mutex_general_factory));
    this->waiting_buyers_counter++;
    while(!this->is_open || this->number_of_resource_writers > 0 || this->number_of_resource_readers > 0){
        pthread_cond_wait(&(this->cond_costumer),&(this->mutex_general_factory));
    }
    this->waiting_buyers_counter--;
    this->number_of_resource_writers++;
    pthread_mutex_unlock(&(this->mutex_general_factory));
}

void Factory::write_lock_factory_produce() {
    pthread_mutex_lock(&(this->mutex_general_factory));
    while(this->number_of_resource_writers > 0 || this->number_of_resource_readers > 0){
        pthread_cond_wait(&(this->cond_factory_produce),&(this->mutex_general_factory));
    }
    this->number_of_resource_writers++;
    pthread_mutex_unlock(&(this->mutex_general_factory));
}

void Factory::write_lock_factory_control() {
    pthread_mutex_lock(&(this->mutex_general_factory));
    this->waiting_control_counter = ++;
    while(this->number_of_resource_writers > 0 || this->number_of_resource_readers > 0){
        pthread_cond_wait(&(this->cond_factory_control),&(this->mutex_general_factory));
    }
    this->waiting_control_counter = --;
    this->number_of_resource_writers++;
    pthread_mutex_unlock(&(this->mutex_general_factory));
}

void Factory::write_unlock() {
    pthread_mutex_lock(&(this->mutex_general_factory));
    this->number_of_resource_writers--;
    if(this->number_of_resource_writers == 0){
        pthread_cond_broadcast(&(this->cond_read));
        pthread_cond_signal(&(this->cond_factory_produce));
        if(this->is_open){
            if(this->waiting_thieves_counter > 0) {
                pthread_cond_signal(&(this->cond_thief));
                pthread_mutex_unlock(&(this->mutex_general_factory));
                return;
            }
            if(this->waiting_companies_counter > 0){
                pthread_cond_signal(&(this->cond_company));
                pthread_mutex_unlock(&(this->mutex_general_factory));
                return;
            }
            if(this->waiting_buyers_counter > 0){
                pthread_cond_signal(&(this->cond_costumer));
                pthread_mutex_unlock(&(this->mutex_general_factory));
                return;
            }
        }
    }
    pthread_mutex_unlock(&(this->mutex_general_factory));
}

void Factory::insertToMap(int id, pthread_t p) {
    this->_id2threadMAP.insert(std::pair<int,pthread_t>(id,p));
}

void Factory::removeFromMap(int id){
    this->_id2threadMAP.erase(id);
}

pthread_t Factory::getThreadIDMap(int id){
    return (*(this->_id2threadMAP.find(id))).second;
}

void Factory::write_lock_map() {
    pthread_mutex_lock(&(this->mutex_map));
    while(this->number_of_map_writers > 0){
        pthread_cond_wait(&(this->cond_map),&(this->mutex_map));
    }
    this->number_of_map_writers++;
    pthread_mutex_unlock(&(this->mutex_map));
}

void Factory::write_unlock_map() {
    pthread_mutex_lock(&(this->mutex_map));
    this->number_of_map_writers--;
    if (this->number_of_map_writers == 0) {
        pthread_cond_signal(&this->cond_map);
    }
    pthread_mutex_unlock(&(this->mutex_map));
}