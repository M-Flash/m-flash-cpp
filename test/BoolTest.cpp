#include <cstring>
#include <string>
#include <vector>
#include <iostream>


template <class IdType, bool inSplit>
class Splitter{
  public:
   IdType getId(IdType in, IdType out);
};

template <class IdType, bool inSplit>
IdType Splitter<IdType, inSplit>::getId(IdType in, IdType out){
#if inSplit
    return in>>2;
#else
    return out>>2;
#endif
};

int main(){
    Splitter<int, true> splitter1;
    Splitter<int, false> splitter2;
    std::cout << splitter1.getId(4, 8) <<std::endl;
    std::cout << splitter2.getId(4, 8) <<std::endl;
    return 0;
}

