// Variant.hh
//
// Copyright (C) 2001 - 2008, 2012 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef VARIANT_HH
#define VARIANT_HH

#include "debug.hh"

#include <string>
using namespace std;

enum VariantType { VARIANT_TYPE_NONE,
                   VARIANT_TYPE_INT,
                   VARIANT_TYPE_LONG,
                   VARIANT_TYPE_BOOL,
                   VARIANT_TYPE_DOUBLE,
                   VARIANT_TYPE_STRING };


class Variant
{
public:
  Variant()
  {
  }

  Variant(std::string v)
  {
    type = VARIANT_TYPE_STRING;
    string_value = v;
  }

  Variant(int v)
  {
    type = VARIANT_TYPE_INT;
    int_value = v;
  }

  Variant(long v)
  {
    type = VARIANT_TYPE_LONG;
    int_value = v;
  }

  Variant(bool v)
  {
    type = VARIANT_TYPE_BOOL;
    bool_value = v;
  }

  Variant(double v)
  {
    type = VARIANT_TYPE_DOUBLE;
    double_value = v;
  }

  Variant(const Variant &rhs)
  {
    type = rhs.type;
    switch(rhs.type)
      {
      case VARIANT_TYPE_INT:
        int_value = rhs.int_value;
        break;

      case VARIANT_TYPE_LONG:
        long_value = rhs.long_value;
        break;

      case VARIANT_TYPE_BOOL:
        bool_value = rhs.bool_value;
        break;

      case VARIANT_TYPE_DOUBLE:
        double_value = rhs.double_value;
        break;

      case VARIANT_TYPE_STRING:
        string_value = rhs.string_value;
        break;

      case VARIANT_TYPE_NONE:
      default:
        break;
      }
  }

  Variant& operator=(const Variant &lid)
  {
    if (this != &lid)
      {
        type = lid.type;
        switch(lid.type)
          {
          case VARIANT_TYPE_INT:
            int_value = lid.int_value;
            break;

          case VARIANT_TYPE_LONG:
            long_value = lid.long_value;
            break;

          case VARIANT_TYPE_BOOL:
            bool_value = lid.bool_value;
            break;

          case VARIANT_TYPE_DOUBLE:
            double_value = lid.double_value;
            break;

          case VARIANT_TYPE_STRING:
            string_value = lid.string_value;
            break;

          case VARIANT_TYPE_NONE:
          default:
            break;
          }
      }
    return *this;
  }

  bool operator!=(const Variant &lid)
  {
    return !operator==(lid);
  }
  
  bool operator==(const Variant &lid)
  {
    if (type != lid.type)
      {
        return false;
      }
    
    switch(lid.type)
      {
      case VARIANT_TYPE_INT:
        return int_value == lid.int_value;
        break;
        
      case VARIANT_TYPE_LONG:
        return long_value == lid.long_value;
        break;
        
      case VARIANT_TYPE_BOOL:
        return bool_value == lid.bool_value;
        break;
        
      case VARIANT_TYPE_DOUBLE:
        return double_value == lid.double_value;
        break;
        
      case VARIANT_TYPE_STRING:
        return string_value == lid.string_value;
        break;
        
      case VARIANT_TYPE_NONE:
      default:
        return false;
      }
  }
  
  virtual ~Variant()
  {
    type = VARIANT_TYPE_NONE;
  }

  VariantType get_type()
  {
    return type;
  }

  //private:
  VariantType type;

  std::string string_value;
  union
  {
    int int_value;
    long long_value;
    bool bool_value;
    double double_value;
  };
};

#endif // VARIANT_HH
