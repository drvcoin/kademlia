/**
 *
 * MIT License
 *
 * Copyright (c) 2018 drvcoin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * =============================================================================
 */

#include <algorithm>
#include <json/json.h>
#include <drive/kad/IndexQuery.h>

namespace kad
{
  bool IndexQuery::Initialize(std::string str)
  {
    size_t start = 0;
    while (start < str.size())
    {

      size_t end = str.find(' ', start);
      if (end == std::string::npos)
      {
        end = str.size();
      }

      auto field = ParseField(&str[start], end - start);
      if (field)
      {
        this->query.emplace_back(std::move(field));
      }

      start = end + 1;
    }

    return !this->query.empty();
  }


  bool IndexQuery::Match(const char * content, size_t len) const
  {
    Json::Value target;
    Json::Reader reader;
    if (!reader.parse(content, len, target))
    {
      return false;
    }

    return this->Match(target);
  }


  bool IndexQuery::Match(Json::Value & target) const
  {
    if (!target.isObject() || target.isNull())
    {
      return false;
    }

    for (const auto & field : this->query)
    {
      if (!target.isMember(field->name))
      {
        return false;
      }

      Json::Value & val = target[field->name];
      if ((!val.isString() && !val.isIntegral()) ||
          (val.isString() && field->type != FieldType::STRING) ||
          (val.isIntegral() && field->type != FieldType::INT))
      {
        return false;
      }

      switch (field->type)
      {
        case FieldType::STRING:
        {
          auto f = static_cast<StringField *>(field.get());
          if (f->exclude != (f->value != val.asString()))      // exclude xor (query!=value)
          {
            return false;
          }
          break;
        }

        case FieldType::INT:
        {
          auto f = static_cast<IntField *>(field.get());
          if (f->exclude != (f->value > val.asInt()))
          {
            return false;
          }
          break;
        }
      }
    }

    return true;
  }


  std::unique_ptr<IndexQuery::FieldBase> IndexQuery::ParseField(const char * str, size_t len)
  {
    if (!str || len == 0)
    {
      return nullptr;
    }

    bool exclude = false;
    if (str[0] == '-')
    {
      exclude = true;
      str++;
    }

    const char * delim = std::find(str, str + len, ':');
    if (delim >= str + len - 1)
    {
      return nullptr;
    }

    Json::Value valueJson;
    Json::Reader reader;
    bool parsed = reader.parse(delim + 1, str + len - delim - 1, valueJson);

    if (!parsed)
    {
      auto ptr = new StringField();
      ptr->name = std::string(str, delim - str);
      ptr->type = FieldType::STRING;
      ptr->value = std::string(delim + 1, str + len - delim - 1);
      return std::unique_ptr<FieldBase>(ptr);
    }
    else if (valueJson.isString())
    {
      auto ptr = new StringField();
      ptr->name = std::string(str, delim - str);
      ptr->type = FieldType::STRING;
      ptr->value = valueJson.asString();
      ptr->exclude = exclude;
      return std::unique_ptr<FieldBase>(ptr);
    }
    else if (valueJson.isIntegral())
    {
      auto ptr = new IntField();
      ptr->name = std::string(str, delim - str);
      ptr->type = FieldType::INT;
      ptr->value = valueJson.asInt();
      ptr->exclude = exclude;
      return std::unique_ptr<FieldBase>(ptr);
    }

    return nullptr;
  }
}
