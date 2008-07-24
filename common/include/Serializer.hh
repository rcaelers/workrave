// Serializer.hh --- Serializer
//
// Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.nl>
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

#ifndef SERIALIZER_HH
#define SERIALIZER_HH

#include <list>
#include <string>
#include <vector>
#include <glib.h>

#include "IArchive.hh"
#include "Factory.hh"

namespace workrave {

  namespace serialization {

    template<class T>
    struct attribute
    {
      explicit attribute(const char *name, T &ref, int version = 0)
        : name(name), ref(&ref), version(version)
      {
      }

      attribute(const attribute &rhs) :
        name(rhs.name), ref(rhs.ref), version(rhs.version)
      {
      }

      const std::string get_name() const
      {
        return name.c_str();
      }

      int get_version() const
      {
        return version;
      }

      T &get_value() const
      {
        return *ref;
      }

      const T &get_const_value() const
      {
        return *ref;
      }

    private:
      const std::string name;
      T *ref;
      const int version;
    };

    template<class T>
    inline const
    attribute<T> attr(const char * name, T &t,  int version = 0)
    {
      return attribute<T>(name, t, version);
    }


    class Target
    {
    public:
      enum Stage
        { SAVING,
          LOADING,
        };

      inline bool is_saving() const
      {
        return stage == SAVING;
      }

      template <typename T>
      Target & operator&(attribute<T> a)
      {
        name = a.get_name();
        version = a.get_version();

        this->operator&(a.get_value());

        reset();
        return *this;
      }

      Target & operator&(UUID &id)
      {
        if (is_saving())
          {
            archive->add_primitive(name, id);
          }
        else
          {
            archive->get_primitive(name, id, version);
          }
        reset();
        return *this;
      }

      Target & operator&(ByteArray &array)
      {
        if (is_saving())
          {
            archive->add_primitive(name, array);
          }
        else
          {
            archive->get_primitive(name, array, version);
          }
        reset();
        return *this;
      }

      Target & operator&(std::string &s)
      {
        if (is_saving())
          {
            archive->add_primitive(name, s);
          }
        else
          {
            archive->get_primitive(name, s, version);
          }
        reset();
        return *this;
      }

      Target & operator&(const std::string &s)
      {
        if (is_saving())
          {
            archive->add_primitive(name, const_cast<std::string &>(s));
          }
        else
          {
            archive->get_primitive(name, const_cast<std::string &>(s), version);
          }
        reset();
        return *this;
      }

      Target & operator&(bool &i)
      {
        if (is_saving())
          {
            archive->add_primitive(name, i);
          }
        else
          {
            archive->get_primitive(name, i, version);
          }
        reset();
        return *this;
      }

      Target & operator&(int &i)
      {
        if (is_saving())
          {
            archive->add_primitive(name, i);
          }
        else
          {
            archive->get_primitive(name, i, version);
          }
        reset();
        return *this;
      }

      Target & operator&(guint8 &i)
      {
        if (is_saving())
          {
            archive->add_primitive(name, i);
          }
        else
          {
            archive->get_primitive(name, i, version);
          }
        reset();
        return *this;
      }

      Target & operator&(guint16 &i)
      {
        if (is_saving())
          {
            archive->add_primitive(name, i);
          }
        else
          {
            archive->get_primitive(name, i, version);
          }
        reset();
        return *this;
      }

      Target & operator&(guint32 &i)
      {
        if (is_saving())
          {
            archive->add_primitive(name, i);
          }
        else
          {
            archive->get_primitive(name, i, version);
          }
        reset();
        return *this;
      }

      Target & operator&(guint64 &i)
      {
        if (is_saving())
          {
            archive->add_primitive(name, i);
          }
        else
          {
            archive->get_primitive(name, i, version);
          }
        reset();
        return *this;
      }

      template<class Type>
      Target & operator&(Type &t)
      {
        if (is_saving())
          {
            archive->start_container(name, "struct");
            t.serialize(this);
            archive->end_container();
          }
        else
          {
            archive->get_container(name, "struct", version);
            t.serialize(this);
            archive->get_next_in_container(name, "struct", version);
          }
        reset();
        return *this;
      }

      template<typename ListType>
      Target & operator&(std::list<ListType> &l)
      {
        if (is_saving())
          {
            typedef typename std::list<ListType>::iterator iter;

            archive->start_container(name, "list");
            for (iter i = l.begin(); i != l.end(); i++)
              {
                this->operator&(*i);
              }
            archive->end_container();
          }
        else
          {
            bool c = false;
            archive->get_container(name, "list", version);

            do
              {
                // TODO: use abstract factory.
                ListType x;

                this->operator&(x);
                l.push_back(x);

                c = archive->get_next_in_container(name, "list", version);
              } while (c);
          }
        reset();
        return *this;
      }

      template<typename VectorType>
      Target & operator&(std::vector<VectorType> &v)
      {
        if (is_saving())
          {
            typedef typename std::vector<VectorType>::iterator iter;

            archive->start_container(name, "vector");
            for (iter i = v.begin(); i != v.end(); i++)
              {
                this->operator&(*i);
              }
            archive->end_container();
          }
        else
          {
            bool c = false;
            archive->get_container(name, "vector", version);

            do
              {
                // TODO: use abstract factory.
                VectorType x;

                this->operator&(x);
                v.push_back(x);

                c = archive->get_next_in_container(name, "vector", version);
              } while (c);
          }
        reset();
        return *this;
      }


      template<typename KeyType, typename ValueType>
      Target & operator&(std::map<KeyType, ValueType> &m)
      {
        if (is_saving())
          {
            typedef typename std::map<KeyType, ValueType>::iterator iter;

            archive->start_container(name, "map");
            for (iter i = m.begin(); i != m.end(); i++)
              {
                this->operator&(i->first);
                this->operator&(i->second);
              }
            archive->end_container();
          }
        else
          {
            bool c = false;
            archive->get_container(name, "map", version);

            do
              {
                // TODO: use abstract factory.
                KeyType k;
                ValueType v;

                this->operator&(k);
                this->operator&(v);
                m[k] = v;

                c = archive->get_next_in_container(name, "map", version);
              } while (c);
          }
        reset();
        return *this;
      }

      Target(IArchive *archive, Stage stage)
        : archive(archive), stage(stage), version(0)
      {
      }

    private:
      void reset()
      {
        name = "";
        version = 0;
      }

    private:
      IArchive *archive;
      Stage stage;
      std::string name;
      int version;
    };

    template <typename SerializableType>
    void save(SerializableType &src, IArchive *archive)
    {
      Target target(archive, Target::SAVING);

      archive->start_class(src.class_name(), 0);
      src.serialize(&target);
    }

    template <typename SerializableType>
    SerializableType *load(IArchive *archive)
    {
      std::string classname;
      archive->get_class(classname);

      SerializableType *dst = NULL;
      AbstractFactory<SerializableType *>::create(dst, classname);

      if (dst != NULL)
        {
          Target target(archive, Target::LOADING);
          dst->serialize(&target);
        }

      return dst;
    }
  }
}


#endif // SERIALIZER_HH
