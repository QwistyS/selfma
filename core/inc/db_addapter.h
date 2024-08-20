#ifndef SELFMA_DB_ADDAPTER_H
#define SELFMA_DB_ADDAPTER_H

#include "error_handler.h"
#include <string_view>


// concept
/*   Template.
template <typename T>
concept IReceiverConcept =
    requires(T t,  parrams ) {
        { t.func() } -> std::same_as<bool>;
    };
*/

// template<typename T>
// concept IDbApi = request(T t,std::string_view path, ) {
//                              {t.connect(path)} -> VoidResult;
//                          }

// Interface

class IDbApi {
    public:
        virtual ~IDbApi();
        virtual VoidResult connect(std::string_view path) = 0;
        
};

class DbWrapper : public IDbApi {
    DbWrapper() {};
    ~DbWrapper() = default;
    VoidResult connect(std::string_view pat) override;
};
#endif // SELFMA_DB_ADDAPTER_H
