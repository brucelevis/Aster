#include "utils.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace Utils
{
  std::string UUID()
  {
    return boost::uuids::to_string(boost::uuids::random_generator{}());
  }
}