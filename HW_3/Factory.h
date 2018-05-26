#ifndef FACTORY_H_
#define FACTORY_H_

#include <pthread.h>
#include <list>
#include <map>
#include "Product.h"

class Factory {
    std::list<std::pair<Product, int>> _lStolenProducts_;
    std::list<Product> _lAvailableProducts_;

    bool _factoryIsOpen_;
    bool _factoryIsReturnSer_;
    pthread_cond_t _cond_FactoryProduce_;

    int _numberOfFactoryWriters_;
    int _numberOfFactoryReaders_;
    pthread_cond_t _cond_Readers_;

    int _counterWaitingThievs_;
    pthread_cond_t _cond_Thievs_;

    int _counterWaitingCompanies_;//Not necessary ONLY for DEBUG
    pthread_cond_t _cond_Companies_;
    pthread_mutex_t _mutex_Factory_;

    std::map<int, pthread_t> _mapProduce_;
    bool _busyMapProduce_;
    int _mapProduceAddersCounter_;
    pthread_cond_t _cond_mapPruduceAdders_;
    pthread_cond_t _cond_mapProduceRemovers_;
    pthread_mutex_t _mutex_ProduceMap_;

    std::map<int, pthread_t> _mapThieves_;
    bool _busyMapThieves_;
    int _mapThievesAddersCounter_;
    pthread_cond_t _cond_mapThievesAdders_;
    pthread_cond_t _cond_mapThievesRemovers_;
    pthread_mutex_t _mutex_ThievesMap_;

    std::map<int, pthread_t> _mapCompanies_;
    bool _busyMapCompanies_;
    int _mapCompaniesAddersCounter_;
    pthread_cond_t _cond_mapCompaniesAdders_;
    pthread_cond_t _cond_mapCompaniesRemovers_;
    pthread_mutex_t _mutex_CompaniesMap_;

    std::map<int, pthread_t> _mapBuyer_;
    bool _busyMapBuyer_;
    int _mapBuyerAddersCounter_;
    pthread_cond_t _cond_mapBuyerAdders_;
    pthread_cond_t _cond_mapBuyerRemovers_;
    pthread_mutex_t _mutex_BuyerMap_;


    void _readLockFactory_();
    void _readUnlockFactory_();

    void _produceLockFactory_();
    void _thiefLockFactory_();
    void _companyLockFactory_(int num_products, bool return_service);
    int _buyerLockFactory_();
    void _writersUnlock_();
    void _callCondByPrio_();


public:
    void addProduceThreadLockMap();
    void removeProduceThreadLockMap();
    void produceMapUnlock();

    void addThiefThreadLockMap();
    void removeThiefThreadLockMap();
    void thievsMapUnlock();

    void addCompanyThreadLockMap();
    void removeCompanyThreadlockMap();
    void companiesMapUnlock();

    void addBuyerThreadLockMap();
    void removeBuyerThreadlockMap();
    void buyersMapUnlock();

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
    };
#endif // FACTORY_H_
