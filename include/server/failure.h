#pragma once

#include <boost/beast.hpp>
#include <iostream>

class failure {
public:
  void static handle(boost::beast::error_code ec, char const* what);
};