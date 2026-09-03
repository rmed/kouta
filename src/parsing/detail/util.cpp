#include "util.hpp"

namespace kouta::parsing::util
{
    boost::endian::order convert_order(std::endian order)
    {
        switch (order)
        {
        case std::endian::big:
            return boost::endian::order::big;
        case std::endian::little:
            return boost::endian::order::little;
        default:
            return boost::endian::order::native;
        }
    }
}  // namespace kouta::parsing::util
