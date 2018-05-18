#include "Factory.h"

Factory::Factory(){
}

Factory::~Factory(){
}

void Factory::startProduction(int num_products, Product* products,unsigned int id){
}

void Factory::produce(int num_products, Product* products){
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
}

std::list<Product> Factory::buyProducts(int num_products){
    return std::list<Product>();
}

void Factory::returnProducts(std::list<Product> products,unsigned int id){
}

int Factory::finishCompanyBuyer(unsigned int id){
    return 0;
}

void Factory::startThief(int num_products,unsigned int fake_id){
}

int Factory::stealProducts(int num_products,unsigned int fake_id){
    return 0;
}

int Factory::finishThief(unsigned int fake_id){
    return 0;
}

void Factory::closeFactory(){
}

void Factory::openFactory(){
}

void Factory::closeReturningService(){
}

void Factory::openReturningService(){
}

std::list<std::pair<Product, int>> Factory::listStolenProducts(){
    return std::list<std::pair<Product, int>>();
}

std::list<Product> Factory::listAvailableProducts(){
    return std::list<Product>();
}
