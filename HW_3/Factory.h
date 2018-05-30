#ifndef FACTORY_H_
#define FACTORY_H_

#include <pthread.h>
#include <list>
#include <map>
#include "Product.h"

class Factory {
public:
    std::list<std::pair<Product, int>> _lStolenProducts_;
    int _numberOfStolenWriters_;
    int _numberOfStolenReaders_;
    pthread_cond_t _cond_StolenReaders_;
    pthread_cond_t _cond_StolenWriters_;
    pthread_mutex_t _mutex_Stolen_;



    bool _factoryIsOpen_;
    bool _factoryIsReturnSer_;

    int _numberOfFactoryWriters_;
    int _numberOfFactoryReaders_;
    pthread_cond_t _cond_AvailableReaders_;
    std::list<Product> _lAvailableProducts_;

    pthread_cond_t _cond_FactoryProduce_;

    int _counterWaitingThievs_;
    pthread_cond_t _cond_Thievs_;

    int _counterWaitingCompanies_;//Not necessary ONLY for DEBUG
    pthread_cond_t _cond_CompaniesBuy_;
    pthread_cond_t _cond_CompaniesReturn_;
    pthread_mutex_t _mutex_Factory_;

    std::map<int, pthread_t> _mapProduce_;
    std::map<int, pthread_t> _mapBuyer_;
    std::map<int, pthread_t> _mapCompanies_;
    std::map<int, pthread_t> _mapThieves_;

    void _writeStolenLockFactory_();

    void _writeStolenUnlockFactory_();

    void _readAvailableLockFactory_();

    void _readStolenLockFactory_();

    void _readAvailableUnlockFactory_();

    void _readStolenUnlockFactory_();

    void _produceLockFactory_();

    void _thiefLockFactory_();

    void _companyBuyLockFactory_(int num_products);

    void _companyReturnLockFactory_();

    int _buyerLockFactory_();

    void _writersUnlock_();

    void _callWaitingCond_();


public:
    void insertProduceIDToMap(int id, pthread_t p);

    pthread_t removeProduceIDFromMap(int id);

    void insertThiefIDToMap(int id, pthread_t p);

    pthread_t removeThiefIDFromMap(int id);

    void insertComapnyIDToMap(int id, pthread_t p);

    pthread_t removeCompanyIDFromMap(int id);

    void insertBuyerIDToMap(int id, pthread_t p);

    pthread_t removeBuyerIDFromMap(int id);

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
