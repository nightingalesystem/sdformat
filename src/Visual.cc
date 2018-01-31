/*
 * Copyright 2018 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#include "sdf/Visual.hh"
#include "Utils.hh"

using namespace sdf;

class sdf::VisualPrivate
{
  /// \brief Name of the visual.
  public: std::string name = "";
};

/////////////////////////////////////////////////
Visual::Visual()
  : dataPtr(new VisualPrivate)
{
}

/////////////////////////////////////////////////
Visual::~Visual()
{
  delete this->dataPtr;
  this->dataPtr = nullptr;
}

/////////////////////////////////////////////////
Errors Visual::Load(ElementPtr _sdf)
{
  Errors errors;

  // Check that the provided SDF element is a <visual>
  // This is an error that cannot be recovered, so return an error.
  if (_sdf->GetName() != "visual")
  {
    errors.push_back({ErrorCode::ELEMENT_INCORRECT_TYPE,
        "Attempting to load a Visual, but the provided SDF element is not a "
        "<visual>."});
    return errors;
  }

  // Read the visuals's name
  if (!loadName(_sdf, this->dataPtr->name))
  {
    errors.push_back({ErrorCode::ATTRIBUTE_MISSING,
                     "A visual name is required, but the name is not set."});
  }

  return errors;
}

/////////////////////////////////////////////////
std::string Visual::Name() const
{
  return this->dataPtr->name;
}

/////////////////////////////////////////////////
void Visual::SetName(const std::string &_name) const
{
  this->dataPtr->name = _name;
}