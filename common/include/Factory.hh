// Factory.hh --- Factory
//
// Copyright (C) 2007 Rob Caelers <robc@krandor.nl>
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
// Based on ideas from libs11n and boost
//

#ifndef FACTORY_HH
#define FACTORY_HH

#include <string>
#include <map>

#include "ISerializable.hh"

namespace workrave {

  namespace serialization {

//     template <typename Type>
//     struct traits
//     {
//       static const std::string class_name(const Type * = 0)
//       {
//         return "unknown";
//       }
//     };

    template <class T, class SubT>
    class create_object
    {
    public:
      static T *create()
      {
        return new SubT;
      }
    };


    template <class T, class SubT>
    class create_abstract
    {
    public:
      static T *create()
      {
        return NULL;
      }
    };

    class abstract_class: public ISerializable
    {
      virtual std::string str() const
      {
        return "[abstract]";
      }

      virtual void serialize(Target *)
      {
      }
    };

    template <class Type>
    class Factory
    {
    public:

      Factory() {}
      virtual ~Factory() {}

      typedef Type * (*FactoryCreator)();
      typedef std::map<std::string, FactoryCreator> FactoryCreatorMap;

      virtual Type *create(const std::string &key)
      {
        typename FactoryCreatorMap::const_iterator it = map.find(key);
        if (it != map.end())
          {
            return it->second();
          }
        return NULL;
      }

      virtual bool add_type(const std::string &key, FactoryCreator fc)
      {
        map.insert(typename FactoryCreatorMap::value_type(key, fc));
        return true;
      }

      static Factory<Type> &instance()
      {
        static Factory<Type> *me = NULL;

        if (me == NULL)
          {
            me = new Factory<Type>;
          }

        return *me;
      }

    private:
      FactoryCreatorMap map;
    };

    template <typename T>
    struct AbstractFactory
    {
      static bool create(T &value,
                         const std::string &name = std::string())
      {
        try
          {
            value = T();
          }
        catch(...)
          {
            return false;
          }
        return true;
      }

    };

    template <typename T>
    struct AbstractFactory<T *>
    {
      static bool create(T *&value,
                         const std::string &name)
      {
        try
          {
            value = Factory<T>::instance().create(name);
          }
        catch(...)
          {
            return false;
          }
        return NULL != &value;
      }
    };


    template <typename Type>
    struct Global
    {
      Global()
      {
        global = false;
      }

      static bool global;
    };

  }
}

#define REGISTER_TYPE(BaseType, Type, Name) \
template<> \
 bool workrave::serialization::Global<Type>::global = workrave::serialization::Factory<BaseType>::instance().add_type(std::string(Name), workrave::serialization::create_object<BaseType,Type>::create); \
 std::string Type::class_name() const                                   \
 {                                                                      \
   return Name;                                                         \
 }                                                                      \



#define REGISTER_ABSTRACT_TYPE(BaseType, Name)  \
  template<>                                                            \
  bool workrave::serialization::Global<BaseType>::global = workrave::serialization::Factory<BaseType>::instance().add_type(std::string(Name), workrave::serialization::create_abstract<BaseType,BaseType>::create); \
  std::string BaseType::class_name() const                              \
  {                                                                     \
    return Name;                                                        \
  }                                                                     \

//   namespace workrave {
//     namespace serialization {
//       template <>
//       struct traits<BaseType>
//       {
//         typedef workrave::serialization::abstract_class base_class;
//         static const std::string class_name(const BaseType * = 0)
//         {
//           return Name;
//         }
//       };
//     }
//   }

#endif // FACTORY_HH
