#include "Factory.h"
#include <cassert>

#define NO_INIT -1

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
            factory_pointer(nullptr),
            stl_pointer(nullptr),
            product_arr(nullptr),
            num_products(NO_INIT),
            id(NO_INIT),
            fake_id(NO_INIT),
            min_value(NO_INIT) {}
};

static void* _produceThreadWrapper_(void *args) {
    Buffer* buffer = (Buffer*)args;

    buffer->factory_pointer->produce(buffer->num_products,buffer->product_arr);
    delete buffer;
    pthread_exit(nullptr);
}

static void *buyerThreadWrapper(void * args) {
    Buffer* buffer = (Buffer*)args;

    int *procuct_id = new int;//transfer id product to finish function
    *procuct_id = buffer->factory_pointer->tryBuyOne();
    delete buffer;
    pthread_exit(procuct_id);
}
static void *companyThreadWrapper(void * args) {
    Buffer* buffer = (Buffer*)args;

    std::list<Product> company_products = buffer->factory_pointer->buyProducts(buffer->num_products);
    std::list<Product> returned_products;
    for(std::list<Product>::iterator it = company_products.begin(), end = company_products.end(); it != end; ++it){
        if((*it).getValue() < buffer->min_value){
            returned_products.push_back((*it));
        }
    }
    int *number_of_returned = new int;
    *number_of_returned = (int) returned_products.size();
    if((*number_of_returned) > 0) {
        buffer->factory_pointer->returnProducts(returned_products,0);
    }
    delete buffer;
    pthread_exit(number_of_returned);
}

static void *thiefThreadWrapper(void * args) {
    Buffer* buffer = (Buffer*)args;

    int *number_of_stolen = new int;//transfer number od stolen products to finish function
    *number_of_stolen = buffer->factory_pointer->stealProducts(buffer->num_products,(unsigned)buffer->fake_id);
    delete buffer;
    pthread_exit(number_of_stolen);
}

Factory::Factory(){

    /*init factory variable*/
    this->_factoryIsOpen_ = true;
    this->_factoryIsReturnSer_ = true;
    pthread_cond_init(&(this->_cond_FactoryProduce_),nullptr);

    this->_numberOfFactoryWriters_ = 0;
    this->_numberOfFactoryReaders_ = 0;
    pthread_cond_init(&(this->_cond_Readers_),nullptr);

    this->_counterWaitingThievs_ = 0;
    pthread_cond_init(&(this->_cond_Thievs_),nullptr);

    this-> _counterWaitingCompanies_ = 0;
    pthread_cond_init(&(this->_cond_Companies_),nullptr);

    pthread_mutex_init(&(this->_mutex_Factory_), nullptr);
    }

Factory::~Factory() {
    /*destroy threads env of factory*/
    pthread_cond_destroy(&(this->_cond_FactoryProduce_));
    pthread_cond_destroy(&(this->_cond_Readers_));
    pthread_cond_destroy(&(this->_cond_Thievs_));
    pthread_cond_destroy(&(this->_cond_Companies_));
    pthread_mutex_destroy(&(this->_mutex_Factory_));
}
/*!!PRODUCE SECTION START!!*/
void Factory::startProduction(int num_products, Product* products,unsigned int id) {
    assert(num_products > 0);
    assert(products != nullptr);

    pthread_t p;
    Buffer *buffer = new Buffer();//buffer to transfer args to thread
    buffer->factory_pointer = this;
    buffer->num_products = num_products;
    buffer->product_arr = products;
    buffer->id = id;
    pthread_create(&p, nullptr, _produceThreadWrapper_, buffer);
	this->insertProduceIDToMap(id,p);
}

void Factory::produce(int num_products, Product* products){
    _produceLockFactory_();
    for(int i = 0; i < num_products; i++){
        this->_lAvailableProducts_.push_back(products[i]);
    }
    _writersUnlock_();
}

void Factory::finishProduction(unsigned int id){
    pthread_t p = this->removeProduceIDFromMap(id);
    pthread_join(p,nullptr);
}
/*!!PRODUCE SECTION END!!*/

/*!!SINGLE BUYER SECTION START!!*/
void Factory::startSimpleBuyer(unsigned int id){
    pthread_t p;
    Buffer *buffer = new Buffer();//buffer to transfer args to thread
    buffer->factory_pointer = this;
    buffer->id = id;
    pthread_create(&p,nullptr,buyerThreadWrapper,buffer);
	this->insertBuyerIDToMap(id,p);
}

int Factory::tryBuyOne(){
    if(_buyerLockFactory_() == -1) {
        return -1;
    }
    if(this->_lAvailableProducts_.empty()){
        _writersUnlock_();
        return -1;
    }
    int product_id = this->_lAvailableProducts_.front().getId();
    this->_lAvailableProducts_.pop_front();
    _writersUnlock_();
    return product_id;
}

int Factory::finishSimpleBuyer(unsigned int id){
    pthread_t p = this->removeBuyerIDFromMap(id);
    void *ret_val = nullptr;
    pthread_join(p,&ret_val);
    assert(ret_val != nullptr);
    int return_id = *(int*)ret_val;
    delete (int*)ret_val;
    return return_id;
}
/*!!SINGLE BUYER SECTION END!!*/

/*!!COMPANY SECTION START!!*/
void Factory::startCompanyBuyer(int num_products, int min_value,unsigned int id) {
    assert(num_products > 0);
    pthread_t p;
    Buffer *buffer = new Buffer();//buffer to transfer args to thread
    buffer->factory_pointer = this;
    buffer->num_products = num_products;
    buffer->min_value = min_value;
    buffer->id = id;
    pthread_create(&p, nullptr, companyThreadWrapper, buffer);
	this->insertComapnyIDToMap(id,p);
}

std::list<Product> Factory::buyProducts(int num_products) {
    _companyLockFactory_(num_products, false);//sending false because company NOT return products
    assert(num_products <= (int) _lAvailableProducts_.size());
    std::list<Product>::iterator it = this->_lAvailableProducts_.begin();
    std::list<Product> company_products = std::list<Product>();
    for(int i = 0; i < num_products ; i++){
        company_products.push_back(*(it));//copy the product to company
        this->_lAvailableProducts_.pop_front();//pop the product
        it = this->_lAvailableProducts_.begin();//reinitialize iterator
    }
    _writersUnlock_();
    return company_products;
}

void Factory::returnProducts(std::list<Product> products,unsigned int id){
    _companyLockFactory_(NO_INIT, true);// sending negative number to remove dependency in num_product
    for(std::list<Product>::iterator it = products.begin(), end = products.end(); it != end; ++it){
        this->_lAvailableProducts_.push_back(*it);
    }
    _writersUnlock_();
}

int Factory::finishCompanyBuyer(unsigned int id){
    pthread_t p = this->removeCompanyIDFromMap(id);
    void *ret_val = nullptr;
    pthread_join(p,&ret_val);
    assert(ret_val != nullptr);
    int number_of_returns = *(int*)ret_val;
    delete (int*)ret_val;
    return number_of_returns;
}
/*!!COMPANY SECTION END!!*/

/*!!THIEF SECTION START!!*/
void Factory::startThief(int num_products,unsigned int fake_id){
    assert(num_products > 0);
	pthread_t p;
    Buffer *buffer = new Buffer();//buffer to transfer args to thread
    buffer->factory_pointer = this;
    buffer->num_products = num_products;
    buffer->fake_id = fake_id;
    pthread_mutex_lock(&(this->_mutex_Factory_));
	this->_counterWaitingThievs_++;
    pthread_mutex_unlock(&(this->_mutex_Factory_));
    pthread_create(&p,nullptr,thiefThreadWrapper,buffer);
	this->insertThiefIDToMap(fake_id,p);
}

int Factory::stealProducts(int num_products,unsigned int fake_id){
    this->_thiefLockFactory_();
    int list_size =(int)this->_lAvailableProducts_.size();
    int min = (num_products > list_size) ? list_size : num_products;
    std::list<Product>::iterator it = this->_lAvailableProducts_.begin();
    for(int i = 0; i < min ; i++){
        this->_lStolenProducts_.push_back(std::pair<Product,int>(*(it),fake_id));
        this->_lAvailableProducts_.pop_front();
        it = this->_lAvailableProducts_.begin();
    }
    this->_writersUnlock_();
    return min;
}

int Factory::finishThief(unsigned int fake_id){
    pthread_t p = this->removeThiefIDFromMap(fake_id);
    void *ret_val = nullptr;
    pthread_join(p,&ret_val);
    assert(ret_val != nullptr);
    int number_of_stolen = *(int*)ret_val;
    delete (int*)ret_val;
    return number_of_stolen;
}
/*!!THIEF SECTION END!!*/

/*!!READERS SECTION START!!*/
std::list<std::pair<Product, int>> Factory::listStolenProducts(){
    _readLockFactory_();
    std::list<std::pair<Product, int>> copy = this->_lStolenProducts_;
    _readUnlockFactory_();
    return copy;
}

std::list<Product> Factory::listAvailableProducts(){
    _readLockFactory_();
    std::list<Product> copy = this->_lAvailableProducts_;
    _readUnlockFactory_();
    return copy;
}
/*!!READERS SECTION END!!*/

/*!!FACTORY CONTROL SECTION START!!*/
void Factory::_callWaitingCond_() {
    pthread_cond_broadcast(&(this->_cond_Readers_));
    pthread_cond_broadcast(&(this->_cond_FactoryProduce_));
    pthread_cond_broadcast(&(this->_cond_Thievs_));
    pthread_cond_broadcast(&(this->_cond_Companies_));
}


void Factory::closeFactory(){
    //assuming all other thief, company, buyers threads terminated
    pthread_mutex_lock(&(this->_mutex_Factory_));
    this->_factoryIsOpen_ = false;
    this->_callWaitingCond_();
    pthread_mutex_unlock(&(this->_mutex_Factory_));

}

void Factory::openFactory() {
    pthread_mutex_lock(&(this->_mutex_Factory_));
    this->_factoryIsOpen_ = true;
    this->_callWaitingCond_();
    pthread_mutex_unlock(&(this->_mutex_Factory_));

}

void Factory::closeReturningService(){
    //assuming all other company threads terminated
    pthread_mutex_lock(&(this->_mutex_Factory_));
    this->_factoryIsReturnSer_ = false;
    this->_callWaitingCond_();
    pthread_mutex_unlock(&(this->_mutex_Factory_));
}

void Factory::openReturningService() {
    pthread_mutex_lock(&(this->_mutex_Factory_));
    this->_factoryIsReturnSer_ = true;
    this->_callWaitingCond_();
    pthread_mutex_unlock(&(this->_mutex_Factory_));
}

void Factory::insertProduceIDToMap(int id, pthread_t p) {
    this->_mapProduce_.insert(std::pair<int,pthread_t>(id,p));
}

pthread_t Factory::removeProduceIDFromMap(int id) {
    std::map<int,pthread_t>::iterator it = this->_mapProduce_.find(id);
    assert(it != this->_mapProduce_.end());
    pthread_t produce_thread_id = (*it).second;
    this->_mapProduce_.erase(id);
    return produce_thread_id;
}

void Factory::insertThiefIDToMap(int id, pthread_t p) {
    this->_mapThieves_.insert(std::pair<int,pthread_t>(id,p));
}

pthread_t Factory::removeThiefIDFromMap(int id){
    std::map<int,pthread_t>::iterator it = this->_mapThieves_.find(id);
    assert(it != this->_mapThieves_.end());
    pthread_t thief_thread_id = (*it).second;
    this->_mapThieves_.erase(id);
    return thief_thread_id;
}

void Factory::insertComapnyIDToMap(int id, pthread_t p) {
    this->_mapCompanies_.insert(std::pair<int,pthread_t>(id,p));
}

pthread_t Factory::removeCompanyIDFromMap(int id) {
    std::map<int,pthread_t>::iterator it = this->_mapCompanies_.find(id);
    assert(it != this->_mapCompanies_.end());
    pthread_t company_thread_id = (*it).second;
    this->_mapCompanies_.erase(id);
    return company_thread_id;
}

void Factory::insertBuyerIDToMap(int id, pthread_t p) {
    this->_mapBuyer_.insert(std::pair<int,pthread_t>(id,p));
}

pthread_t Factory::removeBuyerIDFromMap(int id) {
    std::map<int, pthread_t>::iterator it = this->_mapBuyer_.find(id);
    assert(it != this->_mapBuyer_.end());
    pthread_t buyer_thread_id = (*it).second;
    this->_mapBuyer_.erase(id);
    return buyer_thread_id;
}
/*!!FACTORY CONTROL SECTION END!!*/

/*!!FACTORY MUTEX SECTION START!!*/
void Factory::_readLockFactory_() {
    pthread_mutex_lock(&(this->_mutex_Factory_));
    while(this->_numberOfFactoryWriters_ > 0){
        pthread_cond_wait(&(this->_cond_Readers_),&(this->_mutex_Factory_));
    }
    this->_numberOfFactoryReaders_++;
    pthread_mutex_unlock(&(this->_mutex_Factory_));
}

void Factory::_readUnlockFactory_() {
    pthread_mutex_lock(&(this->_mutex_Factory_));
    this->_numberOfFactoryReaders_--;
    this->_callWaitingCond_();
    pthread_mutex_unlock(&(this->_mutex_Factory_));
}

void Factory::_produceLockFactory_() {
    pthread_mutex_lock(&(this->_mutex_Factory_));
    while(this->_numberOfFactoryWriters_ > 0 || this->_numberOfFactoryReaders_ > 0){
        pthread_cond_wait(&(this->_cond_FactoryProduce_),&(this->_mutex_Factory_));
    }
    this->_numberOfFactoryWriters_++;//production begin
    pthread_mutex_unlock(&(this->_mutex_Factory_));
}

void Factory::_thiefLockFactory_() {
    pthread_mutex_lock(&(this->_mutex_Factory_));
    while(!this->_factoryIsOpen_|| this->_numberOfFactoryWriters_ > 0 || this->_numberOfFactoryReaders_ > 0) {
        pthread_cond_wait(&(this->_cond_Thievs_), &(this->_mutex_Factory_));
    }
    this->_numberOfFactoryWriters_++;//thief write to factory
    this->_counterWaitingThievs_--;//thief enter the factory stop waiting
    pthread_mutex_unlock(&(this->_mutex_Factory_));
}

void Factory::_companyLockFactory_(int num_products, bool want_to_return) {
    pthread_mutex_lock(&(this->_mutex_Factory_));
    this->_counterWaitingCompanies_++;
    while(!this->_factoryIsOpen_ || this->_counterWaitingThievs_ > 0 || (num_products >(int)this->_lAvailableProducts_.size())
          || (want_to_return && !(this->_factoryIsReturnSer_)) ||
          (this->_numberOfFactoryWriters_ > 0) || (this->_numberOfFactoryReaders_ > 0)){
        pthread_cond_wait(&(this->_cond_Companies_),&(this->_mutex_Factory_));
    }
    this->_counterWaitingCompanies_--;//company enter the factory
    this->_numberOfFactoryWriters_++;//company write to factory
    pthread_mutex_unlock(&(this->_mutex_Factory_));
}

int Factory::_buyerLockFactory_() {
    if(pthread_mutex_trylock(&(this->_mutex_Factory_)) != 0){
        return -1;
    }
    if(!this->_factoryIsOpen_ || this->_numberOfFactoryWriters_ > 0 || this->_numberOfFactoryReaders_ > 0){
        pthread_mutex_unlock(&(this->_mutex_Factory_));
        return -1;
    }
    this->_numberOfFactoryWriters_++;//buyer write to factory
    pthread_mutex_unlock(&(this->_mutex_Factory_));
    return 0;
}

void Factory::_writersUnlock_() {
    pthread_mutex_lock(&(this->_mutex_Factory_));
    this->_numberOfFactoryWriters_--;
    this->_callWaitingCond_();
    pthread_mutex_unlock(&(this->_mutex_Factory_));
}
/*!!FACTORY MUTEX SECTION END!!*/
