#include "id_t.h"

template<> std::filesystem::path fileParamethers<shaderId_t>::directory = "./shaders/";
template<> std::string fileParamethers<shaderId_t>::extension = ".json";
template<> std::filesystem::path fileParamethers<materialId_t>::directory = "./models/materials/";
template<> std::string fileParamethers<materialId_t>::extension = ".json";
template<> std::filesystem::path fileParamethers<objectId_t>::directory = "./models/objects/";
template<> std::string fileParamethers<objectId_t>::extension = ".obj";
template<> std::filesystem::path fileParamethers<modelId_t>::directory = "./models/";
template<> std::string fileParamethers<modelId_t>::extension = ".json";
