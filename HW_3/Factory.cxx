#include "Factory.h"
#include <assert.h>
#include <iostream>//TODO: delete after debug
#include <string>

#define NO_INIT -1

pthread_mutex_t mutex_printer;//TODO: delete after debug
pthread_cond_t cond_printer;//TODO: delete after debug
int number_of_printers = 0;
int my_time = 0;

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

void safePrint(const std::string str){
    print_lock();
    std::cout << "\n-------------------------\n";
    std::cout << str;
    std::cout << "\n-------------------------\n";
    print_unlock();
}

class Buffer {
public:
    Factory *factory_pointer;
    void *stl_pointer;
    Product *product_arr;
    int num_products;
    int id;
    int fake_id;
    int min_value;

    Buffer() :
            factory_pointer(NULL),
            stl_pointer(NULL),
            product_arr(NULL),
            num_products(NO_INIT),
            id(NO_INIT),
            fake_id(NO_INIT),
            min_value(NO_INIT) {}
};

static void* produceThreadWrapper(void *args) {
    Buffer* buffer = (Buffer*)args;

    buffer->factory_pointer->adder_lock_map();
    buffer->factory_pointer->insertToMap(buffer->id,pthread_self());
    buffer->factory_pointer->unlock_map();

    buffer->factory_pointer->produce(buffer->num_products,buffer->product_arr);
    delete buffer;
    pthread_exit(NULL);
}

static void *thiefThreadWrapper(void * args) {
    Buffer* buffer = (Buffer*)args;

    buffer->factory_pointer->adder_lock_map();
    buffer->factory_pointer->insertToMap(buffer->fake_id,pthread_self());
    buffer->factory_pointer->unlock_map();
    safePrint("thief No " + std::to_string(buffer->fake_id) + " updated map " + " time: " + std::to_string(my_time++));

    int *number_of_stolen = new int;
    *number_of_stolen = buffer->factory_pointer->stealProducts(buffer->num_products,(unsigned)buffer->fake_id);
    delete buffer;
    safePrint("thief No " + std::to_string(buffer->fake_id) + " stole "+ " time: " + std::to_string(my_time++));
    pthread_exit(number_of_stolen);
}

static void *companyThreadWrapper(void * args) {
    Buffer* buffer = (Buffer*)args;

    buffer->factory_pointer->adder_lock_map();
    buffer->factory_pointer->insertToMap(buffer->id,pthread_self());
    safePrint("company No " + std::to_string(buffer->id) + " updated map "+ " time: " + std::to_string(my_time++));
    buffer->factory_pointer->unlock_map();


    std::list<Product> company_products = buffer->factory_pointer->buyProducts(buffer->num_products);
    safePrint("company No " + std::to_string(buffer->id) + "after buy"+ " time: " + std::to_string(my_time++));

    std::list<Product> returned_products;
    for(std::list<Product>::iterator it = company_products.begin(), end = company_products.end(); it != end; ++it){
        if((*it).getValue() < buffer->min_value){
            returned_products.push_back((*it));
        }
    }
    int *number_of_returned = new int;
    *number_of_returned = (int) returned_products.size();
    safePrint("company No " + std::to_string(buffer->id) + "has "+ std::to_string(*number_of_returned)+" to return"+" time: " + std::to_string(my_time++));
    if((*number_of_returned) > 0) {
        buffer->factory_pointer->returnProducts(returned_products,0);
    }
    delete buffer;
    pthread_exit(number_of_returned);
}

static void *buyerThreadWrapper(void * args) {
    Buffer* buffer = (Buffer*)args;

    buffer->factory_pointer->adder_lock_map();
    buffer->factory_pointer->insertToMap(buffer->id,pthread_self());
    buffer->factory_pointer->unlock_map();
    safePrint("Buyer No " + std::to_string(buffer->id) + " updated map " + " time: " + std::to_string(my_time++));

    int *procuct_id = new int;
    *procuct_id = buffer->factory_pointer->tryBuyOne();
    safePrint("Buyer No " + std::to_string(buffer->id) + " after tryBuyOne " + " time: " + std::to_string(my_time++));
    delete buffer;
    pthread_exit(procuct_id);
}

Factory::Factory(){
    pthread_cond_init(&cond_printer,NULL);//TODO: delete after debug
    pthread_mutex_init(&mutex_printer,NULL);//TODO: delete after debug

    this->busy_map = false;
    this->map_adders_counter = 0;
    pthread_cond_init(&(this->cond_map_adders),NULL);
    pthread_cond_init(&(this->cond_map_remover),NULL);
    pthread_mutex_init(&(this->mutex_map),NULL);


    this->is_open = true;
    this->is_return = true;

    pthread_cond_init(&(this->cond_factory_produce),NULL);

    this->number_of_resource_writers = 0;
    this->number_of_resource_readers = 0;
    pthread_cond_init(&(this->cond_read),NULL);

    this->waiting_thieves_counter = 0;
    pthread_cond_init(&(this->cond_thief),NULL);

    pthread_cond_init(&(this->cond_no_thief),NULL);

    pthread_mutex_init(&(this->mutex_general_factory),NULL);
    this-> waiting_companies_counter = 0;

}

Factory::~Factory(){
    pthread_cond_destroy(&(this->cond_map_adders));
    pthread_cond_destroy(&(this->cond_map_remover));
    pthread_mutex_destroy(&(this->mutex_map));

    pthread_cond_destroy(&(this->cond_factory_produce));

    pthread_cond_destroy(&(this->cond_read));
    pthread_cond_destroy(&(this->cond_thief));
    pthread_cond_destroy(&(this->cond_no_thief));
    pthread_mutex_destroy(&(this->mutex_general_factory));
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
    this->map_adders_counter ++;
    if(pthread_create(&p, NULL, produceThreadWrapper, buffer) != 0){
        this->map_adders_counter --;
    }
}

void Factory::produce(int num_products, Product* products){
    write_lock_factory_produce();
    for(int i = 0; i < num_products; i++){
        this->_listAvailableProducts.push_back(products[i]);
    }
    write_unlock();
}

void Factory::finishProduction(unsigned int id){
    this->remover_lock_map();
    pthread_t p = this->getThreadIDMap(id);
    this->removeFromMap(id);
    unlock_map();
    pthread_join(p,NULL);
}

void Factory::startSimpleBuyer(unsigned int id){
    pthread_t p;
    Buffer *buffer = new Buffer();
    buffer->factory_pointer = this;
    buffer->id = id;
    this->map_adders_counter++;
    safePrint("Buyer No " + std::to_string(buffer->id) + " time: " + std::to_string(my_time++));
    if(pthread_create(&p,NULL,buyerThreadWrapper,buffer) != 0){
        this->map_adders_counter--;
    }
}

int Factory::tryBuyOne(){
    if(write_lock_buyer() == -1) {
        return -1;
    }
    if(this->_listAvailableProducts.empty()){
        write_unlock();
        return -1;
    }
    int product_id = this->_listAvailableProducts.front().getId();
    this->_listAvailableProducts.pop_front();
    write_unlock();
    return product_id;
}

int Factory::finishSimpleBuyer(unsigned int id){
    remover_lock_map();
    safePrint("finish Buyer No " + std::to_string(id) +" waiting to map "+ " time: " + std::to_string(my_time++));
    pthread_t p = this->getThreadIDMap(id);
    this->removeFromMap(id);
    unlock_map();
    safePrint("finish Buyer No " + std::to_string(id) +" updated map "+ " time: " + std::to_string(my_time++));

    void *ret_val;
    pthread_join(p,&ret_val);
    safePrint("finish Buyer No " + std::to_string(id) +" finish waiting to thread "+ " time: " + std::to_string(my_time++));
    assert(ret_val != NULL);
    int return_id = *(int*)ret_val;
    delete (int*)ret_val;
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
    this->map_adders_counter++;
    safePrint("company id No " + std::to_string(id) + " "+ " time: " + std::to_string(my_time++));
    if(pthread_create(&p, NULL,companyThreadWrapper,buffer) != 0){
        this->map_adders_counter--;
    }
}

void Factory::returnProducts(std::list<Product> products,unsigned int id){
    write_lock_company(NO_INIT, true);// sending negative number to remove dependency in num_product
    for(std::list<Product>::iterator it = products.begin(), end = products.end(); it != end; ++it){
        this->_listAvailableProducts.push_back(*it);
    }
    safePrint("company No " + std::to_string(id) + " after return "+ " time: " + std::to_string(my_time++));
    write_unlock();
}

int Factory::finishCompanyBuyer(unsigned int id){
    remover_lock_map();
    pthread_t p = this->getThreadIDMap(id);
    this->removeFromMap(id);
    unlock_map();

    void *ret_val;
    pthread_join(p,&ret_val);
    assert(ret_val != NULL);
    int number_of_returns = *(int*)ret_val;
    delete (int*)ret_val;
    return number_of_returns;
}

void Factory::startThief(int num_products,unsigned int fake_id){
    assert(num_products > 0);
    Buffer *buffer = new Buffer();
    buffer->factory_pointer = this;
    buffer->num_products = num_products;
    buffer->fake_id = fake_id;
    pthread_t p;
    this->waiting_thieves_counter++;
    this->map_adders_counter ++;
    safePrint("Thief id No " + std::to_string(fake_id)+ " "+ " time: " + std::to_string(my_time++));
    if(pthread_create(&p,NULL,thiefThreadWrapper,buffer) != 0){
        this->waiting_thieves_counter--;
        this->map_adders_counter --;
    }
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
    safePrint("finish thief No " + std::to_string(fake_id) +" waiting to map "+ " time: " + std::to_string(my_time++));
    remover_lock_map();
    pthread_t p = this->getThreadIDMap(fake_id);
    this->removeFromMap(fake_id);
    unlock_map();
    safePrint("finish thief No " + std::to_string(fake_id) +" updated map "+ " time: " + std::to_string(my_time++));

    void *ret_val;
    pthread_join(p,&ret_val);
    safePrint("finish thief No " + std::to_string(fake_id) +" finish waiting to thread "+ " time: " + std::to_string(my_time++));
    assert(ret_val != NULL);
    int number_of_stolen = *(int*)ret_val;
    delete (int*)ret_val;
    return number_of_stolen;
}

void Factory::closeFactory(){
    this->is_open = false;
}

void Factory::openFactory() {
    this->is_open = true;
    if (this->number_of_resource_writers == 0 &&
        this->number_of_resource_readers == 0) {
        pthread_cond_broadcast(&(this->cond_read));
        pthread_cond_signal(&(this->cond_factory_produce));
        if (this->waiting_thieves_counter > 0) {
            pthread_cond_signal(&(this->cond_thief));
            return;
        }
        pthread_cond_broadcast(&(this->cond_no_thief));
    }
}

void Factory::closeReturningService(){
    this->is_return = false;
}

void Factory::openReturningService() {
    this->is_return = true;
    if (this->number_of_resource_writers == 0 &&
        this->number_of_resource_readers == 0) {
        pthread_cond_broadcast(&(this->cond_read));
        pthread_cond_signal(&(this->cond_factory_produce));
        if (this->is_open) {
            if (this->waiting_thieves_counter > 0) {
                pthread_cond_signal(&(this->cond_thief));
                return;
            }
            pthread_cond_broadcast(&(this->cond_no_thief));
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
            pthread_cond_broadcast(&(this->cond_no_thief));
        }
    }
    pthread_mutex_unlock(&(this->mutex_general_factory));
}

void Factory::write_lock_thieves() {
    pthread_mutex_lock(&(this->mutex_general_factory));
    while(!this->is_open||this->number_of_resource_writers > 0 || this->number_of_resource_readers > 0){
        pthread_cond_wait(&(this->cond_thief),&(this->mutex_general_factory));
    }
    this->waiting_thieves_counter--;//thief enter the factory
    this->number_of_resource_writers++;//thief catch the factory
    pthread_mutex_unlock(&(this->mutex_general_factory));
}

void Factory::write_lock_company(int num_products, bool want_to_return) {
    pthread_mutex_lock(&(this->mutex_general_factory));
    this->waiting_companies_counter++;
    while(!this->is_open || (this->waiting_thieves_counter > 0) || (num_products >(int)this->_listAvailableProducts.size())
          || (want_to_return && !(this->is_return)) ||(this->number_of_resource_writers > 0) || (this->number_of_resource_readers > 0)){

        pthread_cond_wait(&(this->cond_no_thief),&(this->mutex_general_factory));
    }
    this->waiting_companies_counter--;
    this->number_of_resource_writers++;//company enter the factory for second time
    pthread_mutex_unlock(&(this->mutex_general_factory));
}


int Factory::write_lock_buyer() {
    if(pthread_mutex_trylock(&(this->mutex_general_factory)) != 0){
        return -1;
    }
    if(!this->is_open ||this->number_of_resource_writers > 0 || this->number_of_resource_readers > 0){
        pthread_mutex_unlock(&(this->mutex_general_factory));
        return -1;
    }
    this->number_of_resource_writers++;//buyer catch the factory
    pthread_mutex_unlock(&(this->mutex_general_factory));
    return 0;
}

void Factory::write_lock_factory_produce() {
    pthread_mutex_lock(&(this->mutex_general_factory));
    while(this->number_of_resource_writers > 0 || this->number_of_resource_readers > 0){
        pthread_cond_wait(&(this->cond_factory_produce),&(this->mutex_general_factory));
    }
    this->number_of_resource_writers++;//production begin
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
            pthread_cond_broadcast(&(this->cond_no_thief));
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
    std::map<int,pthread_t>::iterator it = this->_id2threadMAP.find(id);
    assert(it != this->_id2threadMAP.end());

    return (*(it)).second;
}

void Factory::adder_lock_map() {
    pthread_mutex_lock(&(this->mutex_map));
    while(this->busy_map){
        pthread_cond_wait(&(this->cond_map_adders),&(this->mutex_map));
    }
    this->map_adders_counter--;
    this->busy_map = true;
    pthread_mutex_unlock(&(this->mutex_map));
}

void Factory::remover_lock_map() {
    pthread_mutex_lock(&(this->mutex_map));
    while(this->busy_map || this->map_adders_counter > 0){
        pthread_cond_wait(&(this->cond_map_remover),&(this->mutex_map));
    }
    this->busy_map = true;
    pthread_mutex_unlock(&(this->mutex_map));
}

void Factory::unlock_map() {
    pthread_mutex_lock(&(this->mutex_map));
    this->busy_map = false;
    if (this->map_adders_counter > 0) {
        pthread_cond_signal(&this->cond_map_adders);
        pthread_mutex_unlock(&(this->mutex_map));
        return;
    }
    pthread_cond_signal(&this->cond_map_remover);
    pthread_mutex_unlock(&(this->mutex_map));
    return;
}

std::list<Product> Factory::buyProducts(int num_products) {
    write_lock_company(num_products, false);
    assert(num_products <=(int) _listAvailableProducts.size());
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
