

#include "../src/mflash_basic_includes.hpp"

using namespace mflash;

template <class V, class IdType>
class PrintVector: public ZeroOperator<V,IdType>{
    public:
        inline void apply(Element<V, IdType> &out){
            std::cout<< out.id << " = "<< *(out.value) << std::endl;
        }
};

int main(int argc, char* argv[]){
    mflash_init(argc, argv);

    /* Parameters */
    std::string filename    = get_option_string("file"); // Base filename

    PrintVector<float, int32> printOperator;

    PrimitiveVector<float, int32> pvector(filename, 12);
    pvector.operate(printOperator);

    return 0;
}
