/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#ifndef SDF_PARSER_JOINT_HH_
#define SDF_PARSER_JOINT_HH_

#include <string>

#include <pxr/usd/usd/stage.h>

#include "sdf/Joint.hh"
#include "sdf/Model.hh"
#include "sdf/sdf_config.h"

namespace usd
{
  /// \brief Parse an SDF joint into a USD stage.
  /// \param[in] _joint The SDF joint to parse.
  /// \param[in] _stage The stage that should contain the USD representation
  /// of _joint.
  /// \param[in] _path The USD path of the parsed joint in _stage, which must be
  /// a valid USD path.
  /// \param[in] _parentModel The model that is the parent of _joint
  /// \param[in] _linkToUSDPath a map of a link's SDF name to the link's USD path.
  /// \param[in] _worldPath The USD path of the world prim. This is needed if
  /// _joint's parent is the world.
  /// \return True if _joint was succesfully parsed into _stage with a path of
  /// _path. False otherwise.
  bool SDFORMAT_VISIBLE ParseSdfJoint(const sdf::Joint &_joint,
      pxr::UsdStageRefPtr &_stage, const std::string &_path,
      const sdf::Model &_parentModel,
      const std::unordered_map<std::string, pxr::SdfPath> &_linkToUSDPath,
      const pxr::SdfPath &_worldPath);
}

#endif